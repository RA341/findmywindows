#include "tabs.h"

#include <iostream>
#include <windows.h>
#include <psapi.h>
#include <vector>
#include <string>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

// Virtual Desktop Manager interface (Windows 10/11)
// These are undocumented COM interfaces that Windows uses internally
DEFINE_GUID(CLSID_VirtualDesktopManager, 0xaa509086, 0x4258, 0x4bd1, 0x94, 0xcf, 0x3f, 0xde, 0x1c, 0x5d, 0x4b, 0xce);
DEFINE_GUID(IID_IVirtualDesktopManager, 0xa5cd92ff, 0x29be, 0x454c, 0x8d, 0x04, 0xd8, 0x28, 0x79, 0xfb, 0x3f, 0x1b);

// Interface declaration
MIDL_INTERFACE("a5cd92ff-29be-454c-8d04-d82879fb3f1b")
    IVirtualDesktopManager : public IUnknown
{
public:
    virtual ~IVirtualDesktopManager() = default;
    virtual HRESULT STDMETHODCALLTYPE IsWindowOnCurrentVirtualDesktop(
        HWND topLevelWindow,
        BOOL* onCurrentDesktop) = 0;

    virtual HRESULT STDMETHODCALLTYPE GetWindowDesktopId(
        HWND topLevelWindow,
        GUID* desktopId) = 0;

    virtual HRESULT STDMETHODCALLTYPE MoveWindowToDesktop(
        HWND topLevelWindow,
        REFGUID desktopId) = 0;
};

// Function to check if a window should appear in Alt+Tab
bool IsAltTabWindow(HWND hwnd)
{
    if (!IsWindowVisible(hwnd))
    {
        return false;
    }

    DWORD exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
    if (exStyle & WS_EX_TOOLWINDOW)
    {
        return false;
    }

    HWND owner = GetWindow(hwnd, GW_OWNER);
    if (!(exStyle & WS_EX_APPWINDOW) && owner != NULL)
    {
        return false;
    }

    char title[256];
    GetWindowTextA(hwnd, title, sizeof(title));
    if (strlen(title) == 0)
    {
        if (!(exStyle & WS_EX_APPWINDOW))
        {
            return false;
        }
    }

    return true;
}

// Get process name from process ID
std::string GetProcessName(DWORD processId)
{
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
    if (hProcess)
    {
        char processName[MAX_PATH];
        if (GetModuleBaseNameA(hProcess, NULL, processName, sizeof(processName)))
        {
            CloseHandle(hProcess);
            return std::string(processName);
        }
        CloseHandle(hProcess);
    }
    return "Unknown";
}

// Global variables for enumeration
std::vector<WindowInfo>* g_windows = nullptr;
IVirtualDesktopManager* g_vdm = nullptr;

// Callback function for EnumWindows
BOOL CALLBACK EnumWindowsForDesktopProc(HWND hwnd, LPARAM lParam)
{
    if (IsAltTabWindow(hwnd))
    {
        WindowInfo info;
        info.hwnd = hwnd;

        // Get window title
        char title[256];
        GetWindowTextA(hwnd, title, sizeof(title));
        info.title = title;

        // Get class name
        char className[256];
        GetClassNameA(hwnd, className, sizeof(className));
        info.className = className;

        // Get process ID
        GetWindowThreadProcessId(hwnd, &info.processId);

        // Check if window is on current virtual desktop
        info.isOnCurrentDesktop = false;
        if (g_vdm)
        {
            BOOL onCurrentDesktop = FALSE;
            HRESULT hr = g_vdm->IsWindowOnCurrentVirtualDesktop(hwnd, &onCurrentDesktop);
            if (SUCCEEDED(hr))
            {
                info.isOnCurrentDesktop = (onCurrentDesktop == TRUE);
            }
        }

        g_windows->push_back(info);
    }

    return TRUE;
}

