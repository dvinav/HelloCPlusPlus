# HelloCPlusPlus

A simple test project to start learning C++ programming on Windows.

## Project Overview

**HelloCPlusPlus** is a minimal Win32 application that creates an empty window using the Win32 API and DirectX 12 initialization for rendering. This project uses [xmake](https://xmake.io/) as the build system.

## Prerequisites

- Windows 10 or later
- Visual Studio 2019 or newer (with C++ Desktop Development workload)
- [xmake](https://xmake.io/) (installed and available on `%PATH%`)
- (Optional) Git command-line tools

## Repository Structure

```
HelloCPlusPlus/
├── include/            # Project header files
│   └── resource.h      # Resource definitions (e.g., icon ID)
├── src/                # Source code
│   └── main.cpp        # WinMain, DX12 setup, and render loop
├── icon.ico            # Window icon file
├── version.rc          # Resource script for metadata and icon
├── xmake.lua           # xmake build configuration
└── Readme.md           # Project documentation
```

## Build Instructions

1. **Clone the repository**

   ```bash
   git clone https://github.com/dvinav/HelloCPlusPlus.git
   cd HelloCPlusPlus
   ```

2. **Configure the build**

   - For **Debug** mode (default output: `build/windows/x64/debug/`):

     ```bash
     xmake f -m debug
     ```

   - For **Release** mode (default output: `build/windows/x64/release/`):

     ```bash
     xmake f -m release
     ```

3. **Build the project**

   ```bash
   xmake
   ```

4. **Run the executable**

   - Debug build:

     ```bash
     build/windows/x64/debug/HelloCPlusPlus.exe
     ```

   - Release build:

     ```bash
     build/windows/x64/release/HelloCPlusPlus.exe
     ```

## Usage

When you run the application, a window pops up that clears the background to a solid color each frame. You can extend `RenderFrame()` in `main.cpp` to draw shapes, text, or load assets.

## Customization

- **Window icon**: Replace `icon.ico` and update `resource.h` & `version.rc` as needed.
- **Window title**: Modify the title string in `CreateWindowEx` or update dynamically with `SetWindowText`.
- **Rendering loop**: Tweak the clear color or integrate shaders, geometry, and synchronization for advanced experiments.

## Troubleshooting

- If you encounter missing Windows API symbols, ensure `#include <windows.h>` is at the top of your source files.
- Verify that xmake is installed (`xmake --version`).
- Confirm Visual Studio tools are on your `%PATH%` or open the Build Tools command prompt.

---

Feel free to fork, experiment, and extend this project as you learn C++ on Windows!
