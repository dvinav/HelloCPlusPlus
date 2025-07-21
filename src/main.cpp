#include "dxwindow.h"
#include <math.h>
#include <tchar.h>
#include <windows.h>

HWND hwnd;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
  DXWindowOptions opts = {.nWidth = 1280,
                          .nHeight = 720,
                          .windowName = "Hello C++",
                          .className = "MainWindow"};
  DXWindow mainWindow(hInstance, nCmdShow, opts);

  MSG msg = {};
  float r = 0.0f;
  while (true) {
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
      if (msg.message == WM_QUIT)
        mainWindow.Cleanup();
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    mainWindow.RenderFrame();
  }

  return (int)msg.wParam;
}
