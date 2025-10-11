/**
 * Data-Driven CoreGFX Renderer - Main Entry Point
 *
 * A gRPC-driven Vulkan renderer that receives real-time shader updates
 * from a backend server for dynamic visual control.
 */

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Dbghelp.lib")

// Define before including windows.h to prevent winsock.h inclusion
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "ConfigParser.hpp"
#include "Renderer.hpp"
#include "GrpcRendererClient.hpp"
#include <coregfx/core/ocean_log.hpp>
#include <cgfx.pb.h>
#include <windows.h>
#include <dbghelp.h>
#include <iostream>
#include <cstring>
#include <memory>
#include <fstream>
#include <ctime>
#include <sstream>

using namespace datadriven;

// Global Cryo error code tracking
cgfx::CryoErrorCode g_lastExitCode = cgfx::CRYO_SUCCESS;

// Write error report to file
void writeErrorReport(const std::string& errorMessage, cgfx::CryoErrorCode exitCode) {
    try {
        std::ofstream errorFile("error_report.log", std::ios::app);
        if (errorFile.is_open()) {
            // Get current timestamp
            std::time_t now = std::time(nullptr);
            char timestamp[100];
            std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", std::localtime(&now));

            errorFile << "==========================================\n";
            errorFile << "ERROR REPORT - data-driven-coregfx-renderer - " << timestamp << "\n";
            errorFile << "==========================================\n";
            errorFile << "CRYO ERROR CODE: " << static_cast<int>(exitCode) << " (" << cgfx::CryoErrorCode_Name(exitCode) << ")\n";
            errorFile << "Error Message: " << errorMessage << "\n";
            errorFile << "==========================================\n\n";
            errorFile.close();

            ocean::error("Error report written to error_report.log");
        }
    } catch (...) {
        ocean::error("Failed to write error report to file");
    }
}

/**
 * Parse command line arguments
 */
AppSettings parseCommandLine(int argc, char** argv) {
    AppSettings settings;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--config") == 0 && i + 1 < argc) {
            settings.configFile = argv[++i];
        }
        else if (strcmp(argv[i], "--no-grpc") == 0) {
            settings.enableGrpc = false;
        }
        else if (strcmp(argv[i], "--studio") == 0) {
            settings.studioMode = true;
        }
        else if (strcmp(argv[i], "--help") == 0) {
            std::cout << "Data-Driven CoreGFX Renderer\n\n";
            std::cout << "Usage: data-driven-coregfx-renderer [options] [config.xml]\n\n";
            std::cout << "Options:\n";
            std::cout << "  --config <file>     XML configuration file (default: index.arctic)\n";
            std::cout << "  --no-grpc           Disable gRPC client (offline mode)\n";
            std::cout << "  --studio            Enable studio/debug mode\n";
            std::cout << "  --help              Show this help message\n";
            exit(0);
        }
        else if (i == argc - 1) {
            // Last argument is config file path
            settings.configFile = argv[i];
        }
    }

    return settings;
}

/**
 * Main application logic (separated to allow SEH wrapper)
 */