void print_windows(bool currentDesktopOnly, std::vector<WindowInfo> currentDesktopWindows,
                   std::vector<WindowInfo> otherDesktopWindows)
{
    if (currentDesktopOnly)
    {
        std::cout << "Windows on CURRENT Virtual Desktop (" << currentDesktopWindows.size() << " found):\n";
        std::cout << "============================================\n\n";

        for (size_t i = 0; i < currentDesktopWindows.size(); ++i)
        {
            const WindowInfo& info = currentDesktopWindows[i];
            std::string processName = GetProcessName(info.processId);

            std::cout << "[" << (i + 1) << "] " << info.title << "\n";
            std::cout << "    Handle: 0x" << std::hex << info.hwnd << std::dec << "\n";
            std::cout << "    Class:  " << info.className << "\n";
            std::cout << "    Process: " << processName << " (PID: " << info.processId << ")\n\n";
        }
    }
    else
    {
        // Show all windows with desktop indication
        std::cout << "All Windows with Desktop Status:\n";
        std::cout << "================================\n\n";

        std::cout << "CURRENT DESKTOP (" << currentDesktopWindows.size() << " windows):\n";
        std::cout << "-------------------\n";
        for (const auto& info : currentDesktopWindows)
        {
            std::string processName = GetProcessName(info.processId);
            std::cout << "• " << info.title << " (" << processName << ")\n";
        }

        std::cout << "\nOTHER DESKTOPS (" << otherDesktopWindows.size() << " windows):\n";
        std::cout << "------------------\n";
        for (const auto& info : otherDesktopWindows)
        {
            std::string processName = GetProcessName(info.processId);
            std::cout << "• " << info.title << " (" << processName << ")\n";
        }
    }
}

// Function to list windows filtered by desktop
std::vector<WindowInfo> ListWindowsByDesktop(bool currentDesktopOnly = true)
{
    // Initialize COM
    HRESULT hr = CoInitialize(nullptr);
    if (FAILED(hr))
    {
        std::cout << "Failed to initialize COM: " << std::hex << hr << std::endl;
        return {};
    }

    // Create Virtual Desktop Manager
    ComPtr<IVirtualDesktopManager> vdm;
    hr = CoCreateInstance(CLSID_VirtualDesktopManager, NULL, CLSCTX_ALL,
                          IID_IVirtualDesktopManager, &vdm);

    if (FAILED(hr))
    {
        std::cout << "Virtual Desktop Manager not available (Windows 10/11 required)" << std::endl;
        std::cout << "Listing all windows instead...\n" << std::endl;
        currentDesktopOnly = false;
    }

    std::vector<WindowInfo> windows;
    g_windows = &windows;
    g_vdm = vdm.Get();

    // Enumerate all windows
    EnumWindows(EnumWindowsForDesktopProc, 0);

    // Clear global pointer before COM cleanup
    g_vdm = nullptr;

    // Filter and display results
    std::vector<WindowInfo> currentDesktopWindows;
    std::vector<WindowInfo> otherDesktopWindows;

    for (const auto& window : windows)
    {
        if (window.isOnCurrentDesktop)
        {
            currentDesktopWindows.push_back(window);
        }
        else
        {
            otherDesktopWindows.push_back(window);
        }
    }

    print_windows(currentDesktopOnly, currentDesktopWindows, otherDesktopWindows);

    // Cleanup - let ComPtr handle the release automatically, then uninitialize COM
    vdm.Reset(); // Explicitly release the COM object
    CoUninitialize();

    return currentDesktopWindows;
}

// Function to get desktop GUID for a window
void ShowWindowDesktopInfo(HWND hwnd)
{
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr)) return;

    ComPtr<IVirtualDesktopManager> vdm;
    hr = CoCreateInstance(CLSID_VirtualDesktopManager, NULL, CLSCTX_ALL,
                          IID_IVirtualDesktopManager, &vdm);

    if (SUCCEEDED(hr))
    {
        GUID desktopId;
        hr = vdm->GetWindowDesktopId(hwnd, &desktopId);
        if (SUCCEEDED(hr))
        {
            std::cout << "Desktop ID: {" << std::hex
                << desktopId.Data1 << "-" << desktopId.Data2 << "-"
                << desktopId.Data3 << "-";
            for (int i = 0; i < 8; i++)
            {
                std::cout << static_cast<int>(desktopId.Data4[i]);
                if (i == 1) std::cout << "-";
            }
            std::cout << "}" << std::dec << std::endl;
        }
    }

    CoUninitialize();
}

// Function to bring a window to front
void BringWindowToFront(HWND hwnd)
{
    // Restore if minimized
    if (IsIconic(hwnd))
    {
        ShowWindow(hwnd, SW_RESTORE);
    }

    // Bring to foreground
    SetForegroundWindow(hwnd);
    BringWindowToTop(hwnd);

    char title[256];
    GetWindowTextA(hwnd, title, sizeof(title));
    std::cout << "Brought window to front: " << title << std::endl;
}
