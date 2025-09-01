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
#include <iostream>
#include "km_window_affinity.h"

int main() {
    // Retrieve handle to the target window
    HWND hwnd = FindWindowA(NULL, "Target Window");

    // Apply kernel-level display affinity protection
    NTSTATUS status = protect_sprite_content_ex(hwnd, WDA_EXCLUDEFROMCAPTURE);

    if (status == 0)
        std::cout << "Window successfully hidden from screenshots.\n";
    else
        std::cerr << "Failed to apply window protection.\n";

    return 0;
}