int RunApplication(HINSTANCE hInstance, LPSTR lpCmdLine) {
    std::cout << "==========================================\n";
    std::cout << "Data-Driven CoreGFX Renderer\n";
    std::cout << "==========================================\n\n";

    // Parse command line (Win32 lpCmdLine to argc/argv conversion)
    std::vector<std::string> args;
    std::string cmdLineStr = lpCmdLine ? lpCmdLine : "";

    // If no command line args, default to index.arctic
    if (cmdLineStr.empty() || cmdLineStr.find_first_not_of(" \t\n\r") == std::string::npos) {
        args.push_back("index.arctic");
    } else {
        // Simple tokenization (doesn't handle quoted strings, but sufficient for now)
        std::istringstream iss(cmdLineStr);
        std::string arg;
        while (iss >> arg) {
            args.push_back(arg);
        }
    }

    // Convert to argc/argv
    int argc = static_cast<int>(args.size()) + 1;
    std::vector<char*> argv;
    argv.push_back(const_cast<char*>("data-driven-coregfx-renderer.exe"));
    for (auto& a : args) {
        argv.push_back(const_cast<char*>(a.c_str()));
    }

    // Parse command line arguments
    AppSettings settings = parseCommandLine(argc, argv.data());

    // Step 0: Load arctic.config.xml to initialize ConfigProvider (MUST be done first!)
    ocean::info("Loading CoreGFX configuration...");
    if (!ConfigParser::loadCoregfxConfig(settings.coregfxConfigFile)) {
        std::string errorMsg = "Failed to load arctic.config.xml: " + settings.coregfxConfigFile;
        ocean::error(errorMsg);
        writeErrorReport(errorMsg, cgfx::CRYO_ERROR_CONFIG_LOAD_FAILED);
        return static_cast<int>(cgfx::CRYO_ERROR_CONFIG_LOAD_FAILED);
    }

    // Step 1: Load scene configuration (arctic.xml)
    SceneConfig config = ConfigParser::loadConfig(settings.configFile);

    if (!config.isValid) {
        std::string errorMsg = "Failed to load configuration: " + settings.configFile;
        ocean::error(errorMsg);
        for (const auto& error : config.errors) {
            ocean::error("  - " + error);
            errorMsg += "\n  - " + error;
        }
        writeErrorReport(errorMsg, cgfx::CRYO_ERROR_CONFIG_LOAD_FAILED);
        return static_cast<int>(cgfx::CRYO_ERROR_CONFIG_LOAD_FAILED);
    }

    // Step 2: Create and initialize renderer
    std::unique_ptr<Renderer> renderer;

    try {
        // Pass argc/argv to renderer for OceanPbrApp constructor
        renderer = std::make_unique<Renderer>(settings, argc, argv.data());

        // Set scene config BEFORE initialization (renderer needs it during init)
        renderer->setSceneConfig(config);

        if (!renderer->initialize(hInstance)) {
            std::string errorMsg = "Failed to initialize renderer";
            ocean::error(errorMsg);
            writeErrorReport(errorMsg, cgfx::CRYO_ERROR_RENDERER_INIT_FAILED);
            return static_cast<int>(cgfx::CRYO_ERROR_RENDERER_INIT_FAILED);
        }

    } catch (const std::exception& e) {
        std::string errorMsg = "Exception during renderer initialization: " + std::string(e.what());
        ocean::error(errorMsg);
        writeErrorReport(errorMsg, cgfx::CRYO_ERROR_CRITICAL_RENDERER_ERROR);
        return static_cast<int>(cgfx::CRYO_ERROR_CRITICAL_RENDERER_ERROR);
    } catch (...) {
        std::string errorMsg = "Unknown exception during renderer initialization";
        ocean::error(errorMsg);
        writeErrorReport(errorMsg, cgfx::CRYO_ERROR_GENERIC_FAILURE);
        return static_cast<int>(cgfx::CRYO_ERROR_GENERIC_FAILURE);
    }

    // Step 3: Scene is already loaded during initialization (no separate loadScene call needed)

    // Step 4: Initialize gRPC client (optional)
    std::unique_ptr<GrpcRendererClient> grpcClient;

    if (settings.enableGrpc) {
        ocean::info("Initializing gRPC client...");
        grpcClient = std::make_unique<GrpcRendererClient>(config.backend);

        // Set up callbacks for gRPC updates
        grpcClient->setShaderUpdateCallback([&](const arctic::network::ShaderUpdate& update) {
            ocean::info("Received shader update from server: " + update.shader_instance_id());
            renderer->updateShaderInstance(update);
        });

        grpcClient->setSceneUpdateCallback([&](const arctic::network::SceneUpdate& update) {
            ocean::info("Received scene update from server: " + update.scene_id());
            // TODO: Reload scene
            ocean::warn("Scene reloading not yet implemented");
        });

        grpcClient->setUIStyleUpdateCallback([&](const arctic::network::UIStyleUpdate& update) {
            ocean::info("Received UI style update from server: " + update.widget_id());
            // TODO: Apply UI style changes
            ocean::warn("UI style updates not yet implemented");
        });

        if (!grpcClient->connect()) {
            ocean::warn("Failed to connect to gRPC server - continuing in offline mode");
        } else {
            ocean::info("gRPC client connected successfully");
        }
    } else {
        ocean::info("gRPC client disabled (offline mode)");
    }

    // Step 5: Main rendering loop
    ocean::info("\n=== Entering render loop ===\n");

    MSG msg = {};
    bool running = true;

    try {
        while (running && !renderer->shouldClose()) {
            // Process Win32 messages
            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                if (msg.message == WM_QUIT) {
                    running = false;
                    break;
                }
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }

            if (!running) break;

            // Render frame
            renderer->renderFrame();

            // Send telemetry to gRPC server (every 60 frames to reduce network traffic)
            if (grpcClient && grpcClient->isConnected() && (renderer->getFrameCount() % 60 == 0)) {
                grpcClient->sendFrameRendered(
                    renderer->getFrameCount(),
                    renderer->getLastFrameTime()
                );
            }
        }
    } catch (const std::exception& e) {
        std::string errorMsg = "Fatal error during render loop: " + std::string(e.what());
        ocean::error(errorMsg);
        writeErrorReport(errorMsg, cgfx::CRYO_ERROR_CRITICAL_RENDERER_ERROR);

        // Attempt cleanup before exit
        try {
            if (grpcClient) grpcClient->disconnect();
            if (renderer) renderer->shutdown();
        } catch (...) {
            ocean::error("Error during emergency cleanup");
        }

        return static_cast<int>(cgfx::CRYO_ERROR_CRITICAL_RENDERER_ERROR);
    } catch (...) {
        std::string errorMsg = "Unknown fatal error during render loop";
        ocean::error(errorMsg);
        writeErrorReport(errorMsg, cgfx::CRYO_ERROR_CRITICAL_RENDERER_ERROR);

        // Attempt cleanup before exit
        try {
            if (grpcClient) grpcClient->disconnect();
            if (renderer) renderer->shutdown();
        } catch (...) {
            ocean::error("Error during emergency cleanup");
        }

        return static_cast<int>(cgfx::CRYO_ERROR_CRITICAL_RENDERER_ERROR);
    }

    ocean::info("\n=== Shutting down ===\n");

    // Cleanup
    if (grpcClient) {
        grpcClient->disconnect();
    }

    renderer->shutdown();

    ocean::info("Data-driven renderer completed successfully");
    return static_cast<int>(cgfx::CRYO_SUCCESS);
}

