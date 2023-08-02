// This must be included before many other Windows headers.
#pragma once

#include <Windows.h>

#include <shobjidl_core.h>

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>

#include <dwmapi.h>
#include <map>
#include <memory>

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "shcore.lib")
#pragma comment(lib, "Gdi32.lib")

const double kBaseDpi = 96.0;

const char kFrameKey[] = "frame";
const char kVisibleFrameKey[] = "visibleFrame";
const char kScaleFactorKey[] = "scaleFactor";
const char kScreenKey[] = "screen";

namespace {

    const flutter::EncodableValue* ValueOrNull(const flutter::EncodableMap& map,
                                               const char* key) {
        auto it = map.find(flutter::EncodableValue(key));
        if (it == map.end()) {
            return nullptr;
        }
        return &(it->second);
    }

    class ScreenManager {
    public:
        ScreenManager();

        virtual ~ScreenManager();

        HWND native_window;

        bool is_frameless_ = false;
        bool is_resizable_ = true;
        bool is_skip_taskbar_ = true;
        std::string title_bar_style_ = "normal";

        HWND GetMainWindow();
        void ScreenManager::Destroy();
        void ScreenManager::SetAsFrameless();
        void ScreenManager::WaitUntilReadyToShow();
        bool ScreenManager::IsMaximized();
        void ScreenManager::Maximize(const flutter::EncodableMap& args);
        bool ScreenManager::IsFullScreen();
        void ScreenManager::SetFullScreen(const flutter::EncodableMap& args);
        flutter::EncodableMap ScreenManager::GetBounds(
                const flutter::EncodableMap& args);
        void ScreenManager::SetBounds(const flutter::EncodableMap& args);
        bool ScreenManager::IsResizable();
        void ScreenManager::SetResizable(const flutter::EncodableMap& args);
        bool ScreenManager::IsAlwaysOnTop();
        void ScreenManager::SetAlwaysOnTop(const flutter::EncodableMap& args);
        void ScreenManager::SetTitleBarStyle(const flutter::EncodableMap& args);
        bool ScreenManager::IsSkipTaskbar();
        void ScreenManager::SetSkipTaskbar(const flutter::EncodableMap& args);
        bool ScreenManager::IsClosable();
        void ScreenManager::SetClosable(const flutter::EncodableMap& args);
        flutter::EncodableValue ScreenManager::GetAttachedScreenList();

    private:
        bool g_is_window_fullscreen = false;
        std::string g_title_bar_style_before_fullscreen;
        bool g_is_frameless_before_fullscreen;
        RECT g_frame_before_fullscreen;
        bool g_maximized_before_fullscreen;
        LONG g_style_before_fullscreen;
        LONG g_ex_style_before_fullscreen;
        ITaskbarList3* taskbar_ = nullptr;

    };

    ScreenManager::ScreenManager() {}

    ScreenManager::~ScreenManager() {}

    HWND ScreenManager::GetMainWindow() {
        return native_window;
    }

    void ScreenManager::Destroy() {
        PostQuitMessage(0);
    }

    void ScreenManager::SetAsFrameless() {
        is_frameless_ = true;
        HWND hWnd = GetMainWindow();

        RECT rect;

        GetWindowRect(hWnd, &rect);
        SetWindowPos(hWnd, nullptr, rect.left, rect.top, rect.right - rect.left,
                     rect.bottom - rect.top,
                     SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOSIZE |
                     SWP_FRAMECHANGED);
    }

