const vscode = require("vscode");

const KEYWORDS = [
  "type", "alias", "const", "mutable", "primitive", "struct", "enum", "union",
  "function", "template", "return", "requires", "ensures", "result", "old",
  "if", "else", "switch", "case", "default", "for", "foreach", "do", "while",
  "loop", "limit", "continue", "break", "import", "unsafe", "null", "void",
  "pure", "entry", "critical", "interrupt", "true", "false"
];

const PRIMITIVES = [
  "bool", "char8", "char16", "char32", "int8", "int16", "int32", "int64",
  "uint8", "uint16", "uint32", "uint64", "float32", "float64",
  "reference", "array", "list"
];

const BUILTINS = {
  print: "Writes a value without a trailing newline.",
  println: "Writes a value followed by a newline.",
  array_fill: "Fills an array with a value.",
  widen_cast: "Performs a widening conversion.",
  narrow_cast: "Performs a narrowing conversion.",
  underlying_cast: "Converts an enum value to its underlying type.",
  reinterpret_cast: "Reinterprets a value as another type.",
  reference: "Constructs a reference to a value.",
  static_assert: "Checks a compile-time condition and triggers a compilation error if the condition is false.",
  assert: "Checks a runtime condition and triggers an error if the condition is false."
};

function identifierRange(document, position) {
  return document.getWordRangeAtPosition(position, /[A-Za-z_][A-Za-z0-9_]*/);
}

function documentSymbols(document) {
  const symbols = [];
  const pattern = /^\s*(?:(type|struct|enum|union)\s+([A-Za-z_]\w*)|function\s+([A-Za-z_]\w*)|(?:const|mutable\s+)?(?:primitive\s+)?(?:[A-Za-z_]\w*(?:\[\])?)\s+([A-Za-z_]\w*))\b/gm;
  const text = document.getText();
  let match;

  while ((match = pattern.exec(text)) !== null) {
    const name = match[2] || match[3] || match[4];
    const kind = match[2]
      ? (match[1] === "enum" ? vscode.SymbolKind.Enum : match[1] === "struct" || match[1] === "union" ? vscode.SymbolKind.Struct : vscode.SymbolKind.TypeParameter)
      : match[3] ? vscode.SymbolKind.Function : vscode.SymbolKind.Variable;
    const start = document.positionAt(match.index + match[0].lastIndexOf(name));
    const range = new vscode.Range(start, start.translate(0, name.length));
    symbols.push(new vscode.DocumentSymbol(name, match[1] || "declaration", kind, range, range));
  }
  return symbols;
}

function collectWorkspaceSymbols() {
  const result = [];
  for (const document of vscode.workspace.textDocuments) {
    if (document.languageId !== "cprime") continue;
    for (const symbol of documentSymbols(document)) {
      result.push({ name: symbol.name, kind: symbol.kind, location: new vscode.Location(document.uri, symbol.selectionRange) });
    }
  }
  return result;
}

function validateDocument(document, diagnostics) {
  if (document.languageId !== "cprime" || !vscode.workspace.getConfiguration("cprime", document.uri).get("enableDiagnostics", true)) return;
  const errors = [];
  const opening = { "(": ")", "[": "]", "{": "}" };
  const closing = new Set(Object.values(opening));
  const stack = [];
  const text = document.getText();
  let inString = null;
  let inBlockComment = false;

  for (let index = 0; index < text.length; index += 1) {
    const character = text[index];
    const next = text[index + 1];
    if (inBlockComment) {
      if (character === "*" && next === "/") { inBlockComment = false; index += 1; }
      continue;
    }
    if (!inString && character === "/" && next === "*") { inBlockComment = true; index += 1; continue; }
    if (!inString && character === "/" && next === "/") { index = text.indexOf("\n", index); if (index === -1) break; continue; }
    if (inString) {
      if (character === "\\") { index += 1; continue; }
      if (character === inString) inString = null;
      continue;
    }
    if (character === "\"" || character === "'") { inString = character; continue; }
    if (opening[character]) stack.push({ character, index });
    else if (closing.has(character)) {
      const previous = stack.pop();
      if (!previous || opening[previous.character] !== character) {
        const position = document.positionAt(index);
        errors.push(new vscode.Diagnostic(new vscode.Range(position, position.translate(0, 1)), `Unexpected '${character}'.`, vscode.DiagnosticSeverity.Error));
      }
    }
  }
  for (const item of stack) {
    const position = document.positionAt(item.index);
    errors.push(new vscode.Diagnostic(new vscode.Range(position, position.translate(0, 1)), `Missing '${opening[item.character]}' for '${item.character}'.`, vscode.DiagnosticSeverity.Error));
  }
  if (inString) {
    const position = document.positionAt(text.length);
    errors.push(new vscode.Diagnostic(new vscode.Range(position, position), "Unterminated string literal.", vscode.DiagnosticSeverity.Error));
  }
  diagnostics.set(document.uri, errors);
}

