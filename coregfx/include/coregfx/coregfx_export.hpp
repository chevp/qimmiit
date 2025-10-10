#pragma once

#ifdef COREGFX_EXPORTS
    #define COREGFX_API __declspec(dllexport)
#else
    #ifdef COREGFX_STATIC
        #define COREGFX_API
    #else
        #define COREGFX_API __declspec(dllimport)
    #endif
#endif