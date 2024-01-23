#ifndef APP_WINDOW_WIN32_H
#define APP_WINDOW_WIN32_H

#include <core/event/event.hpp>
#include <core/std/basic_types.hpp>
#include <core/std/string.hpp>
#include <map>
#include <windows.h>
#include "types.hpp"

#define WM_COPYGLOBALDATA 0x0049

namespace window
{
    namespace _internal
    {
        bool initPlatform();

        LRESULT CALLBACK wndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

        extern struct Win32PlatformData
        {
            HINSTANCE instance;
            WNDCLASSEXW win32class;
            UINT dpi;
            int frameX;
            int frameY;
            int padding;
            int screenWidth;
            int screenHeight;
            std::map<i16, io::Key> keymap{{VK_SPACE, io::Key::kSpace},
                                          {VK_OEM_7, io::Key::kApostroph},
                                          {VK_OEM_COMMA, io::Key::kComma},
                                          {VK_OEM_MINUS, io::Key::kMinus},
                                          {VK_OEM_PERIOD, io::Key::kPeriod},
                                          {VK_OEM_2, io::Key::kSlash},
                                          {0x30, io::Key::k0},
                                          {0x31, io::Key::k1},
                                          {0x32, io::Key::k2},
                                          {0x33, io::Key::k3},
                                          {0x34, io::Key::k4},
                                          {0x35, io::Key::k5},
                                          {0x36, io::Key::k6},
                                          {0x37, io::Key::k7},
                                          {0x38, io::Key::k8},
                                          {0x39, io::Key::k9},
                                          {VK_OEM_1, io::Key::kSemicolon},
                                          {VK_OEM_PLUS, io::Key::kEqual},
                                          {0x41, io::Key::kA},
                                          {0x42, io::Key::kB},
                                          {0x43, io::Key::kC},
                                          {0x44, io::Key::kD},
                                          {0x45, io::Key::kE},
                                          {0x46, io::Key::kF},
                                          {0x47, io::Key::kG},
                                          {0x48, io::Key::kH},
                                          {0x49, io::Key::kI},
                                          {0x4A, io::Key::kJ},
                                          {0x4B, io::Key::kK},
                                          {0x4C, io::Key::kL},
                                          {0x4D, io::Key::kM},
                                          {0x4E, io::Key::kN},
                                          {0x4F, io::Key::kO},
                                          {0x50, io::Key::kP},
                                          {0x51, io::Key::kQ},
                                          {0x52, io::Key::kR},
                                          {0x53, io::Key::kS},
                                          {0x54, io::Key::kT},
                                          {0x55, io::Key::kU},
                                          {0x56, io::Key::kV},
                                          {0x57, io::Key::kW},
                                          {0x58, io::Key::kX},
                                          {0x59, io::Key::kY},
                                          {0x5A, io::Key::kZ},
                                          {VK_OEM_4, io::Key::kLeftBrace},
                                          {VK_OEM_5, io::Key::kBackslash},
                                          {VK_OEM_6, io::Key::kRightBrace},
                                          {VK_OEM_3, io::Key::kGraveAccent},
                                          {VK_ESCAPE, io::Key::kEscape},
                                          {VK_RETURN, io::Key::kEnter},
                                          {VK_TAB, io::Key::kTab},
                                          {VK_BACK, io::Key::kBackspace},
                                          {VK_INSERT, io::Key::kInsert},
                                          {VK_DELETE, io::Key::kDelete},
                                          {VK_RIGHT, io::Key::kRight},
                                          {VK_LEFT, io::Key::kLeft},
                                          {VK_DOWN, io::Key::kDown},
                                          {VK_UP, io::Key::kUp},
                                          {VK_PRIOR, io::Key::kPageUp},
                                          {VK_NEXT, io::Key::kPageDown},
                                          {VK_HOME, io::Key::kHome},
                                          {VK_END, io::Key::kEnd},
                                          {VK_CAPITAL, io::Key::kCapsLock},
                                          {VK_SCROLL, io::Key::kScrollLock},
                                          {VK_NUMLOCK, io::Key::kNumLock},
                                          {VK_SNAPSHOT, io::Key::kPrintScreen},
                                          {VK_PAUSE, io::Key::kPause},
                                          {VK_F1, io::Key::kF1},
                                          {VK_F2, io::Key::kF2},
                                          {VK_F3, io::Key::kF3},
                                          {VK_F4, io::Key::kF4},
                                          {VK_F5, io::Key::kF5},
                                          {VK_F6, io::Key::kF6},
                                          {VK_F7, io::Key::kF7},
                                          {VK_F8, io::Key::kF8},
                                          {VK_F9, io::Key::kF9},
                                          {VK_F10, io::Key::kF10},
                                          {VK_F11, io::Key::kF11},
                                          {VK_F12, io::Key::kF12},
                                          {VK_F13, io::Key::kF13},
                                          {VK_F14, io::Key::kF14},
                                          {VK_F15, io::Key::kF15},
                                          {VK_F16, io::Key::kF16},
                                          {VK_F17, io::Key::kF17},
                                          {VK_F18, io::Key::kF18},
                                          {VK_F19, io::Key::kF19},
                                          {VK_F20, io::Key::kF20},
                                          {VK_F21, io::Key::kF21},
                                          {VK_F22, io::Key::kF22},
                                          {VK_F23, io::Key::kF23},
                                          {VK_F24, io::Key::kF24},
                                          {VK_NUMPAD0, io::Key::kKP0},
                                          {VK_NUMPAD1, io::Key::kKP1},
                                          {VK_NUMPAD2, io::Key::kKP2},
                                          {VK_NUMPAD3, io::Key::kKP3},
                                          {VK_NUMPAD4, io::Key::kKP4},
                                          {VK_NUMPAD5, io::Key::kKP5},
                                          {VK_NUMPAD6, io::Key::kKP6},
                                          {VK_NUMPAD7, io::Key::kKP7},
                                          {VK_NUMPAD8, io::Key::kKP8},
                                          {VK_NUMPAD9, io::Key::kKP9},
                                          {VK_DECIMAL, io::Key::kKPDecimal},
                                          {VK_DIVIDE, io::Key::kKPDivide},
                                          {VK_MULTIPLY, io::Key::kKPMultiply},
                                          {VK_SUBTRACT, io::Key::kKPSubtract},
                                          {VK_ADD, io::Key::kKPAdd},
                                          {VK_RETURN, io::Key::kKPEnter},
                                          {VK_OEM_PLUS, io::Key::kKPEqual},
                                          {VK_LSHIFT, io::Key::kLeftShift},
                                          {VK_LCONTROL, io::Key::kLeftControl},
                                          {VK_LMENU, io::Key::kLeftAlt},
                                          {VK_LWIN, io::Key::kLeftSuper},
                                          {VK_RSHIFT, io::Key::kRightShift},
                                          {VK_RCONTROL, io::Key::kRightControl},
                                          {VK_RMENU, io::Key::kRightAlt},
                                          {VK_RWIN, io::Key::kRightSuper},
                                          {VK_APPS, io::Key::kMenu}};
        } pd;

