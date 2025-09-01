# No Screen - Kernel Mode Window Affinity Tool

This project is designed for P2C owners and developers.  

**No Screen** is a simple IOCTL-based tool that uses **Set Window Affinity from kernel mode** to give your window more or equivalent permissions compared to anti-cheats. In most cases, anti-cheats cannot capture your screen at all because of the permissions of the driver setting your Window Affinity.  

Tested on **Valorant** and **Call of Duty**.  

---

## How It Works

By modifying the `dwDisplayAffinity` of a window in kernel mode, this tool prevents user-mode screen capture and screenshot attempts. Essentially, it operates at a higher privilege level than most anti-cheats.

---

## How to Use

```cpp
#include <iostream>
#include "km_window_affinity.h"

int main() {
    HWND hwnd = FindWindowA(NULL, "Target Window");

    NTSTATUS status = protect_sprite_content_ex(hwnd, WDA_EXCLUDEFROMCAPTURE);

    if (status == 0)
        std::cout << "Success: Window hidden from screenshots\n";
    else
        std::cerr << "Error: Unable to hide window\n";

    return 0;
}
