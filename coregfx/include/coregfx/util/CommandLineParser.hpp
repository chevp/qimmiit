#pragma once

#include <string>
#include <vector>

namespace coregfx {

struct AppSettings {
    std::string inputFile = "index.elyrion.xml";
    bool showSpecification = false;
    bool customCamera = false;
    float camPosX = 0.0f, camPosY = 0.0f, camPosZ = 10.0f;
    float camRotX = 0.0f, camRotY = 0.0f, camRotZ = 0.0f;
    bool enableHttpServer = false;
    int httpPort = 52009;
    bool dumpState = false;
    std::string dumpStateFile = "state_dump.log";
    int dumpDelaySeconds = 5;
    bool editMode = false;
    bool headlessMode = false;
    bool autoClose = false;
    int autoCloseTimeout = 5;
    bool mockMode = false;

    // Flags to track what was explicitly set via command line (for config override logic)
    bool httpPortSetByCommandLine = false;
    bool editModeSetByCommandLine = false;
    bool headlessModeSetByCommandLine = false;
    bool autoCloseSetByCommandLine = false;
    bool autoCloseTimeoutSetByCommandLine = false;
};

class CommandLineParser {
public:
    static AppSettings parseApplicationArgs(const std::string& cmdLine);
    static void printSpecification();

private:
    static std::vector<std::string> tokenize(const std::string& cmdLine);
};

} // namespace coregfx