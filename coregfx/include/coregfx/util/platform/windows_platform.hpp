#pragma once

#ifdef _WIN32

#include <Windows.h>
#include <string>
#include <cstdint>

namespace coregfx {
namespace platform {

class WindowsPlatform {
public:
    struct WindowSettings {
        bool fullscreen = false;
        uint32_t width = 1280;
        uint32_t height = 720;
        std::string name = "Arctic Game Client";
    };

    static HWND setupWindow(HINSTANCE hinstance, WNDPROC wndproc, const WindowSettings& settings);
    static void handleWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
                                   bool& paused, bool& prepared, void* camera);
    static void sendKey(HWND window, UINT uMsg);

private:
    static bool registerWindowClass(HINSTANCE hinstance, WNDPROC wndproc, const std::string& className);
    static HWND createWindowInternal(HINSTANCE hinstance, const std::wstring& className,
                                   const WindowSettings& settings);
};

}} // namespace coregfx::platform

#endif // _WIN32