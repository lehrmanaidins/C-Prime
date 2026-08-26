# C-Prime Syntax Highlighting

VS Code language support for C-Prime source files.

## Local installation

From this directory, install the packaging tool and create a VSIX:

```bash
npm install --global @vscode/vsce
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
