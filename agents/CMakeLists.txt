cmake_minimum_required(VERSION 3.15)

# Agents are independent executables
# They communicate via gRPC/HTTP only - NO shared C++ code

# Optional common protobuf definitions (health checks, etc.)
add_subdirectory(common)

# Individual agents
add_subdirectory(umiak-agent)

# Add more agents here as needed
# add_subdirectory(other-agent)