        static DWORD getWindowStyle(CreationFlags flags);

        void destroyPlatform();
    } // namespace _internal
    MonitorInfo getPrimaryMonitorInfo();

    class Win32Window : public WindowBase
    {
    public:
        Win32Window(const std::string &title, i32 width = -1, i32 height = -1,
                    CreationFlags flags = WINDOW_DEFAULT_FLAGS);

        ~Win32Window();

        // Get the window title
        virtual std::string title() const override { return convertUTF16toUTF8(_title); }

        // Set the window title
        virtual void title(const std::string &title) override;

        // Enable fullscreen mode.
        virtual void enableFullscreen() override;

        // Disable fullscreen mode.
        virtual void disableFullscreen() override;

        // Get the current cursor position.
        virtual Point2D cursorPosition() const override;

        // Set the cursor position
        virtual void cursorPosition(Point2D position) override;

        // Show the cursor.
        virtual void showCursor() override;

        // Hide the cursor.
        virtual void hideCursor() override;

        // Show the window if it is hidden.
        virtual void showWindow() override;

        // Hide the window
        virtual void hideWindow() override;

        // Get current window position
        virtual Point2D windowPos() const override;

        // Set window position
        virtual void windowPos(Point2D position) override;

        // Center the window to the parent
        virtual void centerWindowPos() override;

        // Get the WIN32 HWND handle
        HWND nativeHandle() const { return _hwnd; }

        friend LRESULT CALLBACK _internal::wndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
        friend void pollEvents();

    private:
        std::u16string _title;
        DWORD _style;
        DWORD _exStyle;
        HWND _hwnd;
        WCHAR _highSurrogate;
        Point2D _savedCursorPos{0, 0};
        bool _cursorTracked{false};
        bool _rawInput{false};
        LPBYTE _rawInputData{nullptr};
        UINT _rawInputSize{0};
    };

    // Processes all pending events in the event queue. This function checks the state
    // of all windows and other event sources, processes those events, and returns
    // control after all events have been processed. Typically used in an application's
    // update loop to handle events as they occur.
    void pollEvents();

    // Waits for new events to occur and processes them as soon as they appear.
    // Unlike pollEvents, this function blocks the execution of the program until
    // new events are available. Useful in situations where you want to conserve CPU
    // usage when the application is idle or when you need to wait for user input
    // without continuously polling.
    void waitEvents();

    // Waits for events with a specified timeout and processes them. If no events occur
    // within the given timeout period, the function returns. This is useful for
    // scenarios where you want to wait for events but also perform some other action
    // if no events occur within a certain time frame, such as updating the UI or
    // handling non-event-related logic.
    void waitEventsTimeout(f64 timeout);

    // Get the client area size
    Point2D getWindowSize(Win32Window &window);
} // namespace window

#endif