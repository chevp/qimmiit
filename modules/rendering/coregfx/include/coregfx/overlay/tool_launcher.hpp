/**
 * Tool Launcher - Manages launching and tracking external Electron-based tools
 *
 * Provides functionality to:
 * - Launch Electron tools from ImGui menu
 * - Track running tool processes
 * - Focus/activate existing tool windows
 * - Automatically open tools in Chrome browser
 */

#pragma once

#include <string>
#include <map>
#include <vector>
#include <memory>
#include <functional>

#ifdef _WIN32
#include <windows.h>
#endif

namespace toollaunch {

    /**
     * Tool definition structure
     */
    struct ToolDefinition {
        std::string id;                  // Unique identifier (e.g., "arctic-studio-prime")
        std::string displayName;         // Menu display name (e.g., "Scene Editor")
        std::string path;                // Path to tool directory
        std::string url;                 // URL when running (e.g., "http://localhost:3001")
        int port;                        // HTTP port
        std::string startCommand;        // Command to start tool (e.g., "npm start")
        std::string icon;                // Icon identifier for menu
        bool launchInBrowser;            // Open in Chrome vs Electron window
    };

    /**
     * Tool process state
     */
    struct ToolProcess {
        std::string toolId;
        HANDLE processHandle;
        DWORD processId;
        bool isRunning;
        std::string url;
        time_t startTime;
    };

    /**
     * Tool Launcher Manager
     *
     * Manages external tool processes and browser windows
     */
    class ToolLauncher {
    public:
        ToolLauncher();
        ~ToolLauncher();

        // Initialization
        void initialize();
        void loadToolRegistry(const std::string& registryPath);
        void shutdown();

        // Configuration
        void setBrowserPath(const std::string& path);
        std::string getBrowserPath() const;

        // Tool management
        bool launchTool(const std::string& toolId);
        bool isToolRunning(const std::string& toolId);
        bool focusTool(const std::string& toolId);
        void stopTool(const std::string& toolId);
        void stopAllTools();

        // Query
        std::vector<ToolDefinition> getAllTools() const;
        ToolDefinition* getToolById(const std::string& toolId);
        std::vector<std::string> getRunningTools() const;

        // ImGui rendering
        void renderToolsMenu();

    private:
        // Internal helpers
        bool startProcess(const std::string& workingDir, const std::string& command, HANDLE& outHandle, DWORD& outPid);
        bool openInBrowser(const std::string& url);
        bool isProcessRunning(HANDLE processHandle);
        bool waitForToolReady(const std::string& url, int timeoutSeconds = 10);
        bool focusWindow(const std::string& windowTitle);
        void detectBrowserPath();

        // Tool registry
        std::map<std::string, ToolDefinition> m_tools;
        std::map<std::string, ToolProcess> m_runningTools;

        // Configuration
        std::string m_chromePath;
        std::string m_projectRoot;
    };

    /**
     * Global tool launcher instance
     */
    extern std::unique_ptr<ToolLauncher> g_toolLauncher;

} // namespace toollaunch