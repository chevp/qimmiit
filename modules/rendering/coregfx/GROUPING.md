1. Group related small components into a single utility library

Instead of 5 different tiny libraries, bundle related ones together, for example:

CoreGFX-Utils

Thread Pool

Resource Manager

Input Handler
These are all general-purpose utilities, no Vulkan or gRPC.

CoreGFX-IO

GLTF Loader

XML Parser (once protobuf dependency is resolved)

CoreGFX-Vulkan

All Vulkan-dependent code.

CoreGFX-gRPC

All protobuf/gRPC dependent code.

This reduces the number of tiny libraries while still keeping concerns separated.