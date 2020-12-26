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

bool Overlay::InitializeD3D12(IDXGISwapChain3* pSwapChain)
{
    static auto checkCmdQueue = [](Overlay* overlay) 
    {
        if (overlay->m_pCommandQueue == nullptr) 
        {
            auto swapChainAddr = reinterpret_cast<uintptr_t>(*(&overlay->m_pdxgiSwapChain));
            overlay->m_pCommandQueue = *reinterpret_cast<ID3D12CommandQueue**>(swapChainAddr + kiero::getCommandQueueOffset());
            if (overlay->m_pCommandQueue != nullptr) 
            {
                auto desc = overlay->m_pCommandQueue->GetDesc();
                if(desc.Type != D3D12_COMMAND_LIST_TYPE_DIRECT) 
                {
                    overlay->m_pCommandQueue = nullptr;
                    spdlog::warn("\tOverlay::InitializeD3D12() - invalid type of command list!");
                    return false;
                }
                return true;
            }
            spdlog::warn("\tOverlay::InitializeD3D12() - swap chain is missing command queue!");
            return false;
        }
        return true;
    };

    static auto reset = [](Overlay* overlay) 
    {
        overlay->m_frameContexts.clear();
        overlay->m_pdxgiSwapChain = nullptr;
        overlay->m_pd3d12Device = nullptr;
        overlay->m_pd3dRtvDescHeap = nullptr;
        overlay->m_pd3dSrvDescHeap = nullptr;
        overlay->m_pd3dCommandList = nullptr;
        // NOTE: not clearing m_hWnd, m_wndProc and m_pCommandQueue, as these should be persistent once set till the EOL of Overlay
        return false;
    };

    // Window hook (repeated till successful, should be on first call)
    if (m_hWnd == nullptr) 
    {
        if (EnumWindows(EnumWindowsProcMy, reinterpret_cast<LPARAM>(&m_hWnd)))
            spdlog::error("\tOverlay::InitializeD3D12() - window hook failed!");
        else 
        {
            m_wndProc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(m_hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WndProc)));
            spdlog::info("\tOverlay::InitializeD3D12() - window hook complete.");
        }
    }

    if (!pSwapChain)
        return false;

    if (m_initialized) 
    {
        if (m_pdxgiSwapChain != pSwapChain)
        {
            spdlog::warn("\tOverlay::InitializeD3D12() - multiple swap chains detected! Currently hooked to {0}, this call was from {1}.", reinterpret_cast<void*>(*(&m_pdxgiSwapChain)), reinterpret_cast<void*>(pSwapChain));
            return false;
        }
        if (!checkCmdQueue(this))
        {
            spdlog::error("\tOverlay::InitializeD3D12() - missing command queue!");
            return false;
        }
        return true;
    }

    m_pdxgiSwapChain = pSwapChain;

    if (FAILED(m_pdxgiSwapChain->GetDevice(IID_PPV_ARGS(&m_pd3d12Device))))
    {
        spdlog::error("\tOverlay::InitializeD3D12() - failed to get device!");
        return reset(this);
    }

    unsigned char* pixels;
    int width, height;

    DXGI_SWAP_CHAIN_DESC sdesc;
    m_pdxgiSwapChain->GetDesc(&sdesc);

    if (sdesc.OutputWindow != m_hWnd) 
        spdlog::warn("\tOverlay::InitializeD3D12() - output window of current swap chain does not match hooked window! Currently hooked to {0} while swap chain output window is {1}.", reinterpret_cast<void*>(m_hWnd), reinterpret_cast<void*>(sdesc.OutputWindow));

    auto buffersCounts = sdesc.BufferCount;
    m_frameContexts.resize(buffersCounts);
    for (UINT i = 0; i < buffersCounts; i++)
        m_pdxgiSwapChain->GetBuffer(i, IID_PPV_ARGS(&m_frameContexts[i].BackBuffer));

    D3D12_DESCRIPTOR_HEAP_DESC rtvdesc = {};
    rtvdesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvdesc.NumDescriptors = buffersCounts;
    rtvdesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    rtvdesc.NodeMask = 1;
    if (FAILED(m_pd3d12Device->CreateDescriptorHeap(&rtvdesc, IID_PPV_ARGS(&m_pd3dRtvDescHeap))))
    {
        spdlog::error("\tOverlay::InitializeD3D12() - failed to create RTV descriptor heap!");
        return reset(this);
    }

    const SIZE_T rtvDescriptorSize = m_pd3d12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_pd3dRtvDescHeap->GetCPUDescriptorHandleForHeapStart();
    for (auto& context : m_frameContexts)
    {
        context.MainRenderTargetDescriptor = rtvHandle;
        rtvHandle.ptr += rtvDescriptorSize;
    }

    D3D12_DESCRIPTOR_HEAP_DESC srvdesc = {};
    srvdesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvdesc.NumDescriptors = 1;
    srvdesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    if (FAILED(m_pd3d12Device->CreateDescriptorHeap(&srvdesc, IID_PPV_ARGS(&m_pd3dSrvDescHeap))))
    {
        spdlog::error("\tOverlay::InitializeD3D12() - failed to create SRV descriptor heap!");
        return reset(this);
    }
    
    for (auto& context : m_frameContexts)
        if (FAILED(m_pd3d12Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&context.CommandAllocator))))
        {
            spdlog::error("\tOverlay::InitializeD3D12() - failed to create command allocator!");
            return reset(this);
        }

    if (FAILED(m_pd3d12Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_frameContexts[0].CommandAllocator, nullptr, IID_PPV_ARGS(&m_pd3dCommandList))) ||
        FAILED(m_pd3dCommandList->Close()))
    {
        spdlog::error("\tOverlay::InitializeD3D12() - failed to create command list!");
        return reset(this);
    }

    for (auto& context : m_frameContexts)
        m_pd3d12Device->CreateRenderTargetView(context.BackBuffer, nullptr, context.MainRenderTargetDescriptor);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::StyleColorsDark();
    io.Fonts->AddFontDefault();
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    io.IniFilename = NULL;

    if (!ImGui_ImplWin32_Init(m_hWnd)) 
    {
        spdlog::error("\tOverlay::InitializeD3D12() - ImGui_ImplWin32_Init call failed!");
        ImGui::DestroyContext();
        return reset(this);
    }

    if (!ImGui_ImplDX12_Init(m_pd3d12Device, buffersCounts,
        DXGI_FORMAT_R8G8B8A8_UNORM, m_pd3dSrvDescHeap,
        m_pd3dSrvDescHeap->GetCPUDescriptorHandleForHeapStart(),
        m_pd3dSrvDescHeap->GetGPUDescriptorHandleForHeapStart()))
    {
        spdlog::error("\tOverlay::InitializeD3D12() - ImGui_ImplDX12_Init call failed!");
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
        return reset(this);
    }

    if (!ImGui_ImplDX12_CreateDeviceObjects()) 
    {
        spdlog::error("\tOverlay::InitializeD3D12() - ImGui_ImplDX12_CreateDeviceObjects call failed!");
        ImGui_ImplDX12_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
        return reset(this);
    }

    spdlog::info("\tOverlay::InitializeD3D12() - initialization successful!");
    m_initialized = true;

    if (!checkCmdQueue(this))
    {
        spdlog::error("\tOverlay::InitializeD3D12() - missing command queue!");
        return false;
    }

    return true;
}

void Overlay::Render(IDXGISwapChain3* pSwapChain)
{
    if (!IsEnabled())
        return;

    DrawImgui(pSwapChain);

    const auto bufferIndex = pSwapChain->GetCurrentBackBufferIndex();
    auto& frameContext = m_frameContexts[bufferIndex];
    frameContext.CommandAllocator->Reset();

    D3D12_RESOURCE_BARRIER barrier;
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = frameContext.BackBuffer;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

    m_pd3dCommandList->Reset(frameContext.CommandAllocator, nullptr);
    m_pd3dCommandList->ResourceBarrier(1, &barrier);
    m_pd3dCommandList->OMSetRenderTargets(1, &frameContext.MainRenderTargetDescriptor, FALSE, nullptr);
    m_pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dSrvDescHeap);

    ImGui::Render();
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_pd3dCommandList);

    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

    m_pd3dCommandList->ResourceBarrier(1, &barrier);
    m_pd3dCommandList->Close();

    m_pCommandQueue->ExecuteCommandLists(1, reinterpret_cast<ID3D12CommandList**>(&m_pd3dCommandList));
}

