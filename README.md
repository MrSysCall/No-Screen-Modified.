# Kernel Mode Window Affinity Controller

# Planning On Releasing This Week.

A high-privilege Windows kernel driver for hiding windows from screenshots and screen capture. Operates entirely in kernel mode, bypassing user-mode API restrictions and hooks.


---

## Features

- **Kernel-Level Access**: Operates at Ring-0 with direct access to `win32kbase.sys` and `tagWND` structures.  
- **Stealth**: Modifications are invisible to user-mode API calls and hooks.  
- **No Impact on Target Process**: Applies changes externally, without injecting code or modifying memory.  
- **Hook Resistant**: Immune to user-mode hooking engines such as Detours.

---

## Why Kernel Mode

| Aspect | User-Mode API | Kernel Driver |
|--------|---------------|---------------|
| Privilege | Ring-3 | Ring-0 |
| Detection | Easily intercepted | Undetectable in user-mode |
| Access | Public API | Internal `tagWND` structures |
| Bypasses Hooks | No | Yes |
| Overhead | API call | Direct memory write |

---

## How It Works

The driver locates the target window's `tagWND` structure in kernel memory and modifies the `dwDisplayAffinity` field to the requested value (`WDA_MONITOR`, `WDA_EXCLUDEFROMCAPTURE`). The change is applied directly via the Window Manager, with no observable side effects in user-mode.

---

## Example Usage

```cpp
#include <iostream>
#include "km_window_affinity.h"

int main() {
    HWND hwnd = FindWindowA(NULL, "Target Window");

    NTSTATUS status = hide_window(hwnd, WDA_EXCLUDEFROMCAPTURE);

    if (status == 0)
        std::cout << "Success: Window hidden from screenshots\n";
    else
        std::cerr << "Error: Unable to hide window\n";

    return 0;
}