    void ScreenManager::WaitUntilReadyToShow() {
        ::CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_INPROC_SERVER,
                           IID_PPV_ARGS(&taskbar_));
    }

    void ScreenManager::Maximize(const flutter::EncodableMap& args) {
        bool vertically =
                std::get<bool>(args.at(flutter::EncodableValue("vertically")));

        HWND hwnd = GetMainWindow();
        WINDOWPLACEMENT windowPlacement;
        GetWindowPlacement(hwnd, &windowPlacement);

        if (vertically) {
            POINT cursorPos;
            GetCursorPos(&cursorPos);
            PostMessage(hwnd, WM_NCLBUTTONDBLCLK, HTTOP,
                        MAKELPARAM(cursorPos.x, cursorPos.y));
        } else {
            if (windowPlacement.showCmd != SW_MAXIMIZE) {
                PostMessage(hwnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
            }
        }
    }

    bool ScreenManager::IsFullScreen() {
        return g_is_window_fullscreen;
    }

    void ScreenManager::SetFullScreen(const flutter::EncodableMap& args) {
        bool isFullScreen =
                std::get<bool>(args.at(flutter::EncodableValue("isFullScreen")));

        HWND mainWindow = GetMainWindow();

        // Inspired by how Chromium does this
        // https://src.chromium.org/viewvc/chrome/trunk/src/ui/views/win/fullscreen_handler.cc?revision=247204&view=markup

        // Save current window state if not already fullscreen.
        if (!g_is_window_fullscreen) {
            // Save current window information.
            g_maximized_before_fullscreen = !!::IsZoomed(mainWindow);
            g_style_before_fullscreen = GetWindowLong(mainWindow, GWL_STYLE);
            g_ex_style_before_fullscreen = GetWindowLong(mainWindow, GWL_EXSTYLE);
            if (g_maximized_before_fullscreen) {
                SendMessage(mainWindow, WM_SYSCOMMAND, SC_RESTORE, 0);
            }
            ::GetWindowRect(mainWindow, &g_frame_before_fullscreen);
            g_title_bar_style_before_fullscreen = title_bar_style_;
            g_is_frameless_before_fullscreen = is_frameless_;
        }

        if (isFullScreen) {
            flutter::EncodableMap args2 = flutter::EncodableMap();
            args2[flutter::EncodableValue("titleBarStyle")] =
                    flutter::EncodableValue("normal");
            SetTitleBarStyle(args2);

            // Set new window style and size.
            ::SetWindowLong(mainWindow, GWL_STYLE,
                            g_style_before_fullscreen & ~(WS_CAPTION | WS_THICKFRAME));
            ::SetWindowLong(mainWindow, GWL_EXSTYLE,
                            g_ex_style_before_fullscreen &
                            ~(WS_EX_DLGMODALFRAME | WS_EX_WINDOWEDGE |
                              WS_EX_CLIENTEDGE | WS_EX_STATICEDGE));

            MONITORINFO monitor_info;
            monitor_info.cbSize = sizeof(monitor_info);
            ::GetMonitorInfo(::MonitorFromWindow(mainWindow, MONITOR_DEFAULTTONEAREST),
                             &monitor_info);
            ::SetWindowPos(mainWindow, NULL, monitor_info.rcMonitor.left,
                           monitor_info.rcMonitor.top,
                           monitor_info.rcMonitor.right - monitor_info.rcMonitor.left,
                           monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top,
                           SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
            ::SendMessage(mainWindow, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
        } else {
            ::SetWindowLong(mainWindow, GWL_STYLE, g_style_before_fullscreen);
            ::SetWindowLong(mainWindow, GWL_EXSTYLE, g_ex_style_before_fullscreen);

            SendMessage(mainWindow, WM_SYSCOMMAND, SC_RESTORE, 0);

            if (title_bar_style_ != g_title_bar_style_before_fullscreen) {
                flutter::EncodableMap args2 = flutter::EncodableMap();
                args2[flutter::EncodableValue("titleBarStyle")] =
                        flutter::EncodableValue(g_title_bar_style_before_fullscreen);
                SetTitleBarStyle(args2);
            }

            if (g_is_frameless_before_fullscreen)
                SetAsFrameless();

            if (g_maximized_before_fullscreen) {
                flutter::EncodableMap args2 = flutter::EncodableMap();
                args2[flutter::EncodableValue("vertically")] =
                        flutter::EncodableValue(false);
                Maximize(args2);
            } else {
                ::SetWindowPos(
                        mainWindow, NULL, g_frame_before_fullscreen.left,
                        g_frame_before_fullscreen.top,
                        g_frame_before_fullscreen.right - g_frame_before_fullscreen.left,
                        g_frame_before_fullscreen.bottom - g_frame_before_fullscreen.top,
                        SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
            }
        }

        g_is_window_fullscreen = isFullScreen;
    }

    flutter::EncodableMap ScreenManager::GetBounds(
            const flutter::EncodableMap& args) {
        HWND hwnd = GetMainWindow();
        double devicePixelRatio =
                std::get<double>(args.at(flutter::EncodableValue("devicePixelRatio")));

        flutter::EncodableMap resultMap = flutter::EncodableMap();
        RECT rect;
        if (GetWindowRect(hwnd, &rect)) {
            double x = rect.left / devicePixelRatio * 1.0f;
            double y = rect.top / devicePixelRatio * 1.0f;
            double width = (rect.right - rect.left) / devicePixelRatio * 1.0f;
            double height = (rect.bottom - rect.top) / devicePixelRatio * 1.0f;

            resultMap[flutter::EncodableValue("x")] = flutter::EncodableValue(x);
            resultMap[flutter::EncodableValue("y")] = flutter::EncodableValue(y);
            resultMap[flutter::EncodableValue("width")] =
                    flutter::EncodableValue(width);
            resultMap[flutter::EncodableValue("height")] =
                    flutter::EncodableValue(height);
        }
        return resultMap;
    }

    void ScreenManager::SetBounds(const flutter::EncodableMap& args) {
        HWND hwnd = GetMainWindow();

        double devicePixelRatio =
                std::get<double>(args.at(flutter::EncodableValue("devicePixelRatio")));

        auto* null_or_x = std::get_if<double>(ValueOrNull(args, "x"));
        auto* null_or_y = std::get_if<double>(ValueOrNull(args, "y"));
        auto* null_or_width = std::get_if<double>(ValueOrNull(args, "width"));
        auto* null_or_height = std::get_if<double>(ValueOrNull(args, "height"));

        int x = 0;
        int y = 0;
        int width = 0;
        int height = 0;
        UINT uFlags = NULL;

        if (null_or_x != nullptr && null_or_y != nullptr) {
            x = static_cast<int>(*null_or_x * devicePixelRatio);
            y = static_cast<int>(*null_or_y * devicePixelRatio);
        }
        if (null_or_width != nullptr && null_or_height != nullptr) {
            width = static_cast<int>(*null_or_width * devicePixelRatio);
            height = static_cast<int>(*null_or_height * devicePixelRatio);
        }

        if (null_or_x == nullptr || null_or_y == nullptr) {
            uFlags = SWP_NOMOVE;
        }
        if (null_or_width == nullptr || null_or_height == nullptr) {
            uFlags = SWP_NOSIZE;
        }

        SetWindowPos(hwnd, HWND_TOP, x, y, width, height, uFlags);
    }

    bool ScreenManager::IsResizable() {
        return is_resizable_;
    }

    void ScreenManager::SetResizable(const flutter::EncodableMap& args) {
        HWND hWnd = GetMainWindow();
        is_resizable_ =
                std::get<bool>(args.at(flutter::EncodableValue("isResizable")));
        DWORD gwlStyle = GetWindowLong(hWnd, GWL_STYLE);
        if (is_resizable_) {
            gwlStyle |= WS_THICKFRAME;
        } else {
            gwlStyle &= ~WS_THICKFRAME;
        }
        ::SetWindowLong(hWnd, GWL_STYLE, gwlStyle);
    }

    bool ScreenManager::IsAlwaysOnTop() {
        DWORD dwExStyle = GetWindowLong(GetMainWindow(), GWL_EXSTYLE);
        return (dwExStyle & WS_EX_TOPMOST) != 0;
    }

    void ScreenManager::SetAlwaysOnTop(const flutter::EncodableMap& args) {
        bool isAlwaysOnTop =
                std::get<bool>(args.at(flutter::EncodableValue("isAlwaysOnTop")));
        SetWindowPos(GetMainWindow(), isAlwaysOnTop ? HWND_TOPMOST : HWND_NOTOPMOST,
                     0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    }

    void ScreenManager::SetTitleBarStyle(const flutter::EncodableMap& args) {
        title_bar_style_ =
                std::get<std::string>(args.at(flutter::EncodableValue("titleBarStyle")));
        // Enables the ability to go from setAsFrameless() to
        // TitleBarStyle.normal/hidden
        is_frameless_ = false;

        MARGINS margins = {0, 0, 0, 0};
        HWND hWnd = GetMainWindow();
        RECT rect;
        GetWindowRect(hWnd, &rect);
        DwmExtendFrameIntoClientArea(hWnd, &margins);
        SetWindowPos(hWnd, nullptr, rect.left, rect.top, 0, 0,
                     SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOSIZE |
                     SWP_FRAMECHANGED);
    }

    bool ScreenManager::IsSkipTaskbar() {
        return is_skip_taskbar_;
    }

    void ScreenManager::SetSkipTaskbar(const flutter::EncodableMap& args) {
        is_skip_taskbar_ =
                std::get<bool>(args.at(flutter::EncodableValue("isSkipTaskbar")));

        HWND hWnd = GetMainWindow();

        LPVOID lp = NULL;
        CoInitialize(lp);

        taskbar_->HrInit();
        if (!is_skip_taskbar_)
            taskbar_->AddTab(hWnd);
        else
            taskbar_->DeleteTab(hWnd);
    }

    bool ScreenManager::IsClosable() {
        HWND hWnd = GetMainWindow();
        DWORD gclStyle = GetClassLong(hWnd, GCL_STYLE);
        return !((gclStyle & CS_NOCLOSE) != 0);
    }

    void ScreenManager::SetClosable(const flutter::EncodableMap& args) {
        HWND hWnd = GetMainWindow();
        bool isClosable =
                std::get<bool>(args.at(flutter::EncodableValue("isClosable")));
        DWORD gclStyle = GetClassLong(hWnd, GCL_STYLE);
        gclStyle = isClosable ? gclStyle & ~CS_NOCLOSE : gclStyle | CS_NOCLOSE;
        SetClassLong(hWnd, GCL_STYLE, gclStyle);
    }

    // Returns the serializable form of |frame| expected by the platform channel.
    flutter::EncodableValue GetPlatformChannelRepresentationForRect(const RECT &rect) {
        return flutter::EncodableValue(flutter::EncodableList{
                flutter::EncodableValue(static_cast<double>(rect.left)),
                flutter::EncodableValue(static_cast<double>(rect.top)),
                flutter::EncodableValue(static_cast<double>(rect.right) -
                                        static_cast<double>(rect.left)),
                flutter::EncodableValue(static_cast<double>(rect.bottom) -
                                        static_cast<double>(rect.top)),
        });
    }

    // Extracts information from monitor |monitor| and returns the
    // serializable form expected by the platform channel.
    flutter::EncodableValue GetPlatformChannelRepresentationForMonitor(HMONITOR monitor) {
        if (!monitor) {
            return flutter::EncodableValue();
        }

        MONITORINFO info;
        info.cbSize = sizeof(MONITORINFO);
        ::GetMonitorInfo(monitor, &info);
        UINT dpi = FlutterDesktopGetDpiForMonitor(monitor);
        double scale_factor = dpi / kBaseDpi;
        return flutter::EncodableValue(flutter::EncodableMap{
                {flutter::EncodableValue(kFrameKey),
                                                           GetPlatformChannelRepresentationForRect(
                                                                   info.rcMonitor)},
                {flutter::EncodableValue(kVisibleFrameKey),
                                                           GetPlatformChannelRepresentationForRect(
                                                                   info.rcWork)},
                {flutter::EncodableValue(kScaleFactorKey), flutter::EncodableValue(scale_factor)},
        });
    }

    BOOL CALLBACK MonitorRepresentationEnumProc(HMONITOR monitor, HDC hdc,
            LPRECT clip, LPARAM list_ref) {
        flutter::EncodableValue *monitors = reinterpret_cast<flutter::EncodableValue *>(list_ref);
        std::get<flutter::EncodableList>(*monitors).push_back(
        GetPlatformChannelRepresentationForMonitor(monitor));
        return TRUE;
    }

    flutter::EncodableValue ScreenManager::GetAttachedScreenList() {
        flutter::EncodableValue screens(std::in_place_type < flutter::EncodableList > );
        ::EnumDisplayMonitors(nullptr, nullptr, MonitorRepresentationEnumProc,
                              reinterpret_cast<LPARAM>(&screens));
        return screens;
    }

}  // namespace