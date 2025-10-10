#ifdef _WIN32

#include <coregfx/util/platform/windows_platform.hpp>
#include <coregfx/core/ocean_keycodes.hpp>
#include <coregfx/core/ocean_log.hpp>
#include <iostream>

#ifndef SOLUTION_NAME
#define SOLUTION_NAME "Arctic Game Client"
#endif

namespace coregfx {
namespace platform {

HWND WindowsPlatform::setupWindow(HINSTANCE hinstance, WNDPROC wndproc, const WindowSettings& settings) {
    std::string className = SOLUTION_NAME;  // Use the same name as registered

    if (!registerWindowClass(hinstance, wndproc, className)) {
        return nullptr;
    }

    return createWindowInternal(hinstance, std::wstring(className.begin(), className.end()), settings);
}

bool WindowsPlatform::registerWindowClass(HINSTANCE hinstance, WNDPROC wndproc, const std::string& className) {
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = wndproc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hinstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName = NULL;

    std::string s = SOLUTION_NAME;
    std::wstring stemp = std::wstring(s.begin(), s.end());
    wc.lpszClassName = stemp.c_str();
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    wc.style = CS_HREDRAW | CS_VREDRAW;

    if (!RegisterClassExW(&wc)) {
        int lastError = GetLastError();
        ocean::error("Could not register window class! Error: " + std::to_string(lastError));
        return false;
    }

    return true;
}

HWND WindowsPlatform::createWindowInternal(HINSTANCE hinstance, const std::wstring& className,
                                         const WindowSettings& settings) {
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // Handle fullscreen mode
    if (settings.fullscreen) {
        DEVMODE dmScreenSettings = {};
        dmScreenSettings.dmSize = sizeof(dmScreenSettings);
        dmScreenSettings.dmPelsWidth = screenWidth;
        dmScreenSettings.dmPelsHeight = screenHeight;
        dmScreenSettings.dmBitsPerPel = 32;
        dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

        if ((settings.width != (uint32_t)screenWidth) && (settings.height != (uint32_t)screenHeight)) {
            if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL) {
                ocean::error("Fullscreen Mode not supported! Falling back to window mode.");
                // Note: This modifies the const settings, which is not ideal
                // In a real refactor, we'd return a status or use a mutable copy
            }
        }
    }

    DWORD dwExStyle = settings.fullscreen ? WS_EX_APPWINDOW : (WS_EX_APPWINDOW | WS_EX_WINDOWEDGE);
    DWORD dwStyle = settings.fullscreen ?
                   (WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN) :
                   (WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);

    RECT windowRect = {};
    windowRect.left = 0L;
    windowRect.top = 0L;
    windowRect.right = settings.fullscreen ? (long)screenWidth : (long)settings.width;
    windowRect.bottom = settings.fullscreen ? (long)screenHeight : (long)settings.height;

    AdjustWindowRectEx(&windowRect, dwStyle, FALSE, dwExStyle);

    std::wstring windowTitle = std::wstring(settings.name.begin(), settings.name.end());
    HWND window = CreateWindowExW(WS_EX_ACCEPTFILES,
                                 className.c_str(),
                                 windowTitle.c_str(),
                                 dwStyle | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
                                 0, 0,
                                 windowRect.right - windowRect.left,
                                 windowRect.bottom - windowRect.top,
                                 NULL, NULL, hinstance, NULL);

    if (!settings.fullscreen && window) {
        uint32_t x = (GetSystemMetrics(SM_CXSCREEN) - windowRect.right) / 2;
        uint32_t y = (GetSystemMetrics(SM_CYSCREEN) - windowRect.bottom) / 2;
        SetWindowPos(window, 0, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
    }

    if (!window) {
        ocean::error("Could not create window!");
        return nullptr;
    }

    ShowWindow(window, SW_SHOW);
    SetForegroundWindow(window);
    SetFocus(window);

    return window;
}

void WindowsPlatform::sendKey(HWND window, UINT uMsg) {
    // Implementation would need access to the main window's message handler
    // This is a placeholder for the refactored version
}

void WindowsPlatform::handleWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
                                        bool& paused, bool& prepared, void* cameraPtr) {
    // Cast the camera pointer - in real implementation we'd use a proper interface
    // For now, this is a placeholder showing the structure

    switch (uMsg) {
        case WM_CLOSE:
            prepared = false;
            DestroyWindow(hWnd);
            PostQuitMessage(0);
            break;

        case WM_PAINT:
            ValidateRect(hWnd, NULL);
            break;

        case WM_KEYDOWN:
            switch (wParam) {
                case KEY_P:
                    paused = !paused;
                    break;
                case KEY_ESCAPE:
                    PostQuitMessage(0);
                    break;
            }
            // Camera handling would be implemented with proper interface
            break;

        case WM_KEYUP:
            // Camera key releases would be handled here
            break;

        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MBUTTONUP:
            // Mouse button handling
            break;

        case WM_MOUSEWHEEL:
            // Mouse wheel handling
            break;

        case WM_MOUSEMOVE:
            // Mouse movement handling
            break;

        case WM_SIZE:
            // Window resize handling
            break;

        case WM_ENTERSIZEMOVE:
            // Start resize mode
            break;

        case WM_EXITSIZEMOVE:
            // End resize mode
            break;

        case WM_DROPFILES:
            // File drop handling
            break;
    }
}

}} // namespace coregfx::platform

#endif // _WIN32