/**
 * Capture stack trace with symbols (Windows x86)
 */
std::string captureStackTrace(CONTEXT* context) {
    std::ostringstream oss;

    HANDLE process = GetCurrentProcess();
    HANDLE thread = GetCurrentThread();

    // Initialize symbol handler
    SymInitialize(process, NULL, TRUE);
    SymSetOptions(SYMOPT_LOAD_LINES | SYMOPT_UNDNAME);

    // Setup stack frame
    STACKFRAME64 stackFrame = {};
#ifdef _M_X64
    stackFrame.AddrPC.Offset = context->Rip;
    stackFrame.AddrFrame.Offset = context->Rbp;
    stackFrame.AddrStack.Offset = context->Rsp;
    DWORD machineType = IMAGE_FILE_MACHINE_AMD64;
#else
    stackFrame.AddrPC.Offset = context->Eip;
    stackFrame.AddrFrame.Offset = context->Ebp;
    stackFrame.AddrStack.Offset = context->Esp;
    DWORD machineType = IMAGE_FILE_MACHINE_I386;
#endif
    stackFrame.AddrPC.Mode = AddrModeFlat;
    stackFrame.AddrFrame.Mode = AddrModeFlat;
    stackFrame.AddrStack.Mode = AddrModeFlat;

    oss << "\n========== STACK TRACE ==========\n";

    int frameNum = 0;
    while (frameNum < 25) {
        if (!StackWalk64(
            machineType,
            process,
            thread,
            &stackFrame,
            context,
            NULL,
            SymFunctionTableAccess64,
            SymGetModuleBase64,
            NULL)) {
            break;
        }

        if (stackFrame.AddrPC.Offset == 0) {
            break;
        }

        // Get symbol information
        DWORD64 displacement = 0;
        char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
        PSYMBOL_INFO pSymbol = (PSYMBOL_INFO)buffer;
        pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        pSymbol->MaxNameLen = MAX_SYM_NAME;

        // Get line information
        IMAGEHLP_LINE64 line = {};
        line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
        DWORD lineDisplacement = 0;

        oss << "#" << frameNum << " ";

        if (SymFromAddr(process, stackFrame.AddrPC.Offset, &displacement, pSymbol)) {
            oss << pSymbol->Name;

            if (SymGetLineFromAddr64(process, stackFrame.AddrPC.Offset, &lineDisplacement, &line)) {
                oss << " at " << line.FileName << ":" << line.LineNumber;
            }
        } else {
            oss << "0x" << std::hex << stackFrame.AddrPC.Offset << std::dec;
        }

        oss << "\n";
        frameNum++;
    }

    oss << "=================================\n";

    SymCleanup(process);
    return oss.str();
}

