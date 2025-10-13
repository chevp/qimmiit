/**
 * Content CLI Tool
 * Command-line interface for managing game content
 */

#include <iostream>
#include <string>
#include <vector>
#include "../validation/content_validator.hpp"

using namespace avannaaq::cms;

void printUsage(const char* progName) {
    std::cout << "Avannaaq Content CLI\n";
    std::cout << "=====================\n\n";
    std::cout << "Usage: " << progName << " <command> [options]\n\n";
    std::cout << "Commands:\n";
    std::cout << "  validate <path>    Validate content files\n";
    std::cout << "  list [type]        List all content\n";
    std::cout << "  info <file>        Show info about a content file\n";
    std::cout << "  compile <file>     Compile .pbtxt to binary .pb\n";
    std::cout << "  help               Show this help message\n";
    std::cout << "\nExamples:\n";
    std::cout << "  " << progName << " validate content/\n";
    std::cout << "  " << progName << " list items\n";
    std::cout << "  " << progName << " info content/worlds/overworld.pbtxt\n";
}

int cmdValidate(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cerr << "Error: validate command requires a path\n";
        return 1;
    }

    std::string path = args[0];
    std::cout << "Validating content at: " << path << "\n\n";

    ContentValidator validator;
    auto result = validator.validateAllContent(path);

    if (result.success) {
        std::cout << "\n✓ Validation successful!\n";
        if (!result.warnings.empty()) {
            std::cout << "\nWarnings:\n";
            for (const auto& warning : result.warnings) {
                std::cout << "  ⚠ " << warning << "\n";
            }
        }
        return 0;
    } else {
        std::cout << "\n✗ Validation failed!\n\n";
        std::cout << "Errors:\n";
        for (const auto& error : result.errors) {
            std::cout << "  ✗ " << error << "\n";
        }
        return 1;
    }
}

int cmdList(const std::vector<std::string>& args) {
    std::string type = args.empty() ? "all" : args[0];

    std::cout << "Listing content: " << type << "\n";
    // TODO: Implement content listing

    return 0;
}

int cmdInfo(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cerr << "Error: info command requires a file path\n";
        return 1;
    }

    std::string file = args[0];
    std::cout << "Content info for: " << file << "\n";
    // TODO: Implement content info display

    return 0;
}

int cmdCompile(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cerr << "Error: compile command requires a file path\n";
        return 1;
    }

    std::string file = args[0];
    std::cout << "Compiling: " << file << "\n";
    // TODO: Implement .pbtxt to .pb compilation

    return 0;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    std::string command = argv[1];
    std::vector<std::string> args;

    for (int i = 2; i < argc; ++i) {
        args.push_back(argv[i]);
    }

    try {
        if (command == "validate") {
            return cmdValidate(args);
        } else if (command == "list") {
            return cmdList(args);
        } else if (command == "info") {
            return cmdInfo(args);
        } else if (command == "compile") {
            return cmdCompile(args);
        } else if (command == "help" || command == "--help" || command == "-h") {
            printUsage(argv[0]);
            return 0;
        } else {
            std::cerr << "Unknown command: " << command << "\n\n";
            printUsage(argv[0]);
            return 1;
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
