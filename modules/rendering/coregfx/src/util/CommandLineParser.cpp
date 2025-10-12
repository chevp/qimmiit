#include <coregfx/util/CommandLineParser.hpp>
#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>

namespace coregfx {

AppSettings CommandLineParser::parseApplicationArgs(const std::string& cmdLine) {
    AppSettings settings;

    if (cmdLine.empty()) {
        return settings;
    }

    auto tokens = tokenize(cmdLine);

    // Parse each argument
    for (size_t i = 0; i < tokens.size(); ++i) {
        const std::string& arg = tokens[i];

        if (arg == "--help" || arg == "-h" || arg == "--spec" || arg == "--specification") {
            settings.showSpecification = true;
        }
        else if (arg == "--camera-pos" && i + 3 < tokens.size()) {
            settings.customCamera = true;
            settings.camPosX = std::stof(tokens[++i]);
            settings.camPosY = std::stof(tokens[++i]);
            settings.camPosZ = std::stof(tokens[++i]);
        }
        else if (arg == "--camera-rot" && i + 3 < tokens.size()) {
            settings.customCamera = true;
            settings.camRotX = std::stof(tokens[++i]);
            settings.camRotY = std::stof(tokens[++i]);
            settings.camRotZ = std::stof(tokens[++i]);
        }
        else if (arg == "--http-server") {
            settings.enableHttpServer = true;
        }
        else if (arg == "--http-port" && i + 1 < tokens.size()) {
            settings.enableHttpServer = true;
            settings.httpPort = std::stoi(tokens[++i]);
            settings.httpPortSetByCommandLine = true;
        }
        else if (arg == "--dump-state") {
            settings.dumpState = true;
            if (i + 1 < tokens.size()) {
                settings.dumpStateFile = tokens[++i];
            }
        }
        else if (arg == "--dump-delay" && i + 1 < tokens.size()) {
            settings.dumpDelaySeconds = std::stoi(tokens[++i]);
        }
        else if (arg == "--edit-mode") {
            settings.editMode = true;
            settings.editModeSetByCommandLine = true;
        }
        else if (arg == "--headless") {
            settings.headlessMode = true;
            settings.headlessModeSetByCommandLine = true;
        }
        else if (arg == "--auto-close") {
            settings.autoClose = true;
            settings.autoCloseSetByCommandLine = true;
        }
        else if (arg == "--timeout" && i + 1 < tokens.size()) {
            settings.autoCloseTimeout = std::stoi(tokens[++i]);
            settings.autoCloseTimeoutSetByCommandLine = true;
        }
        else if (arg == "--mock-mode") {
            settings.mockMode = true;
        }
        // Skip Vulkan logging arguments (handled by VulkanStateLogger)
        else if (arg.find("--vulkan-") == 0) {
            if (arg == "--vulkan-log-level" || arg == "--vulkan-log-format" ||
                arg == "--vulkan-log-file" || arg == "--vulkan-compact-file") {
                if (i + 1 < tokens.size()) {
                    ++i;  // Skip the argument value
                }
            }
            continue;
        }
        else if (!arg.empty() && arg[0] == '-') {
            std::cerr << "Warning: Unknown argument '" << arg << "' ignored" << std::endl;
        }
        else if (!arg.empty() && arg[0] != '-') {
            // Set input file if not already set
            if (settings.inputFile == "index.elyrion.xml") {
                std::string cleanArg = arg;
                // Remove surrounding quotes if present
                if (cleanArg.length() >= 2 && cleanArg.front() == '"' && cleanArg.back() == '"') {
                    cleanArg = cleanArg.substr(1, cleanArg.length() - 2);
                }
                if (cleanArg.length() >= 4 && cleanArg.substr(0, 2) == "\\\"" && cleanArg.substr(cleanArg.length() - 2) == "\\\"") {
                    cleanArg = cleanArg.substr(2, cleanArg.length() - 4);
                }
                settings.inputFile = cleanArg;
            }
        }
    }

    return settings;
}

void CommandLineParser::printSpecification() {
    std::string helpContent;

    // Try to read from specification file first
    std::ifstream specFile("RENDERER_SPECIFICATION.md");
    if (specFile.good()) {
        std::string line;
        while (std::getline(specFile, line)) {
            helpContent += line + "\n";
        }
        specFile.close();
    } else {
        // Fallback to built-in specification
        helpContent =
            "# Arctic Game Client - Elyrion CoreGFX Renderer\n\n"
            "## Command Line Usage\n"
            "elyrion.coregfx.renderer.exe [input_file] [options]\n\n"
            "## Basic Options\n"
            "- --help, -h: Show help\n"
            "- --spec: Show specification\n"
            "- --camera-pos X Y Z: Set camera position\n"
            "- --camera-rot X Y Z: Set camera rotation\n"
            "- --http-server: Enable HTTP REST API server on port 52009\n"
            "- --http-port PORT: Enable HTTP server on custom port\n"
            "- --dump-state [filename]: Dump full Vulkan and entity state to compact SQL log\n"
            "- --dump-delay SECONDS: Time delay before dumping state (default: 5s)\n"
            "- --edit-mode: Enable studio mode with menu bar and advanced features\n"
            "- --headless: Run without GUI window (console/API only mode)\n"
            "- --auto-close: Automatically close window after operations\n"
            "- --timeout N: Auto-close timeout in seconds (requires --auto-close)\n"
            "- --mock-mode: Use mock renderer for fast testing (no Vulkan initialization)\n\n"
            "For complete specification, ensure RENDERER_SPECIFICATION.md is in the working directory.\n";
    }

    // Output to console and file
    std::cout << helpContent;
    std::cout.flush();

    // Also use printf to ensure output
    printf("%s", helpContent.c_str());
    fflush(stdout);

    std::ofstream helpFile("help.txt");
    if (helpFile.is_open()) {
        helpFile << helpContent;
        helpFile.close();
    }
}

std::vector<std::string> CommandLineParser::tokenize(const std::string& cmdLine) {
    std::vector<std::string> tokens;
    std::istringstream iss(cmdLine);
    std::string token;
    while (iss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}} // namespace coregfx
