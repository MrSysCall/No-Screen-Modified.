# 🛡️ Kernel Mode Window Affinity Controller

## 🔥 Features

- **Kernel-Level Authority** – Operates at Ring-0, accessing `win32kbase.sys` & internal `tagWND` objects.
- **Ultimate Stealth** – Completely invisible to user-mode API hooks & `GetWindowDisplayAffinity`.
- **Zero Process Impact** – Target windows are unaware; no injection, no memory modification.
- **Bypass Hooks** – Immune to Detours-style monitoring or API interceptions.

---

## 🚀 Why Kernel Mode?

| Aspect | User-Mode API | Kernel Driver |
|--------|---------------|---------------|
| Privilege | Ring-3 | Ring-0 |
| Detection | ❌ Easily intercepted | ✅ Undetectable in user-mode |
| Access | Public API | Internal tagWND structures |
| Hook Bypass | ❌ | ✅ |
| Overhead | API Call | Direct memory write |

---

## 🛠 Technical Overview

The driver:

1. Finds the target window's `tagWND` structure in kernel memory.
2. Modifies `dwDisplayAffinity` to the desired value (`WDA_MONITOR`, `WDA_EXCLUDEFROMCAPTURE`, etc.).
3. Applies the change directly via the Window Manager—**no trace in user-mode**.

**Challenge solved:** reliable cross-version detection of `tagWND`.

---

## 💻 Example

```cpp
#include <iostream>
#include "km_window_affinity.h"

HWND hwnd = FindWindowA(NULL, "Target Window");

NTSTATUS status = set_attributes(hwnd, WDA_EXCLUDEFROMCAPTURE);

if (status == 0)
    std::cout << "✅ Success: Window hidden from screenshots\n";
else
    std::cerr << "❌ Error: Unable to hide window\n";
