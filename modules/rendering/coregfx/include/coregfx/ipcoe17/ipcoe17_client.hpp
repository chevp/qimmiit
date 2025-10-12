/**
 * Copyright (C) by Patrice Chevillat
 */

#pragma once

#ifndef IPCOE17_CLIENT
#define IPCOE17_CLIENT

// ============================================================================
// ⚠️ ARCHITECTURAL WARNING - LEGACY CODE
// ============================================================================
//
// This file contains gRPC client code that VIOLATES the architectural
// separation of concerns for the coregfx library.
//
// ISSUE:
// - coregfx should ONLY handle local asset loading and Vulkan rendering
// - Remote communication (gRPC/HTTP) should be in separate client libraries
//
// RECOMMENDATION:
// - Move this file to: apps/foundation/coregfx-stream-client/include/
// - Remove gRPC dependencies from coregfx/CMakeLists.txt
//
// REFERENCE:
// - See: docs/remote-asset-loading-architecture.md
// - See: coregfx/ARCHITECTURE_ANALYSIS.md
//
// STATUS: Legacy code - marked for refactoring
// ============================================================================

#include <string>
#include <string_view>
#include <coregfx/core/ocean_log.hpp>
#include <grpcpp/grpcpp.h>  // ⚠️ WARNING: gRPC dependency should NOT be in coregfx
#include <coregfx/protobuf_wrapper.hpp>

namespace cgfx
{
    // ⚠️ WARNING: This gRPC client violates coregfx architectural boundaries
    // Should be moved to: apps/foundation/coregfx-stream-client/
    class Client
    {
    public:
        Client(const std::string& address);
        ~Client();

        bool connect();
        void disconnect();

        // Add other public methods as needed

    private:
        std::string server_address;
        std::shared_ptr<grpc::Channel> channel;
        // Add other private members as needed
    };
}

#endif