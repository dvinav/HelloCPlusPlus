#include "renderer.h"
#include <cmath>
#include <d3d12.h>
#include <d3dcompiler.h>
#include <dxgi1_6.h>
#include <stdio.h>
#include <windows.h>

ID3D12Device *device;
IDXGISwapChain3 *swapChain;
ID3D12CommandQueue *commandQueue;
ID3D12DescriptorHeap *rtvHeap;
ID3D12Resource *renderTargets[2];
UINT rtvDescriptorSize;
ID3D12CommandAllocator *commandAllocator;
ID3D12GraphicsCommandList *commandList;
IDXGIFactory6 *dxgiFactory = nullptr;

LARGE_INTEGER frequency = {};
LARGE_INTEGER lastTime = {};
bool initialized = false;

void DXInit(HWND *hwnd) {
  // Initialize DXGI Factory

  CreateDXGIFactory2(0, IID_PPV_ARGS(&dxgiFactory));

  // Create device
  D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));

  // Create command queue
  D3D12_COMMAND_QUEUE_DESC queueDesc = {};
  queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
  queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
  queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
  queueDesc.NodeMask = 0;
  device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue));

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
  dxgiFactory->CreateSwapChainForHwnd(commandQueue, *hwnd, &swapChainDesc,
                                      nullptr, nullptr, &tempSwapChain);

  tempSwapChain->QueryInterface(IID_PPV_ARGS(&swapChain));
  tempSwapChain->Release();

  // Create RTV heap
  D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
  rtvHeapDesc.NumDescriptors = 2;
  rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
  rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
  device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap));

  rtvDescriptorSize =
      device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

  // Create render target views
  D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle =
      rtvHeap->GetCPUDescriptorHandleForHeapStart();
  for (UINT i = 0; i < 2; i++) {
    swapChain->GetBuffer(i, IID_PPV_ARGS(&renderTargets[i]));
    device->CreateRenderTargetView(renderTargets[i], nullptr, rtvHandle);
    rtvHandle.ptr += rtvDescriptorSize;
  }

  // Create command allocator and list
  device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                 IID_PPV_ARGS(&commandAllocator));
  device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator,
                            nullptr, IID_PPV_ARGS(&commandList));
  commandList->Close();

  QueryPerformanceFrequency(&frequency);
  QueryPerformanceCounter(&lastTime);
  initialized = true;
}

void RenderFrame(HWND *hwnd) {
  static UINT frameIndex = 0;

  frameIndex = swapChain->GetCurrentBackBufferIndex();

  // Reset allocator and command list
  commandAllocator->Reset();
  commandList->Reset(commandAllocator, nullptr);

  // Transition back buffer from present -> render target
  D3D12_RESOURCE_BARRIER barrier = {};
  barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
  barrier.Transition.pResource = renderTargets[frameIndex];
  barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
  barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
  barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

  commandList->ResourceBarrier(1, &barrier);

  // Clear render target (color)
  D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle =
      rtvHeap->GetCPUDescriptorHandleForHeapStart();
  rtvHandle.ptr += frameIndex * rtvDescriptorSize;

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
  commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

  // Transition back buffer from render target -> present
  barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
  barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
  commandList->ResourceBarrier(1, &barrier);

  commandList->Close();

  static int frameCount = 0;
  static float elapsedTime = 0.0f;

  frameCount++;
  elapsedTime += dt; // dt from QueryPerformanceCounter timing

  if (elapsedTime >= 1.0f) {
    int fps = frameCount;
    frameCount = 0;
    elapsedTime = 0.0f;

    char title[256];
    sprintf_s(title, "Hello C++ (%d FPS)", fps);
    SetWindowTextA(*hwnd, title);
  }

  // Execute
  ID3D12CommandList *ppCommandLists[] = {commandList};
  commandQueue->ExecuteCommandLists(1, ppCommandLists);

  // Present
  swapChain->Present(1, 0);
}

void cleanup() {
  if (commandList)
    commandList->Release();
  if (commandAllocator)
    commandAllocator->Release();
  for (int i = 0; i < 2; i++)
    if (renderTargets[i])
      renderTargets[i]->Release();
  if (rtvHeap)
    rtvHeap->Release();
  if (swapChain)
    swapChain->Release();
  if (commandQueue)
    commandQueue->Release();
  if (device)
    device->Release();
  if (dxgiFactory)
    dxgiFactory->Release();
}