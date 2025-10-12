# Content Tools

Developer tools for managing game content in the Qimmiit engine ecosystem.

## Available Tools

### Sakku - Content Management CLI

**Sakku** is a CLI tool (like Angular CLI or Vue CLI) that helps manage game content packages from npm.

- **Location**: [sakku/](sakku/)
- **Install**: `npm install -g @qimmiit/sakku`
- **Purpose**: Create projects, manage content packages, build assets

**Quick Start:**
```bash
npm install -g @qimmiit/sakku
sakku create my-game
sakku add @qimmiit-content/industrial-pack
sakku build
```

See [sakku/README.md](sakku/README.md) for full documentation.

## Tool Philosophy

Content tools in Qimmiit follow these principles:

1. **Build on npm**: Use npm as the package manager, don't reinvent the wheel
2. **CLI-first**: Provide powerful command-line interfaces
3. **Developer Experience**: Make common tasks simple and intuitive
4. **Integration**: Work seamlessly with the Qimmiit engine

## Related

- **[Orchestrators](../orchestrators/)**: Central orchestrators (Nanook)
- **[Modules](../modules/)**: Engine modules
- **[Apps](../apps/)**: Executable applications
- **[Qilak Quickstarts](../qilak-quickstarts/)**: Example projects