# Preview -> https://streamable.com/0ewfwu

## All Windows Versions Supports - Windows 10/Windows 11 24H2


# No Screen - Kernel Mode Window Affinity Controller

**No Screen** is a high-privilege kernel-mode tool designed for developers requiring secure window protection against screen capture. By leveraging a direct kernel-level modification of window display affinity, it ensures that target windows remain inaccessible to most user-mode screen capture and anti-cheat mechanisms.  

This tool has been tested on **Valorant** and **Call of Duty** and demonstrates reliable protection against typical user-mode screenshot attempts.  

---

## Technical Overview

The driver operates by locating the target window's kernel-mode `tagWND` structure and modifying the `dwDisplayAffinity` field. This enforces the requested display affinity (`WDA_MONITOR`, `WDA_EXCLUDEFROMCAPTURE`, etc.) directly within the Window Manager.  

Key advantages of this approach:  

- **Kernel-Level Enforcement**: Operates at Ring-0, providing a privilege level higher than standard user-mode screen capture routines.  
- **Stealth**: Modifications are invisible to user-mode API monitoring, hooks, and anti-cheat inspection.  
- **No Process Interference**: Changes are applied externally; the target process remains unaware, and no memory injection is required.  

---

## Usage Example

```cpp
// NoScreen Example By Developer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "kernel.h"

int main ( )
{
    if ( !kernel->IsLoaded ( ) ) {
        std::cerr << "Could not find driver. Make sure NoScreen is loaded." << std::endl;
        Sleep ( 2500 );
        return 1;
    }
    std::cout << "Driver found successfully." << std::endl;

    HWND hwnd = FindWindowA ( NULL , "Untitled - Notepad" );
    if ( !hwnd ) {
        std::cerr << "Target window not found. Make sure Notepad is open with the correct title." << std::endl;
        Sleep ( 2500 );
        return 1;
    }
    std::cout << "Window handle found: 0x" << hwnd << std::endl;

    NTSTATUS status = kernel->ProtectWindow ( hwnd , WDA_NONE ); // WDA_NONE UNHIDES IT // WDA_EXCLUDEFROMCAPTURE HIDES IT.
    if ( status == STATUS_SUCCESS ) {
        std::cout << "Window has been hidden from capture successfully." << std::endl;
    }
    else {
        std::cerr << "Failed to hide window. NTSTATUS: 0x" << std::hex << status << std::endl;
    }

    Sleep ( 2500 );
    return 0;
}




