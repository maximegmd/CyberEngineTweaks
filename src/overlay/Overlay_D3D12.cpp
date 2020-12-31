#include <stdafx.h>

#include "Overlay.h"

#include <kiero/kiero.h>

#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"

static BOOL CALLBACK EnumWindowsProcMy(HWND hwnd, LPARAM lParam)
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

bool Overlay::ResetD3D12State()
{
    if (m_initialized)
    {   
        m_initialized = false;
        ImGui_ImplDX12_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }
    m_frameContexts.clear();
    m_downlevelBackbuffers.clear();
    m_pdxgiSwapChain = nullptr;
    m_pd3d12Device = nullptr;
    m_pd3dRtvDescHeap = nullptr;
    m_pd3dSrvDescHeap = nullptr;
    m_pd3dCommandList = nullptr;
    m_downlevelBufferIndex = 0;
    m_outWidth = 0;
    m_outHeight = 0;
    // NOTE: not clearing m_hWnd, m_wndProc and m_pCommandQueue, as these should be persistent once set till the EOL of Overlay
    return false;
}

bool Overlay::InitializeD3D12(IDXGISwapChain* pSwapChain)
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
                    spdlog::warn("Overlay::InitializeD3D12() - invalid type of command list!");
                    return false;
                }
                return true;
            }
            spdlog::warn("Overlay::InitializeD3D12() - swap chain is missing command queue!");
            return false;
        }
        return true;
    };

    // Window hook (repeated till successful, should be on first call)
    if (m_hWnd == nullptr) 
    {
        if (EnumWindows(EnumWindowsProcMy, reinterpret_cast<LPARAM>(&m_hWnd)))
            spdlog::error("Overlay::InitializeD3D12() - window hook failed!");
        else 
        {
            m_wndProc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(m_hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WndProc)));
            spdlog::info("Overlay::InitializeD3D12() - window hook complete.");
        }
    }

    if (!pSwapChain)
        return false;

    if (m_initialized) 
    {
        CComPtr<IDXGISwapChain3> pSwapChain3;
		if (FAILED(pSwapChain->QueryInterface(IID_PPV_ARGS(&pSwapChain3))))
		{
			spdlog::error("Overlay::InitializeD3D12() - unable to query pSwapChain interface for IDXGISwapChain3! (pSwapChain = {0})", reinterpret_cast<void*>(pSwapChain));
            return false;
		}
        if (m_pdxgiSwapChain != pSwapChain3)
        {
            spdlog::warn("Overlay::InitializeD3D12() - multiple swap chains detected! Currently hooked to {0}, this call was from {1}.", reinterpret_cast<void*>(*(&m_pdxgiSwapChain)), reinterpret_cast<void*>(pSwapChain));
            return false;
        }
        if (!checkCmdQueue(this))
        {
            spdlog::error("Overlay::InitializeD3D12() - missing command queue!");
            return false;
        }
        return true;
    }

    if (FAILED(pSwapChain->QueryInterface(IID_PPV_ARGS(&m_pdxgiSwapChain))))
    {
        spdlog::error("Overlay::InitializeD3D12() - unable to query pSwapChain interface for IDXGISwapChain3! (pSwapChain = {0})", reinterpret_cast<void*>(pSwapChain));
        return ResetD3D12State();
    }

    if (FAILED(m_pdxgiSwapChain->GetDevice(IID_PPV_ARGS(&m_pd3d12Device))))
    {
        spdlog::error("Overlay::InitializeD3D12() - failed to get device!");
        return ResetD3D12State();
    }

    DXGI_SWAP_CHAIN_DESC sdesc;
    m_pdxgiSwapChain->GetDesc(&sdesc);

    if (sdesc.OutputWindow != m_hWnd) 
        spdlog::warn("Overlay::InitializeD3D12() - output window of current swap chain does not match hooked window! Currently hooked to {0} while swap chain output window is {1}.", reinterpret_cast<void*>(m_hWnd), reinterpret_cast<void*>(sdesc.OutputWindow));

    m_outWidth = sdesc.BufferDesc.Width;
    m_outHeight = sdesc.BufferDesc.Height;

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
        spdlog::error("Overlay::InitializeD3D12() - failed to create RTV descriptor heap!");
        return ResetD3D12State();
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
        spdlog::error("Overlay::InitializeD3D12() - failed to create SRV descriptor heap!");
        return ResetD3D12State();
    }
    
    for (auto& context : m_frameContexts)
        if (FAILED(m_pd3d12Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&context.CommandAllocator))))
        {
            spdlog::error("Overlay::InitializeD3D12() - failed to create command allocator!");
            return ResetD3D12State();
        }

    if (FAILED(m_pd3d12Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_frameContexts[0].CommandAllocator, nullptr, IID_PPV_ARGS(&m_pd3dCommandList))) ||
        FAILED(m_pd3dCommandList->Close()))
    {
        spdlog::error("Overlay::InitializeD3D12() - failed to create command list!");
        return ResetD3D12State();
    }

    for (auto& context : m_frameContexts)
        m_pd3d12Device->CreateRenderTargetView(context.BackBuffer, nullptr, context.MainRenderTargetDescriptor);

    if (!InitializeImGui(buffersCounts))
    {
        spdlog::error("Overlay::InitializeD3D12() - failed to initialize ImGui!");
        return ResetD3D12State();
    }

    spdlog::info("Overlay::InitializeD3D12() - initialization successful!");
    m_initialized = true;

    if (!checkCmdQueue(this))
    {
        spdlog::error("Overlay::InitializeD3D12() - missing command queue!");
        return false;
    }

    return true;
}

