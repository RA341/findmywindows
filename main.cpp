#include <iostream>
#include <map>
#include <windows.h>
#include <psapi.h>
#include <vector>
#include <string>
#include <shobjidl.h>
#include <wrl/client.h>

#include "gui.h"
#include "tabs.h"


template <typename T>
T* safeGet(std::vector<T>& vec, size_t idx)
{
    return idx < vec.size() ? &vec[idx] : nullptr;
}

void handle_sht1()
{
    auto windows = ListWindowsByDesktop(true);
    if (windows.empty())
    {
        std::cout << "No windows found " << windows.size() << std::endl;
        return;
    }

    if (const auto win = safeGet(windows, 1))
    {
        BringWindowToFront(win->hwnd);
    }
}

void handle_sht2()
{
    auto windows = ListWindowsByDesktop(true);
    if (windows.empty())
    {
        std::cout << "No windows found " << windows.size() << std::endl;
        return;
    }

    if (const auto win = safeGet(windows, 2))
    {
        BringWindowToFront(win->hwnd);
    }
}

void handle_sht3()
{
    auto windows = ListWindowsByDesktop(true);
    if (windows.empty())
    {
        std::cout << "No windows found " << windows.size() << std::endl;
        return;
    }

    if (const auto win = safeGet(windows, 3))
    {
        BringWindowToFront(win->hwnd);
    }
}

void handle_sht4()
{
    auto windows = ListWindowsByDesktop(true);
    if (windows.empty())
    {
        std::cout << "No windows found " << windows.size() << std::endl;
        return;
    }

    if (const auto win = safeGet(windows, 4))
    {
        BringWindowToFront(win->hwnd);
    }
}


struct ShortcutConfig
{
    int KeyModifiers;
    int TriggerKey;
    void (*callback)();
};


const std::map<UINT, ShortcutConfig> shortcuts = {
    {
        69, {
            MOD_WIN | MOD_SHIFT,
            VK_TAB,
            launch_gui
        },
    },
    {
        1, {
            MOD_CONTROL,
            '1',
            handle_sht1
        },
    },
    {
        2, {
            MOD_CONTROL,
            '2',
            handle_sht2
        },
    },
    {
        3, {
            MOD_CONTROL,
            '3',
            handle_sht3
        },
    },
    {
        4, {
            MOD_CONTROL,
            '4',
            handle_sht4
        },
    },
};

bool RegisterGlobalHotkey()
{
    for (auto [hotKeyID, config] : shortcuts)
    {
        // MOD_ALT = Alt modifier, VK_1 = '1' key
        // Register Alt+1 hotkey
        if (RegisterHotKey(nullptr, hotKeyID, config.KeyModifiers, config.TriggerKey))
        {
            std::cout << "Registering shortcuts " << config.TriggerKey << std::endl;
        }
        else
        {
            std::cout << "Failed to register hotkey. Error code: " << GetLastError() << std::endl;
            return false;
        }
    }

    return true;
}

void UnregisterGlobalHotkey()
{
    for (auto [hotKeyID, config] : shortcuts)
    {
        UnregisterHotKey(nullptr, hotKeyID);
        std::cout << "un registering shortcuts " << config.TriggerKey << std::endl;
    }
}

void MessageLoop()
{
    std::cout << "Listening for CTRL+1" << std::endl;

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (msg.message == WM_HOTKEY)
        {
            auto item = shortcuts.find(msg.wParam);
            if (item != shortcuts.end())
            {
                item->second.callback();
            }
            else
            {
                std::cout << "shortcut not found" << msg.wParam << std::endl;
            }
        }

        //TranslateMessage(&msg)
        // Converts raw keyboard input (like WM_KEYDOWN) into actual character messages (like WM_CHAR)
        // For hotkeys specifically: This isn't strictly necessary since
        // WM_HOTKEY messages don't need translation, but it's included for completeness
        TranslateMessage(&msg);
        // Sends the message to the appropriate window procedure for processing
        DispatchMessage(&msg);
    }
}


int main()
{
    std::cout << "FindMyTabs\n";
    std::cout << "==================================\n\n";

    if (RegisterGlobalHotkey())
    {
        MessageLoop();
        UnregisterGlobalHotkey();
    }

    return 0;
}
