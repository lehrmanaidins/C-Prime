# C-Prime Language Support

VS Code language support for `.cprime` and `.hprime` source files.

## Features

- Syntax highlighting for C-Prime keywords, types, comments, strings, numbers, attributes, functions, and operators.
- Bracket matching, automatic closing pairs, indentation, and C-Prime snippets.
- Completions for keywords, primitives, built-in functions, and declarations in open C-Prime files.
- Hover documentation for runtime built-ins, go-to-definition, document outline, and workspace symbol search.
- Live diagnostics for unmatched delimiters and unterminated string literals. Set `cprime.enableDiagnostics` to `false` to disable them.

## Local installation

From this directory, use Node.js 18 or later to create a VSIX:

```bash
npm run validate
npm run package
```

Then install the generated package globally for your VS Code user:

```bash
code --install-extension c-prime-syntax-0.1.0.vsix
```

Restart or reload VS Code, then open a `.cprime` or `.hprime` file.

## Publishing

To publish this extension to the Visual Studio Marketplace, replace the
publisher value in `package.json` with your Marketplace publisher ID and run:

```bash
vsce publish
```
