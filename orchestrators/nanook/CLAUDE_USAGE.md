# Claude Code Usage Guide - Cryo Engine

**AI-Assisted Development Workflow for Central Orchestrator Architecture**

---

## Overview

This guide explains how to effectively use **Claude Code** (AI-assisted development) when working with the Cryo Engine central orchestrator architecture. Claude Code can help with module implementation, debugging, refactoring, and documentation.

---

## Table of Contents

1. [Getting Started with Claude](#getting-started-with-claude)
2. [Common Tasks](#common-tasks)
3. [Module Development Workflow](#module-development-workflow)
4. [Debugging with Claude](#debugging-with-claude)
5. [Refactoring Assistance](#refactoring-assistance)
6. [Documentation Generation](#documentation-generation)
7. [Best Practices](#best-practices)
8. [Prompt Templates](#prompt-templates)

---

## Getting Started with Claude

### Initial Context Setup

When starting a Claude Code session for Cryo Engine development, provide this context:

```
I'm working on Cryo Engine, a central orchestrator architecture for a Vulkan
graphics engine. The architecture uses a CentralAgent singleton that coordinates
multiple modules (Renderer, Cache, Asset, HTTP, Compute). All modules implement
the IModule interface and communicate via an EventBus or direct messaging.

Key files:
- cryo-engine/src/core/CentralAgent.hpp|cpp - Main orchestrator
- cryo-engine/src/core/IModule.hpp - Module interface
- cryo-engine/src/modules/ - Individual module implementations
- cryo-engine/ARCHITECTURE.md - Detailed design document
```

### Relevant Documentation to Share

Before asking questions, tell Claude to read:
1. `cryo-engine/README.md` - Overview and usage
2. `cryo-engine/ARCHITECTURE.md` - Detailed technical design
3. Specific module files you're working on

**Example prompt**:
```
Read cryo-engine/ARCHITECTURE.md and cryo-engine/src/modules/RendererModule.cpp,
then help me add a new takeScreenshot() method to the RendererModule.
```

---

## Common Tasks

### 1. Creating a New Module

**Prompt Template**:
```
Create a new module called [ModuleName] that implements IModule. The module should:
- [Responsibility 1]
- [Responsibility 2]
- Depend on [DependencyModule1, DependencyModule2]
- Subscribe to [EventType1, EventType2]
- Expose these public methods: [method1(), method2()]

Follow the existing pattern in RendererModule.cpp and include:
- Header file with class declaration
- Source file with implementation
- Proper initialization/shutdown
- Error handling via LogManager
```

**Example**:
```
Create a new module called PhysicsModule that implements IModule. The module should:
- Initialize Bullet Physics engine
- Update physics simulation each frame
- Depend on RendererModule (for debug visualization)
- Subscribe to ENTITY_SPAWNED events
- Expose these public methods: addRigidBody(), setGravity(), raycast()
```

### 2. Adding a New Event Type

**Prompt Template**:
```
Add a new event type called [EVENT_NAME] to the EventBus system. This event should:
- Be published by [SourceModule]
- Contain these fields in the payload: [field1, field2, field3]
- Be subscribed to by [TargetModule1, TargetModule2]

Update EventBus.hpp to include the new event type and show me how to publish
and subscribe to it.
```

**Example**:
```
Add a new event type called TEXTURE_LOADED to the EventBus system. This event should:
- Be published by AssetModule when a texture finishes loading
- Contain these fields: textureId (string), width (int), height (int), format (enum)
- Be subscribed to by RendererModule to upload the texture to GPU

Update EventBus.hpp and show example usage.
```

### 3. Implementing Inter-Module Communication

**Prompt Template**:
```
I need [ModuleA] to request data from [ModuleB] and wait for a response.
Implement a request/response pattern using direct messaging via CentralAgent.

The request should include: [parameters]
The response should include: [results]

Show me the code for both the requesting side and the responding side.
```

**Example**:
```
I need RendererModule to request a cached scene from CacheModule and wait for a response.

The request should include: sceneId (string)
The response should include: success (bool), sceneData (protobuf binary)

Show me the code for both sides.
```

### 4. Adding Configuration Options

**Prompt Template**:
```
Add configuration options for [ModuleName] to arctic.config.xml:
- [option1]: [description, type, default value]
- [option2]: [description, type, default value]

Show me:
1. The XML structure to add to arctic.config.xml
2. How to read these values in the module's initialize() method
3. Validation logic to ensure values are in valid ranges
```

**Example**:
```
Add configuration options for PhysicsModule to arctic.config.xml:
- gravity: Gravity vector (vec3), default (0, -9.81, 0)
- timestep: Simulation timestep in seconds (float), default 0.016
- max-substeps: Maximum physics substeps per frame (int), default 10

Show me the XML and the C++ code to read and validate these values.
```

---

## Module Development Workflow

### Step 1: Design Phase

**Prompt**:
```
I want to create a [ModuleName] that [high-level description].

Before writing code, help me design:
1. What dependencies should this module have?
2. What events should it publish/subscribe to?
3. What public API should it expose?
4. What configuration options does it need?

Use the existing modules (Renderer, Cache, Asset, HTTP) as reference.
```

### Step 2: Implementation

**Prompt**:
```
Based on the design we discussed, generate:
1. [ModuleName].hpp - Header file with class declaration
2. [ModuleName].cpp - Source file with implementation stubs
3. CMakeLists.txt entry to build this module

Follow the existing code style and patterns from RendererModule.
```

### Step 3: Testing

**Prompt**:
```
Generate unit tests for [ModuleName] using Google Test. Tests should cover:
- Initialization success/failure
- Dependency resolution
- Event publishing/subscribing
- Message handling
- Shutdown cleanup

Use MockAgent for isolating the module during testing.
```

### Step 4: Integration

**Prompt**:
```
Show me how to integrate [ModuleName] into the main application:
1. Where to register it in main.cpp
2. What order it should be initialized in (considering dependencies)
3. Example usage of its public API
4. How to verify it's working correctly
```

---

## Debugging with Claude

### 1. Understanding Initialization Failures

**Prompt**:
```
My [ModuleName] is failing to initialize with this error:
[paste error message]

Here's the relevant code:
[paste code snippet]

Help me debug this. Check:
1. Are dependencies declared correctly?
2. Is the module registered before its dependents?
3. Are there any initialization order issues?
4. Is the configuration being read correctly?
```

### 2. Diagnosing Event Bus Issues

**Prompt**:
```
I'm publishing [EVENT_TYPE] from [ModuleA] but [ModuleB] isn't receiving it.

Here's the publish code:
[paste publisher code]

Here's the subscribe code:
[paste subscriber code]

What could be wrong?
```

### 3. Analyzing Performance Issues

**Prompt**:
```
The engine is running slow when [scenario description]. I suspect [ModuleName]
is the bottleneck.

Here's the module's update() method:
[paste code]

Help me:
1. Identify performance bottlenecks
2. Suggest optimizations
3. Show how to add profiling with agent.getProfiler()
```

### 4. Memory Leak Detection

**Prompt**:
```
I'm seeing increasing memory usage over time. I think [ModuleName] has a leak.

Here's the module code:
[paste code]

Check for:
1. Missing cleanup in shutdown()
2. Event subscription leaks (not unsubscribing)
3. Memory allocations that aren't freed
4. Reference cycles
```

---

## Refactoring Assistance

### 1. Extracting Shared Functionality

**Prompt**:
```
I have duplicate code in [ModuleA] and [ModuleB]:
[paste duplicate code]

Help me extract this into a shared utility function that both modules can use.
Where should this utility live (e.g., cryo-engine/src/utils/)?
```

### 2. Improving Module Interfaces

**Prompt**:
```
My [ModuleName] interface is getting complex with too many public methods.
Here's the current interface:
[paste class declaration]

Help me refactor this into a cleaner API. Consider:
1. Grouping related methods
2. Hiding implementation details
3. Making the interface more intuitive
```

### 3. Migrating from Legacy Code

**Prompt**:
```
I need to migrate existing code from [LegacyProject] to fit the Cryo Engine
architecture. Here's the legacy code:
[paste legacy code]

Show me how to:
1. Wrap this in a module that implements IModule
2. Integrate it with CentralAgent
3. Replace hardcoded paths with ConfigManager
4. Replace printf debugging with LogManager
```

---

## Documentation Generation

### 1. Generating Module Documentation

**Prompt**:
```
Generate comprehensive documentation for [ModuleName] in Markdown format. Include:
- Overview and purpose
- Dependencies
- Configuration options
- Public API reference (with examples)
- Events published/subscribed
- Usage examples
- Common troubleshooting

Format it like the existing README.md files.
```

### 2. Creating Architecture Diagrams

**Prompt**:
```
Create an ASCII art diagram showing how [ModuleA], [ModuleB], and [ModuleC]
interact via the CentralAgent. Include:
- Initialization order
- Event flow
- Message passing
- Shared services used
```

### 3. Writing API Documentation

**Prompt**:
```
Generate Doxygen-style comments for this class:
[paste class declaration]

Include:
- Class description
- Method descriptions
- Parameter documentation
- Return value documentation
- Usage examples
```

---

## Best Practices

### 1. Provide Context

Always tell Claude:
- What architecture you're using (Central Orchestrator)
- What files are relevant
- What you're trying to achieve

**Good prompt**:
```
In the Cryo Engine central orchestrator architecture, I'm adding a screenshot
feature to RendererModule. The module should handle a TAKE_SCREENSHOT message
from HttpModule. Show me how to implement this, including message handling and
Vulkan framebuffer readback.
```

**Bad prompt**:
```
How do I take a screenshot in Vulkan?
```

### 2. Reference Existing Code

Point Claude to similar code that already works:

```
I want to add a new module similar to AssetModule. Use AssetModule.cpp as a
template and create a TextureCompressionModule that compresses textures in
the background using the TaskScheduler.
```

### 3. Ask for Explanations

Don't just ask for code - ask Claude to explain the architecture:

```
Explain how the EventBus topological sort works in CentralAgent::topologicalSortModules().
Then show me how to add better error handling for circular dependencies.
```

### 4. Iterate and Refine

Use multi-turn conversations to refine solutions:

```
First turn: "Create a basic PhysicsModule"
Second turn: "Add collision detection support"
Third turn: "Optimize the collision detection for 1000+ entities"
Fourth turn: "Add debug visualization of collision shapes"
```

### 5. Validate Generated Code

Always review Claude's suggestions:
- Check for memory leaks
- Verify thread safety
- Ensure it follows existing patterns
- Test before integrating

---

## Prompt Templates

### Module Creation Template

```
Create a new module for Cryo Engine:

**Module Name**: [name]
**Purpose**: [what it does]
**Dependencies**: [other modules it depends on]
**Events Published**: [events it publishes]
**Events Subscribed**: [events it subscribes to]
**Public API**: [methods to expose]
**Configuration**: [config options needed]

Generate:
1. [ModuleName].hpp with class declaration
2. [ModuleName].cpp with implementation
3. Example usage in main.cpp
4. Unit tests

Follow the existing pattern from RendererModule and AssetModule.
```

### Debugging Template

```
Debug this issue in Cryo Engine:

**Module**: [module name]
**Problem**: [description of the issue]
**Error Message**: [paste error]
**Relevant Code**: [paste code snippet]
**What I've Tried**: [attempted fixes]

Help me:
1. Identify the root cause
2. Suggest a fix
3. Explain why it's happening
4. Prevent similar issues in the future
```

### Refactoring Template

```
Refactor this code in Cryo Engine:

**Current Code**: [paste code]
**Problems**: [what's wrong with it]
**Goals**: [what the refactored code should achieve]

Improve:
1. Performance (if applicable)
2. Readability
3. Maintainability
4. Error handling

Keep it consistent with the existing architecture and coding style.
```

### Documentation Template

```
Generate documentation for this Cryo Engine component:

**Component**: [name]
**Type**: [Module/Service/Utility]
**Files**: [list of files]

Include:
- Overview and purpose
- Architecture diagram (ASCII art)
- API reference with examples
- Configuration options
- Common use cases
- Troubleshooting guide

Format: Markdown, similar to existing README.md files.
```

---

## Advanced Claude Usage

### 1. Architectural Decision Making

**Prompt**:
```
I need to decide between two approaches for [feature]:

**Option A**: [description, pros, cons]
**Option B**: [description, pros, cons]

Given the Cryo Engine central orchestrator architecture, which approach is better?
Consider:
- Module coupling
- Performance
- Maintainability
- Consistency with existing code
```

### 2. Code Review

**Prompt**:
```
Review this module implementation for Cryo Engine:
[paste code]

Check for:
- Architectural compliance (does it follow IModule interface correctly?)
- Memory leaks
- Thread safety
- Error handling
- Performance issues
- Code style consistency
- Missing documentation
```

### 3. Migration Planning

**Prompt**:
```
I want to migrate [LegacySystem] to Cryo Engine's central orchestrator architecture.

Current system:
- [description of legacy architecture]
- [key components]
- [dependencies]

Create a migration plan:
1. What modules should I create?
2. What order should I migrate components in?
3. How do I ensure backward compatibility during migration?
4. What risks should I watch out for?
```

---

## Troubleshooting Claude Interactions

### If Claude Doesn't Understand Your Architecture

**Solution**: Share the architecture documentation explicitly:

```
Before answering, read these files in order:
1. cryo-engine/README.md
2. cryo-engine/ARCHITECTURE.md
3. cryo-engine/src/core/CentralAgent.hpp

Then answer my question about [topic].
```

### If Claude Suggests Code That Doesn't Fit the Architecture

**Solution**: Correct it and ask for an alternative:

```
That approach doesn't fit the central orchestrator pattern because [reason].
Instead, the module should [correct approach]. Regenerate the code following
this pattern.
```

### If Claude's Code Is Too Generic

**Solution**: Ask for specifics:

```
That code is too generic. Use the actual Cryo Engine classes:
- CentralAgent (singleton)
- IModule interface
- EventBus for pub/sub
- ConfigManager for config
- LogManager for logging

Regenerate using these specific classes.
```

---

## Summary

Claude Code is most effective when:
1. ✅ You provide **clear context** about Cryo Engine's architecture
2. ✅ You **reference existing code** as examples
3. ✅ You **iterate** on solutions through multi-turn conversations
4. ✅ You **validate** generated code before integrating
5. ✅ You use **structured prompts** (templates)

Claude struggles when:
1. ❌ Context is missing (architecture, file structure)
2. ❌ Prompts are too vague ("make it better")
3. ❌ You accept code without reviewing it
4. ❌ You don't point to existing patterns

**Best practice**: Treat Claude as a **collaborative pair programmer** who knows general C++ and Vulkan but needs to learn your specific Cryo Engine architecture through documentation and examples.

---

## Additional Resources

- [CentralAgent API Reference](./API_REFERENCE.md)
- [Module Development Guide](./MODULE_DEVELOPMENT.md)
- [Event Bus Patterns](./EVENT_BUS_PATTERNS.md)
- [Cryo Engine Examples](./examples/)

---

*Claude Code Usage Guide - Cryo Engine - Version 1.0*

**Generated with Claude Code** - Last updated: 2025-10-10