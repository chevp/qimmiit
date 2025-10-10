/**
 * Copyright (C) by Patrice Chevillat
 */

#pragma once

#ifndef IPCOE17
#define IPCOE17

// ============================================================================
// ⚠️ ARCHITECTURAL WARNING - LEGACY CODE
// ============================================================================
//
// This header includes gRPC client code that should NOT be in coregfx.
//
// ISSUE: coregfx should focus on local rendering and asset loading only.
// RECOMMENDATION: Move ipcoe17_client to apps/foundation/coregfx-stream-client/
// REFERENCE: See coregfx/ARCHITECTURE_ANALYSIS.md
//
// ============================================================================

#ifndef IPCOE17_CLIENT
#include "ipcoe17_client.hpp"  // ⚠️ WARNING: Legacy gRPC client code
#endif

#endif