bool Overlay::InitializeD3D12Downlevel(ID3D12CommandQueue* pCommandQueue, ID3D12Resource* pSourceTex2D, HWND hWindow)
{
    // Window hook (repeated till successful, should be on first call)
    if (m_hWnd == nullptr) 
    {
        if (EnumWindows(EnumWindowsProcMy, reinterpret_cast<LPARAM>(&m_hWnd)))
            spdlog::error("Overlay::InitializeD3D12Downlevel() - window hook failed!");
        else 
        {
            m_wndProc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(m_hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WndProc)));
            spdlog::info("Overlay::InitializeD3D12Downlevel() - window hook complete.");
        }
    }

    if (!pCommandQueue || !pSourceTex2D)
        return false;

    if (m_initialized) 
        return true;

    auto cmdQueueDesc = pCommandQueue->GetDesc();
    if(cmdQueueDesc.Type != D3D12_COMMAND_LIST_TYPE_DIRECT) 
    {
        spdlog::warn("Overlay::InitializeD3D12Downlevel() - ignoring command queue - invalid type of command list!");
        return false;
    }

    m_pCommandQueue = pCommandQueue;

    if (hWindow != m_hWnd) 
        spdlog::warn("Overlay::InitializeD3D12Downlevel() - current output window does not match hooked window! Currently hooked to {0} while current output window is {1}.", reinterpret_cast<void*>(m_hWnd), reinterpret_cast<void*>(hWindow));

    if (FAILED(pSourceTex2D->GetDevice(IID_PPV_ARGS(&m_pd3d12Device))))
    {
        spdlog::error("Overlay::InitializeD3D12Downlevel() - failed to get device!");
        return ResetD3D12State();
    }

    // Limit to at most 3 buffers
    const auto buffersCounts = std::min<size_t>(m_downlevelBackbuffers.size(), 3);
    m_frameContexts.resize(buffersCounts);
    if (buffersCounts == 0)
    {
        spdlog::error("Overlay::InitializeD3D12Downlevel() - no backbuffers were found!");
        return ResetD3D12State();
    }

    D3D12_DESCRIPTOR_HEAP_DESC rtvdesc;
    rtvdesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvdesc.NumDescriptors = static_cast<UINT>(buffersCounts);
    rtvdesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    rtvdesc.NodeMask = 1;
    if (FAILED(m_pd3d12Device->CreateDescriptorHeap(&rtvdesc, IID_PPV_ARGS(&m_pd3dRtvDescHeap))))
    {
        spdlog::error("Overlay::InitializeD3D12Downlevel() - failed to create RTV descriptor heap!");
        return ResetD3D12State();
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
        spdlog::error("Overlay::InitializeD3D12Downlevel() - failed to create SRV descriptor heap!");
        return ResetD3D12State();
    }
    
    for (auto& context : m_frameContexts)
    {
        if (FAILED(m_pd3d12Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&context.CommandAllocator))))
        {
            spdlog::error("Overlay::InitializeD3D12Downlevel() - failed to create command allocator!");
            return ResetD3D12State();
        }
    }

    if (FAILED(m_pd3d12Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_frameContexts[0].CommandAllocator, nullptr, IID_PPV_ARGS(&m_pd3dCommandList))))
    {
        spdlog::error("Overlay::InitializeD3D12Downlevel() - failed to create command list!");
        return ResetD3D12State();
    }

    if (FAILED(m_pd3dCommandList->Close()))
    {
        spdlog::error("Overlay::InitializeD3D12Downlevel() - failed to close command list!");
        return ResetD3D12State();
    }

    // Skip the first N - 3 buffers as they are no longer in use
    auto skip = m_downlevelBackbuffers.size() - buffersCounts;
    for (size_t i = 0; i < buffersCounts; i++)
    {
        auto& context = m_frameContexts[i];
        context.BackBuffer = m_downlevelBackbuffers[i + skip];
        m_pd3d12Device->CreateRenderTargetView(context.BackBuffer, nullptr, context.MainRenderTargetDescriptor);
    }

    if (!InitializeImGui(buffersCounts))
    {
        spdlog::error("Overlay::InitializeD3D12Downlevel() - failed to initialize ImGui!");
        return ResetD3D12State();
    }

    spdlog::info("Overlay::InitializeD3D12Downlevel() - initialization successful!");
    m_initialized = true;

    return true;
}

bool Overlay::InitializeImGui(size_t buffersCounts)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::StyleColorsDark();
    io.Fonts->AddFontDefault();
    io.IniFilename = NULL;

    if (!ImGui_ImplWin32_Init(m_hWnd)) 
    {
        spdlog::error("Overlay::InitializeImGui() - ImGui_ImplWin32_Init call failed!");
        ImGui::DestroyContext();
        return false;
    }

    if (!ImGui_ImplDX12_Init(m_pd3d12Device, static_cast<int>(buffersCounts),
        DXGI_FORMAT_R8G8B8A8_UNORM, m_pd3dSrvDescHeap,
        m_pd3dSrvDescHeap->GetCPUDescriptorHandleForHeapStart(),
        m_pd3dSrvDescHeap->GetGPUDescriptorHandleForHeapStart()))
    {
        spdlog::error("Overlay::InitializeImGui() - ImGui_ImplDX12_Init call failed!");
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
        return false;
    }

    if (!ImGui_ImplDX12_CreateDeviceObjects()) 
    {
        spdlog::error("Overlay::InitializeImGui() - ImGui_ImplDX12_CreateDeviceObjects call failed!");
        ImGui_ImplDX12_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
        return false;
    }

    return true;
}

void Overlay::Render()
{
    if (!IsEnabled())
        return;

    DrawImgui();

    const auto bufferIndex = (m_pdxgiSwapChain != nullptr) ? (m_pdxgiSwapChain->GetCurrentBackBufferIndex()) : (m_downlevelBufferIndex);
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

