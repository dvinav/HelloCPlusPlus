#pragma once
#include <d3d12.h>
#include <d3dcompiler.h>
#include <dxgi1_6.h>
#include <windows.h>

struct DXWindowOptions {
  int nWidth = 1280;
  int nHeight = 720;
  LPCSTR windowName = "Default Window";
  LPCSTR className = "DefaultClass";
};

class DXWindow {
private:
  HWND hwnd;
  ID3D12Device *device;
  IDXGISwapChain3 *swapChain;
  ID3D12CommandQueue *commandQueue;
  ID3D12DescriptorHeap *rtvHeap;
  ID3D12Resource *renderTargets[2];
  UINT rtvDescriptorSize;
  ID3D12CommandAllocator *commandAllocator;
  ID3D12GraphicsCommandList *commandList;
  IDXGIFactory6 *dxgiFactory = nullptr;
  HINSTANCE hInstance;
  int nCmdShow;
  short fps;

  LARGE_INTEGER frequency = {};
  LARGE_INTEGER lastTime = {};
  bool initialized = false;

  void DXInit();
  void InitWindow(const DXWindowOptions &opts);

public:
  DXWindow(HINSTANCE hInstance, int nCmdShow, const DXWindowOptions &opts);

  void RenderFrame();
  void Cleanup();
};