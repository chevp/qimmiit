# Modules Directory
# Contains independent modules that communicate via orchestrators

cmake_minimum_required(VERSION 3.20)

# Rendering modules
add_subdirectory(rendering/coregfx)
add_subdirectory(rendering/data-driven-renderer)

# Asset modules (cryo-cache must come before cryo-asset)
add_subdirectory(assets/cryo-cache)
add_subdirectory(assets/cryo-asset)

# Tooling modules
add_subdirectory(toolings/akutik)
add_subdirectory(toolings/cryo-tooling)