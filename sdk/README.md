# Qimmiit SDK

This directory contains the Qimmiit SDK infrastructure.

## Structure

```
sdk/
├── qimmiit-sdk-dev/              # Development SDK (source)
│   ├── CMakeLists.txt            # Aggregates all modules
│   ├── examples/                 # Example applications
│   └── README.md                 # Detailed documentation
│
└── packaging/                    # SDK distribution tools
    └── generate-release-sdk.cmake # Script to generate release SDK
```

## Quick Start

### For Qimmiit Developers

Use `qimmiit-sdk-dev` when building Qimmiit applications:

```bash
cd qimmiit-sdk-dev
cmake -B build
cmake --build build
```

### For External Users

Generate a release SDK package:

```bash
cd packaging
cmake -P generate-release-sdk.cmake
```

This creates a distributable SDK in `dist/sdk/qimmiit-sdk-{version}-{platform}/`

## What's Included

The SDK provides unified access to:
- **Rendering** - Vulkan-based graphics (coregfx)
- **Assets** - Asset management and caching (cryo-asset, cryo-cache)
- **Storage** - Backend storage system (akutik)
- **Tooling** - Development tools (cryo-tooling)

## Usage

See [qimmiit-sdk-dev/README.md](qimmiit-sdk-dev/README.md) for detailed usage instructions.