/**
 * Unhandled Exception Filter (global crash handler)
 */
LONG WINAPI UnhandledExceptionHandler(EXCEPTION_POINTERS* exceptionInfo) {
    DWORD exceptionCode = exceptionInfo->ExceptionRecord->ExceptionCode;
    PVOID exceptionAddress = exceptionInfo->ExceptionRecord->ExceptionAddress;

    char errorMsg[512];
    char hexCode[20];
    const char* exceptionType = "Unknown";

    sprintf(hexCode, "%X", exceptionCode);
    sprintf(errorMsg, "FATAL CRASH - Windows Structured Exception: 0x%s", hexCode);

    // Common exception codes
    if (exceptionCode == 0xC0000005) {
        exceptionType = "ACCESS_VIOLATION (null pointer or invalid memory access)";
    } else if (exceptionCode == 0xC000001D) {
        exceptionType = "ILLEGAL_INSTRUCTION";
    } else if (exceptionCode == 0xC0000094) {
        exceptionType = "INTEGER_DIVIDE_BY_ZERO";
    }

    // Capture stack trace
    std::string stackTrace = captureStackTrace(exceptionInfo->ContextRecord);

    // Write to console
    fprintf(stderr, "==========================================\n");
    fprintf(stderr, "FATAL CRASH DETECTED\n");
    fprintf(stderr, "==========================================\n");
    fprintf(stderr, "Exception Code: 0x%s\n", hexCode);
    fprintf(stderr, "Type: %s\n", exceptionType);
    fprintf(stderr, "Address: 0x%p\n", exceptionAddress);
    fprintf(stderr, "%s", stackTrace.c_str());

    // Write error report
    char fullErrorMsg[4096];
    sprintf(fullErrorMsg, "%s\nType: %s\nAddress: 0x%p\n%s",
            errorMsg, exceptionType, exceptionAddress, stackTrace.c_str());
    writeErrorReport(fullErrorMsg, cgfx::CRYO_ERROR_CRITICAL_RENDERER_ERROR);

    // Don't show message box - error is logged to error_report.log and console
    // (Removed popup to avoid 20+ popups during debugging)

    return EXCEPTION_EXECUTE_HANDLER;
}

/**
 * Win32 entry point
 */
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Install global crash handler
    SetUnhandledExceptionFilter(UnhandledExceptionHandler);

    return RunApplication(hInstance, lpCmdLine);
}
