# Sakku Quick Start Guide

Get started with Sakku in 5 minutes!

## Installation

```bash
# Build Sakku
cd sakku
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

# The executable is in: build/bin/sakku-cli
```

## Basic Usage

### 1. Search for Libraries

```bash
sakku-cli search "industrial"
```

Output:
```
Searching for: "industrial"
Registry: https://registry.sakku.io

Found 25 libraries (showing page 1)
================================================================================

industrial-pack v1.0.0
  Industrial 3D assets including containers, crates, and machinery
  Author: Example Studio
  Tags: 3d-models, industrial, pbr
  Downloads: 1234

industrial-building-kit v2.1.0
  Modular industrial building components
  Author: Architecture Team
  Tags: 3d-models, industrial, modular
  Downloads: 856

================================================================================
To install: sakku add <library-name>[@version]
```

### 2. Install a Library

```bash
sakku-cli add industrial-pack@latest
```

Output:
```
Installing: industrial-pack (latest)
Registry: https://registry.sakku.io
Output: ./libraries

Resolving latest version...
Latest version: 1.0.0
Fetching library metadata...
Found: industrial-pack v1.0.0
Description: Industrial 3D assets including containers, crates, and machinery
Author: Example Studio

Downloading to: ./libraries/industrial-pack
Success! Library installed successfully.
Location: ./libraries/industrial-pack
```

### 3. Create Your Own Library

```bash
# Create directory structure
mkdir my-awesome-library
cd my-awesome-library
mkdir -p assets/{models,textures,materials}

# Create manifest
cat > sakku.json << 'EOF'
{
  "name": "my-awesome-library",
  "version": "1.0.0",
  "description": "My collection of awesome game assets",
  "author": "Your Name <you@example.com>",
  "tags": ["3d-models", "game-ready"],
  "assets": []
}
EOF
```

### 4. Validate Your Library

```bash
sakku-cli publish ./my-awesome-library --dry-run
```

Output:
```
Publishing library from: ./my-awesome-library
Registry: https://registry.sakku.io
Creating manifest...
Library: my-awesome-library v1.0.0
Description: My collection of awesome game assets
Assets: 0 files

Dry run mode - validation successful!
Manifest is valid and ready for publishing.
```

### 5. Publish Your Library

```bash
# Set your API token
export SAKKU_API_TOKEN=your_token_here

# Publish
sakku-cli publish ./my-awesome-library
```

## Common Workflows

### Installing Multiple Libraries

```bash
sakku-cli add pbr-materials@latest
sakku-cli add environment-pack@2.0.0
sakku-cli add character-animations@1.5.0
```

### Custom Installation Directory

```bash
# Install to specific directory
sakku-cli add industrial-pack --output ./game-assets/libraries

# Or set environment variable
export SAKKU_LIBRARIES_PATH=./game-assets/libraries
sakku-cli add industrial-pack
```

### Using Custom Registry

```bash
# Use company-internal registry
sakku-cli search "internal-assets" --registry https://registry.yourcompany.com

# Or set environment variable
export SAKKU_REGISTRY_URL=https://registry.yourcompany.com
sakku-cli search "internal-assets"
```

### Search with Filters

```bash
# Search with tag filter
sakku-cli search "environment" --tag nature

# Paginated search
sakku-cli search "character" --page 2 --limit 20
```

## Environment Variables

Configure Sakku with these environment variables:

```bash
# Registry URL
export SAKKU_REGISTRY_URL=https://registry.sakku.io

# API Token for publishing
export SAKKU_API_TOKEN=your_api_token_here

# Default library installation path
export SAKKU_LIBRARIES_PATH=./libraries
```

## Manifest Reference

Minimal `sakku.json`:

```json
{
  "name": "my-library",
  "version": "1.0.0",
  "description": "Library description",
  "author": "Your Name"
}
```

Complete `sakku.json`:

```json
{
  "name": "my-library",
  "version": "1.0.0",
  "description": "Detailed library description",
  "author": "Your Name <email@example.com>",
  "tags": ["tag1", "tag2", "tag3"],
  "dependencies": [
    "other-library@^2.0.0",
    "another-library@1.5.0"
  ],
  "metadata": {
    "license": "MIT",
    "homepage": "https://example.com",
    "repository": "https://github.com/user/repo"
  },
  "assets": [
    {
      "path": "assets/model.gltf",
      "type": "model",
      "size": 123456,
      "hash": "abc123"
    }
  ]
}
```

## Tips

1. **Always use semantic versioning**: `major.minor.patch` (e.g., `1.2.3`)
2. **Add descriptive tags**: Help others find your library
3. **Document your assets**: Include README with usage instructions
4. **Test before publishing**: Use `--dry-run` flag
5. **Version your dependencies**: Use specific versions or ranges

## Next Steps

- Read the full [README.md](README.md)
- Check out [example manifests](examples/)
- Visit the registry: https://registry.sakku.io
- Read the API docs: https://sakku.io/docs

---

Happy library sharing! 🚀
