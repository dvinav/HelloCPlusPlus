#include "resource.h"
#include <math.h>
#include <renderer.h>
#include <tchar.h>
#include <windows.h>

HWND hwnd;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam,
                            LPARAM lParam) {
  if (uMsg == WM_DESTROY) {
    PostQuitMessage(0);
    return 0;
  }
  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
  const LPCSTR CLASS_NAME = "MainWindow";

  WNDCLASS wc = {};
  wc.lpfnWndProc = WindowProc;
  wc.hInstance = hInstance;
  wc.lpszClassName = CLASS_NAME;
  wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
  RegisterClass(&wc);

  hwnd = CreateWindowEx(0, CLASS_NAME, "Hello C++!", WS_OVERLAPPEDWINDOW,
                        CW_USEDEFAULT, CW_USEDEFAULT, 1280, 720, nullptr,
                        nullptr, hInstance, nullptr);

  ShowWindow(hwnd, nCmdShow);

  DXInit(&hwnd);

  // Main loop
  MSG msg = {};
  float r = 0.0f;
  while (true) {
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
      if (msg.message == WM_QUIT)
        goto cleanup;
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    RenderFrame(&hwnd);
  }

cleanup:
  if (msg.message == WM_QUIT) {
    cleanup();
  }

  return (int)msg.wParam;
}
