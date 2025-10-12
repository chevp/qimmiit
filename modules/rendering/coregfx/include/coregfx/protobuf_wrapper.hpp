/**
 * Copyright (C) by Patrice Chevillat
 * 
 * Wrapper header to suppress protobuf warnings
 */

#pragma once

#ifndef OCEAN_PROTOBUF_WRAPPER_HPP_
#define OCEAN_PROTOBUF_WRAPPER_HPP_

// Disable protobuf-related warnings for MSVC
#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable: 4251) // class needs to have dll-interface
    #pragma warning(disable: 4275) // non dll-interface class used as base
    #pragma warning(disable: 4996) // deprecated functions
    #pragma warning(disable: 4244) // conversion with possible loss of data
    #pragma warning(disable: 4267) // size_t to int conversion
    #pragma warning(disable: 4100) // unreferenced formal parameter
    #pragma warning(disable: 4127) // conditional expression is constant
    #pragma warning(disable: 4125) // decimal digit terminates octal escape sequence
    #pragma warning(disable: 4146) // unary minus operator applied to unsigned type
    #pragma warning(disable: 4800) // forcing value to bool 'true' or 'false'
#endif

// Include commonly used protobuf headers here
#include "cgfx.pb.h"
#include "nyx1.pb.h"
#include "lunara1.pb.h"
#include "lunara2.pb.h"

// Re-enable warnings after protobuf includes
#ifdef _MSC_VER
    #pragma warning(pop)
#endif

#endif // OCEAN_PROTOBUF_WRAPPER_HPP_