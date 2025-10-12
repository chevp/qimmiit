# Sakku Architecture

## Overview

Sakku is a library registry CLI tool that enables game developers to share and discover reusable game content libraries. It follows the package manager pattern popularized by npm, cargo, and pip, but specifically tailored for game development assets.

## Design Goals

1. **Simple to use**: Intuitive CLI with minimal learning curve
2. **Fast**: Efficient download and installation
3. **Extensible**: Support for custom registries and plugins
4. **Portable**: Cross-platform (Windows, Linux, macOS)
5. **Secure**: Manifest validation and content verification

## Architecture Components

### 1. CLI Layer (`src/main.cpp`)

The entry point for the application. Handles:
- Command-line argument parsing
- Command routing to appropriate handlers
- Help and version information
- Error handling and user feedback

**Commands:**
- `publish` - Publish a library to registry
- `search` - Search for libraries
- `add` - Install a library
- `init` - Initialize a new library
- `version` - Show version info
- `help` - Display help

### 2. Command Handlers (`src/commands/`)

Individual command implementations:

#### PublishCommand
- Validates library structure
- Creates/validates manifest
- Packages library into archive
- Uploads to registry via API
- Handles authentication

#### SearchCommand
- Queries registry API
- Filters results by tags/criteria
- Displays formatted results
- Handles pagination

#### AddCommand
- Resolves library versions
- Downloads from registry
- Extracts to local directory
- Resolves dependencies (future)

### 3. Registry Client (`src/registry/RegistryClient.cpp`)

Handles all communication with the registry server:

**Responsibilities:**
- HTTP/HTTPS requests using libcurl
- JSON serialization/deserialization
- Authentication (Bearer tokens)
- Response parsing
- Error handling

**API Endpoints:**
```
GET  /api/search?q=<query>&page=<n>&limit=<n>
GET  /api/packages/<name>
GET  /api/packages/<name>/<version>
GET  /api/packages/<name>/latest
GET  /api/packages/<name>/<version>/manifest
POST /api/packages (with multipart/form-data)
```

### 4. Manifest Manager (`src/manifest/ManifestManager.cpp`)

Manages library manifests (metadata):

**Responsibilities:**
- Create manifest from directory
- Load/parse manifest files
- Validate manifest structure
- Generate manifest JSON
- Asset enumeration
- Hash generation

**Manifest Schema:**
```json
{
  "name": "string (required)",
  "version": "semver (required)",
  "description": "string",
  "author": "string",
  "tags": ["string"],
  "dependencies": ["string@version"],
  "metadata": {"key": "value"},
  "assets": [
    {
      "path": "string",
      "type": "string",
      "size": "number",
      "hash": "string"
    }
  ]
}
```

### 5. File Utilities (`src/utils/FileUtils.cpp`)

Cross-platform file system operations:

**Responsibilities:**
- Directory creation/traversal
- File reading/writing
- Path manipulation
- Archive creation/extraction
- File size/hash calculation

Uses C++17 `<filesystem>` for portability.

## Data Flow

### Publishing a Library

```
User Command
    ↓
PublishCommand
    ↓
ManifestManager.createFromDirectory()
    ↓
ManifestManager.validate()
    ↓
FileUtils.createArchive()
    ↓
RegistryClient.publishLibrary()
    ↓
HTTP POST to /api/packages
    ↓
Registry Server
```

### Searching for Libraries

```
User Command
    ↓
SearchCommand
    ↓
RegistryClient.search()
    ↓
HTTP GET to /api/search
    ↓
Registry Server
    ↓
JSON Response
    ↓
Display Results
```

### Installing a Library

```
User Command
    ↓
AddCommand
    ↓
RegistryClient.getLatestVersion() (if needed)
    ↓
RegistryClient.getLibrary()
    ↓
RegistryClient.downloadLibrary()
    ↓
HTTP GET to download URL
    ↓
FileUtils.extractArchive()
    ↓
Local Installation
```

