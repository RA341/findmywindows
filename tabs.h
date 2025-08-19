#ifndef FINDMYTABS_TABS_H
#define FINDMYTABS_TABS_H

#include <windows.h>
#include <vector>
#include <string>

struct WindowInfo
{
    HWND hwnd;
    std::string title;
    std::string className;
    DWORD processId;
    bool isOnCurrentDesktop;
};

std::vector<WindowInfo> ListWindowsByDesktop(bool currentDesktopOnly);

void BringWindowToFront(HWND hwnd);

#endif //FINDMYTABS_TABS_H
