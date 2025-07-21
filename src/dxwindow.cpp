#include "dxwindow.h"
#include "resource.h"
#include <cmath>
#include <d3d12.h>
#include <d3dcompiler.h>
#include <dxgi1_6.h>
#include <windows.h>

void DXWindow::DXInit() {
  // Initialize DXGI Factory

  CreateDXGIFactory2(0, IID_PPV_ARGS(&this->dxgiFactory));

  // Create device
  D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));

  // Create command queue
  D3D12_COMMAND_QUEUE_DESC queueDesc = {};
  queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
  queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
  queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
  queueDesc.NodeMask = 0;
  device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&this->commandQueue));

  // Create swap chain
  DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
  swapChainDesc.BufferCount = 2;
  swapChainDesc.Width = 1280;
  swapChainDesc.Height = 720;
  swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
  swapChainDesc.SampleDesc.Count = 1;

  IDXGISwapChain1 *tempSwapChain;
  this->dxgiFactory->CreateSwapChainForHwnd(this->commandQueue, hwnd,
                                            &swapChainDesc, nullptr, nullptr,
                                            &tempSwapChain);

  tempSwapChain->QueryInterface(IID_PPV_ARGS(&this->swapChain));
  tempSwapChain->Release();

  // Create RTV heap
  D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
  rtvHeapDesc.NumDescriptors = 2;
  rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
  rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
  this->device->CreateDescriptorHeap(&rtvHeapDesc,
                                     IID_PPV_ARGS(&this->rtvHeap));

  this->rtvDescriptorSize = this->device->GetDescriptorHandleIncrementSize(
      D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

  // Create render target views
  D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle =
      this->rtvHeap->GetCPUDescriptorHandleForHeapStart();
  for (UINT i = 0; i < 2; i++) {
    this->swapChain->GetBuffer(i, IID_PPV_ARGS(&this->renderTargets[i]));
    this->device->CreateRenderTargetView(this->renderTargets[i], nullptr,
                                         rtvHandle);
    rtvHandle.ptr += this->rtvDescriptorSize;
  }

  // Create command allocator and list
  device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                 IID_PPV_ARGS(&this->commandAllocator));
  device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                            this->commandAllocator, nullptr,
                            IID_PPV_ARGS(&this->commandList));
  this->commandList->Close();

  QueryPerformanceFrequency(&frequency);
  QueryPerformanceCounter(&lastTime);
  initialized = true;
}

void DXWindow::RenderFrame() {
  static UINT frameIndex = 0;

  frameIndex = this->swapChain->GetCurrentBackBufferIndex();

  // Reset allocator and command list
  this->commandAllocator->Reset();
  this->commandList->Reset(this->commandAllocator, nullptr);

  // Transition back buffer from present -> render target
  D3D12_RESOURCE_BARRIER barrier = {};
  barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
  barrier.Transition.pResource = this->renderTargets[frameIndex];
  barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
  barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
  barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

  this->commandList->ResourceBarrier(1, &barrier);

  // Clear render target (color)
  D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle =
      this->rtvHeap->GetCPUDescriptorHandleForHeapStart();
  rtvHandle.ptr += frameIndex * this->rtvDescriptorSize;

  // Inside RenderFrame():
  LARGE_INTEGER currentTime;
  QueryPerformanceCounter(&currentTime);
  float dt =
      float(currentTime.QuadPart - lastTime.QuadPart) / frequency.QuadPart;
  lastTime = currentTime;

  static float time = 0.0f;
  time += dt;

  float redValue = 0.1f + (sinf(time * 3.14159f * 2.0f) + 1.0f) * 0.2f;
  FLOAT clearColor[] = {redValue, 0.2f, 0.4f, 1.0f};
  this->commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

  // Transition back buffer from render target -> present
  barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
  barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
  this->commandList->ResourceBarrier(1, &barrier);

  this->commandList->Close();

  static int frameCount = 0;
  static float elapsedTime = 0.0f;

  frameCount++;
  elapsedTime += dt; // dt from QueryPerformanceCounter timing

  if (elapsedTime >= 1.0f) {
    int fps = frameCount;
    frameCount = 0;
    elapsedTime = 0.0f;
  }

  // Execute
  ID3D12CommandList *ppCommandLists[] = {this->commandList};
  this->commandQueue->ExecuteCommandLists(1, ppCommandLists);

  // Present
  this->swapChain->Present(1, 0);
}

void DXWindow::Cleanup() {
  if (this->commandList)
    this->commandList->Release();
  if (this->commandAllocator)
    this->commandAllocator->Release();
  for (int i = 0; i < 2; i++)
    if (this->renderTargets[i])
      this->renderTargets[i]->Release();
  if (this->rtvHeap)
    this->rtvHeap->Release();
  if (this->swapChain)
    this->swapChain->Release();
  if (this->commandQueue)
    this->commandQueue->Release();
  if (device)
    device->Release();
  if (this->dxgiFactory)
    this->dxgiFactory->Release();
}

void DXWindow::InitWindow(const DXWindowOptions &opts) {
  WNDCLASS wc = {};
  wc.lpfnWndProc = DefWindowProc;
  wc.hInstance = this->hInstance;
  wc.lpszClassName = opts.className;
  wc.hIcon = LoadIcon(this->hInstance, MAKEINTRESOURCE(IDI_ICON1));
  RegisterClass(&wc);

  hwnd = CreateWindowEx(0, opts.className, opts.windowName,
                        WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT,
                        CW_USEDEFAULT, opts.nWidth, opts.nHeight, nullptr,
                        nullptr, this->hInstance, nullptr);

  ShowWindow(hwnd, nCmdShow);
}

DXWindow::DXWindow(HINSTANCE hInstance, int nCmdShow,
                   const DXWindowOptions &opts) {
  this->hInstance = hInstance;
  this->nCmdShow = nCmdShow;
  this->InitWindow(opts);
  this->DXInit();
}