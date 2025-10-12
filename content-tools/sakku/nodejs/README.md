# Sakku Node.js Wrapper

This is the Node.js wrapper for Sakku that can be installed via npm.

## Installation

```bash
npm install -g @qimmiit/sakku
```

## How It Works

This package contains:

1. **Node.js CLI** - Command-line interface and npm integration
2. **Native Binaries** - Bundled C++ binaries for asset processing (in `binaries/`)

When you run `sakku build`, the Node.js wrapper:
1. Parses command-line arguments
2. Validates configuration
3. Spawns the native binary (`sakku-cli.exe`) for heavy processing
4. Displays progress and results

## Development

### Install Dependencies

```bash
npm install
```

### Build Native Binaries

The native binaries must be built separately for each platform:

```bash
# Build for current platform
npm run build
```

This will compile the C++ code from `../native/` and place the binary in the correct `binaries/` directory.

### Testing

```bash
npm test
```

### Publishing

```bash
npm publish
```

This will automatically bundle the platform-specific binaries with the package.

## Structure

```
nodejs/
├── package.json          # npm package definition
├── bin/
│   └── sakku.js         # CLI entry point
├── src/
│   ├── index.js         # Programmatic API
│   ├── native-bridge.js # Bridge to C++ binary
│   ├── utils/
│   │   └── platform.js  # Platform detection
│   └── commands/        # CLI commands
│       ├── create.js
│       ├── add.js
│       ├── build.js
│       ├── publish.js
│       └── search.js
└── binaries/            # Platform-specific binaries
    ├── win32-x64/
    │   └── sakku-cli.exe
    ├── linux-x64/
    │   └── sakku-cli
    └── darwin-arm64/
        └── sakku-cli
```

## See Also

- [../native/](../native/) - C++ binary source code
- [../README.md](../README.md) - Main Sakku documentation
