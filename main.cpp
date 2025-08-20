#include <algorithm>
#include <iostream>
#include <map>
#include <windows.h>
#include <psapi.h>
#include <vector>
#include <string>
#include <shobjidl.h>
#include <wrl/client.h>

#include "file.h"
#include "gui.h"
#include "tabs.h"


template <typename T>
T* safeGet(std::vector<T>& vec, size_t idx)
{
    return idx < vec.size() ? &vec[idx] : nullptr;
}

void handle_sht1(std::vector<WindowInfo>* windows)
{
    if (windows->empty())
    {
        std::cout << "No windows found " << windows->size() << std::endl;
        return;
    }

    if (const auto win = safeGet(*windows, 0))
    {
        BringWindowToFront(win->hwnd);
    }
}

void handle_sht2(std::vector<WindowInfo>* windows)
{
    if (windows->empty())
    {
        std::cout << "No windows found " << windows->size() << std::endl;
        return;
    }

    if (const auto win = safeGet(*windows, 2 - 1))
    {
        BringWindowToFront(win->hwnd);
    }
}

void handle_sht3(std::vector<WindowInfo>* windows)
{
    if (windows->empty())
    {
        std::cout << "No windows found " << windows->size() << std::endl;
        return;
    }

    if (const auto win = safeGet(*windows, 3 - 1))
    {
        BringWindowToFront(win->hwnd);
    }
}

void handle_sht4(std::vector<WindowInfo>* windows)
{
    if (windows->empty())
    {
        std::cout << "No windows found " << windows->size() << std::endl;
        return;
    }

    if (const auto win = safeGet(*windows, 4 - 1))
    {
        BringWindowToFront(win->hwnd);
    }
}


struct ShortcutConfig
{
    int KeyModifiers;
    int TriggerKey;
    void (*callback)(std::vector<WindowInfo>* desktops);
};

const std::string FIND_MY_WIN_CONFIG = "findmywindows.txt";

std::vector<WindowInfo> availableWindows;

std::string transform(WindowInfo win)
{
    return win.processName;
}

constexpr auto trigger = MOD_CONTROL;

const std::map<UINT, ShortcutConfig> shortcuts = {
    {
        69, ShortcutConfig{
            MOD_WIN | MOD_SHIFT,
            VK_TAB,
            [](std::vector<WindowInfo>* desktops)
            {
                availableWindows = launch_gui(*desktops);
                std::vector<std::string> process_id_list;

                std::ranges::transform(
                    availableWindows,
                    std::back_inserter(process_id_list), transform
                );

                write_strings_to_file(FIND_MY_WIN_CONFIG, process_id_list);
            }
        },
    },
    {
        1, ShortcutConfig{
            trigger,
            '1',
            handle_sht1
        },
    },
    {
        2, ShortcutConfig{
            trigger,
            '2',
            handle_sht2
        },
    },
    {
        3, ShortcutConfig{
            trigger,
            '3',
            handle_sht3
        },
    },
    {
        4, ShortcutConfig{
            trigger,
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

void load_window_list();

void MessageLoop()
{
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        load_window_list();
        if (msg.message == WM_HOTKEY)
        {
            auto item = shortcuts.find(msg.wParam);
            if (item != shortcuts.end())
            {
                item->second.callback(&availableWindows);
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


void load_window_list()
{
    std::vector<WindowInfo> orderedWindows;

    std::vector<WindowInfo> initialWindows = ListWindowsByDesktop(true);
    const std::vector<std::string> saved_process = read_strings_from_file(FIND_MY_WIN_CONFIG);

    std::vector<WindowInfo> finalWindowList;
    finalWindowList.reserve(initialWindows.size());

    // Iterate through the desired process order.
    for (const auto& process_name : saved_process)
    {
        // Find the window corresponding to the saved process name.
        auto it = std::ranges::find_if(
            initialWindows,
            [&](const WindowInfo& window)
            {
                return window.processName == process_name;
            });

        // If the window was found in the available list...
        if (it != initialWindows.end())
        {
            // move it to our final list and erase it from the original
            // to mark it as "processed".
            finalWindowList.push_back(std::move(*it));
            initialWindows.erase(it);
        }
    }

    finalWindowList.insert(
        finalWindowList.end(),
        std::make_move_iterator(initialWindows.begin()),
        std::make_move_iterator(initialWindows.end())
    );

    availableWindows = finalWindowList;
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