function activate(context) {
  const diagnostics = vscode.languages.createDiagnosticCollection("cprime");
  const completionItems = [
    ...KEYWORDS.map(word => new vscode.CompletionItem(word, vscode.CompletionItemKind.Keyword)),
    ...PRIMITIVES.map(word => new vscode.CompletionItem(word, vscode.CompletionItemKind.TypeParameter)),
    ...Object.entries(BUILTINS).map(([name, documentation]) => {
      const item = new vscode.CompletionItem(name, vscode.CompletionItemKind.Function);
      item.documentation = documentation;
      item.insertText = new vscode.SnippetString(`${name}(\${1:value})`);
      return item;
    })
  ];

  const selector = { language: "cprime" };
  context.subscriptions.push(diagnostics);
  context.subscriptions.push(vscode.languages.registerCompletionItemProvider(selector, {
    provideCompletionItems() {
      return [...completionItems, ...collectWorkspaceSymbols().map(symbol => new vscode.CompletionItem(symbol.name, vscode.CompletionItemKind.Variable))];
    }
  }, ".", ":"));
  context.subscriptions.push(vscode.languages.registerHoverProvider(selector, {
    provideHover(document, position) {
      const range = identifierRange(document, position);
      if (!range) return undefined;
      const word = document.getText(range);
      const builtin = BUILTINS[word];
      if (builtin) return new vscode.Hover(new vscode.MarkdownString(`**${word}**\n\n${builtin}`), range);
      const symbol = collectWorkspaceSymbols().find(candidate => candidate.name === word);
      return symbol ? new vscode.Hover(new vscode.MarkdownString(`**${word}**\n\nC-Prime declaration`), range) : undefined;
    }
  }));
  context.subscriptions.push(vscode.languages.registerDefinitionProvider(selector, {
    provideDefinition(document, position) {
      const range = identifierRange(document, position);
      if (!range) return undefined;
      const word = document.getText(range);
      const local = documentSymbols(document).find(symbol => symbol.name === word);
      return local ? new vscode.Location(document.uri, local.selectionRange) : collectWorkspaceSymbols().find(symbol => symbol.name === word)?.location;
    }
  }));
  context.subscriptions.push(vscode.languages.registerDocumentSymbolProvider(selector, { provideDocumentSymbols: documentSymbols }));
  context.subscriptions.push(vscode.languages.registerWorkspaceSymbolProvider({ provideWorkspaceSymbols: query => collectWorkspaceSymbols().filter(symbol => symbol.name.toLowerCase().includes(query.toLowerCase())) }));
  context.subscriptions.push(vscode.workspace.onDidChangeTextDocument(event => validateDocument(event.document, diagnostics)));
  context.subscriptions.push(vscode.workspace.onDidCloseTextDocument(document => diagnostics.delete(document.uri)));
  context.subscriptions.push(vscode.workspace.onDidChangeConfiguration(() => vscode.workspace.textDocuments.forEach(document => validateDocument(document, diagnostics))));
  vscode.workspace.textDocuments.forEach(document => validateDocument(document, diagnostics));
}

function deactivate() {}

module.exports = { activate, deactivate };