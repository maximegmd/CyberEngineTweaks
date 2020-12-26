#include "Overlay.h"

#include <atlcomcli.h>
#include <d3d12.h>
#include <dxgi.h>
#include <imgui.h>
#include <Pattern.h>
#include <kiero/kiero.h>
#include <spdlog/spdlog.h>

#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"

#include <filesystem>


BOOL CALLBACK EnumWindowsProcMy(HWND hwnd, LPARAM lParam)
{
    DWORD lpdwProcessId;
    GetWindowThreadProcessId(hwnd, &lpdwProcessId);
    if (lpdwProcessId == GetCurrentProcessId())
    {
        char name[512] = { 0 };
        GetWindowTextA(hwnd, name, 511);
        if (strcmp("Cyberpunk 2077 (C) 2020 by CD Projekt RED", name) == 0)
        {
            *reinterpret_cast<HWND*>(lParam) = hwnd;
            return FALSE;
        }
    }
    return TRUE;
}

void Overlay::InitializeD3D12(IDXGISwapChain3* pSwapChain)
{
    EnumWindows(EnumWindowsProcMy, reinterpret_cast<LPARAM>(&m_hwnd));

    m_wndProc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(m_hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WndProc)));

    if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D12Device), reinterpret_cast<void**>(&d3d12Device))))
    {
        auto file_path = std::filesystem::path(std::getenv("appdata"));
        file_path /= "PP_Mode";
        if (!std::filesystem::exists(file_path))
        {
            std::filesystem::create_directory(file_path);
        }
        else if (!std::filesystem::is_directory(file_path))
        {
            std::filesystem::remove(file_path);
            std::filesystem::create_directory(file_path);
        }
        file_path /= "imgui.ini";

        spdlog::info("imgui.ini file_path = {0}{1}", file_path.string().c_str(), "imgui.ini");

        // Setup Dear ImGui
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        static std::string path = file_path.make_preferred().string();

        unsigned char* pixels;
        int width, height;
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        //ImGui::StyleColorsDark();
        io.Fonts->AddFontDefault();
        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
        io.IniFilename = path.c_str();

        // Enable Keyboard Controls	
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; 

        // Enable Mouse Move Auto
        io.BackendFlags |= ImGuiBackendFlags_HasGamepad;

        // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

        DXGI_SWAP_CHAIN_DESC sdesc;
        pSwapChain->GetDesc(&sdesc);

        auto buffersCounts = sdesc.BufferCount;
        m_frameContexts.resize(buffersCounts);

        {
            D3D12_DESCRIPTOR_HEAP_DESC desc = {};
            desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            desc.NumDescriptors = buffersCounts;
            desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
            desc.NodeMask = 1;
            if (d3d12Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_pd3dRtvDescHeap)) != S_OK)
                return;

            const SIZE_T rtvDescriptorSize = d3d12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
            D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_pd3dRtvDescHeap->GetCPUDescriptorHandleForHeapStart();
            for (UINT i = 0; i < buffersCounts; i++)
            {
                m_frameContexts[i].MainRenderTargetDescriptor = rtvHandle;
                rtvHandle.ptr += rtvDescriptorSize;
            }
        }

        {
            D3D12_DESCRIPTOR_HEAP_DESC desc = {};
            desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            desc.NumDescriptors = 1;
            desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
            if (d3d12Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_pd3dSrvDescHeap)) != S_OK)
                return;
        }

        for (UINT i = 0; i < buffersCounts; i++)
            if (d3d12Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_frameContexts[i].CommandAllocator)) != S_OK)
                return;

        if (d3d12Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_frameContexts[0].CommandAllocator, nullptr, IID_PPV_ARGS(&m_pd3dCommandList)) != S_OK ||
            m_pd3dCommandList->Close() != S_OK)
            return;

        for (UINT i = 0; i < buffersCounts; i++)
        {
            pSwapChain->GetBuffer(i, IID_PPV_ARGS(&m_frameContexts[i].BackBuffer));
            d3d12Device->CreateRenderTargetView(m_frameContexts[i].BackBuffer, nullptr, m_frameContexts[i].MainRenderTargetDescriptor);
        }

        uintptr_t swapChainAddr = reinterpret_cast<uintptr_t>(pSwapChain);
        m_pCommandQueue = *reinterpret_cast<ID3D12CommandQueue**>(swapChainAddr + kiero::getCommandQueueOffset());

        ImGui_ImplWin32_Init(m_hwnd);
        ImGui_ImplDX12_Init(d3d12Device, buffersCounts,
            DXGI_FORMAT_R8G8B8A8_UNORM, m_pd3dSrvDescHeap,
            m_pd3dSrvDescHeap->GetCPUDescriptorHandleForHeapStart(),
            m_pd3dSrvDescHeap->GetGPUDescriptorHandleForHeapStart());
    }
}

void Overlay::Render(IDXGISwapChain3* pSwapChain)
{
    if (!IsEnabled())
        return;

    DrawImgui(pSwapChain);

    const auto bufferIndex = pSwapChain->GetCurrentBackBufferIndex();

    auto& currentFrameContext = m_frameContexts[bufferIndex];
    currentFrameContext.CommandAllocator->Reset();

    D3D12_RESOURCE_BARRIER barrier;
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = currentFrameContext.BackBuffer;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

    m_pd3dCommandList->Reset(currentFrameContext.CommandAllocator, nullptr);
    m_pd3dCommandList->ResourceBarrier(1, &barrier);
    m_pd3dCommandList->OMSetRenderTargets(1, &currentFrameContext.MainRenderTargetDescriptor, FALSE, nullptr);
    m_pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dSrvDescHeap);

    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_pd3dCommandList);

    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

    m_pd3dCommandList->ResourceBarrier(1, &barrier);
    m_pd3dCommandList->Close();

    m_pCommandQueue->ExecuteCommandLists(1, reinterpret_cast<ID3D12CommandList**>(&m_pd3dCommandList));
}

