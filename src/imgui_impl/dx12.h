// dear imgui: Renderer Backend for DirectX12
// This needs to be used along with a Platform Backend (e.g. Win32)

// Implemented features:
//  [X] Renderer: User texture binding. Use 'D3D12_GPU_DESCRIPTOR_HANDLE' as ImTextureID. Read the FAQ about
//  ImTextureID! [X] Renderer: Support for large meshes (64k+ vertices) with 16-bit indices.

// Important: to compile on 32-bit systems, this backend requires code to be compiled with '#define ImTextureID ImU64'.
// This is because we need ImTextureID to carry a 64-bit value and by default ImTextureID is defined as void*.
// This define is set in the example .vcxproj file and need to be replicated in your app or by adding it to your
// imconfig.h file.

// You can copy and use unmodified imgui_impl_* files in your project. See examples/ folder for examples of using this.
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#pragma once
#include "imgui.h" // IMGUI_IMPL_API

struct ID3D12Device;
struct ID3D12DescriptorHeap;
struct ID3D12GraphicsCommandList;
struct D3D12_CPU_DESCRIPTOR_HANDLE;
struct D3D12_GPU_DESCRIPTOR_HANDLE;

// cmd_list is the command list that the implementation will use to render imgui draw lists.
// Before calling the render function, caller must prepare cmd_list by resetting it and setting the appropriate
// render target and descriptor heap that contains font_srv_cpu_desc_handle/font_srv_gpu_desc_handle.
// font_srv_cpu_desc_handle and font_srv_gpu_desc_handle are handles to a single SRV descriptor to use for the internal
// font texture.
IMGUI_IMPL_API bool ImGui_ImplDX12_Init(
    ID3D12Device* apDevice, int aNumFramesInFlight, DXGI_FORMAT aRTVFormat, ID3D12DescriptorHeap* apSRVHeapDesc, D3D12_CPU_DESCRIPTOR_HANDLE aFontSRVDescCPU,
    D3D12_GPU_DESCRIPTOR_HANDLE aFontSRVDescGPU);
IMGUI_IMPL_API void ImGui_ImplDX12_Shutdown();
IMGUI_IMPL_API void ImGui_ImplDX12_NewFrame(ID3D12CommandQueue* apCommandQueue);
IMGUI_IMPL_API void ImGui_ImplDX12_RenderDrawData(ImDrawData* apDrawData, ID3D12GraphicsCommandList* apCommandLists);

// Use if you want to reset your rendering device without losing Dear ImGui state.
IMGUI_IMPL_API void ImGui_ImplDX12_InvalidateDeviceObjects();
IMGUI_IMPL_API bool ImGui_ImplDX12_CreateDeviceObjects(ID3D12CommandQueue* apCommandQueue);

// Use to recreate font texture during runtime. Call before NewFrame().Should switch to https://github.com/ocornut/imgui/pull/3761 once it's implemented.
IMGUI_IMPL_API void ImGui_ImplDX12_RecreateFontsTexture(ID3D12CommandQueue* apCommandQueue);