## Registry Server (Conceptual)

While Sakku is the CLI client, it communicates with a registry server. The server would implement:

### API Endpoints

**Search:**
```
GET /api/search?q=query&page=1&limit=10&tag=filter
Response: {
  "libraries": [...],
  "total": 123,
  "page": 1,
  "pageSize": 10
}
```

**Get Library:**
```
GET /api/packages/{name}/{version}
Response: {
  "name": "...",
  "version": "...",
  "downloadUrl": "https://cdn.sakku.io/...",
  "manifestUrl": "https://cdn.sakku.io/..."
}
```

**Publish:**
```
POST /api/packages
Headers: Authorization: Bearer <token>
Body: multipart/form-data with archive and manifest
Response: {
  "success": true,
  "packageUrl": "..."
}
```

### Storage

- **Metadata Database**: PostgreSQL/MySQL for library metadata
- **CDN**: S3/CloudFront for library archives
- **Search Index**: Elasticsearch for full-text search

## Security Considerations

### Authentication
- API tokens for publishing
- Token stored in environment variable
- HTTPS-only communication

### Validation
- Manifest schema validation
- File hash verification
- Size limit enforcement
- Malware scanning (server-side)

### Privacy
- No tracking of downloads
- Optional author information
- Public registry by default

## Future Enhancements

### Dependency Resolution
Automatically install dependencies listed in manifest:
```json
"dependencies": [
  "common-materials@^2.0.0",
  "shader-library@1.5.0"
]
```

### Semantic Versioning
Full semver support with version constraints:
- `^1.2.3` - Compatible with 1.x.x
- `~1.2.3` - Compatible with 1.2.x
- `>=1.0.0 <2.0.0` - Range

### Lock Files
Generate `sakku-lock.json` for reproducible builds:
```json
{
  "dependencies": {
    "industrial-pack": {
      "version": "1.0.0",
      "resolved": "https://cdn.sakku.io/...",
      "integrity": "sha256-..."
    }
  }
}
```

### Mirror Registries
Support for multiple registries:
```bash
sakku-cli config set-registry https://company-registry.internal
```

### Plugin System
Allow custom commands and transformations:
```cpp
// Custom plugin
class MyPlugin : public SakkuPlugin {
  void onPostInstall(Library& lib) override {
    // Custom processing
  }
};
```

### Content Validation
Registry-side validation:
- Model format verification
- Texture dimension checks
- File integrity verification
- License compliance

### Statistics
Track library usage:
- Download counts
- Version distribution
- Popular tags
- Trending libraries

## Technology Stack

- **Language**: C++17
- **HTTP Client**: libcurl
- **JSON**: nlohmann/json
- **Filesystem**: std::filesystem (C++17)
- **Build**: CMake 3.20+
- **Package Manager**: vcpkg

## Performance Considerations

### Download Optimization
- Parallel chunk downloads
- Resume capability
- Compression (gzip/brotli)
- CDN caching

### Search Optimization
- Client-side result caching
- Pagination
- Tag-based indexing

### Storage Efficiency
- Deduplication of common assets
- Delta updates for versions
- Compressed archives

## Error Handling

### Network Errors
- Retry with exponential backoff
- Timeout handling
- Connection failure recovery

### File System Errors
- Insufficient permissions
- Disk space checks
- Atomic file operations

### Validation Errors
- Clear error messages
- Suggestions for fixes
- Dry-run mode for testing

## Testing Strategy

### Unit Tests
- Manifest validation
- Path manipulation
- Hash generation
- Version parsing

### Integration Tests
- Mock registry server
- End-to-end workflows
- Error scenarios

### Performance Tests
- Large library downloads
- Many small files
- Search performance

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for development guidelines.

## License

[License information]

---

**Sakku Architecture v1.0.0**
*Building a better ecosystem for game content sharing*
