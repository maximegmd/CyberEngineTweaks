#include "Overlay.h"

#include <atlcomcli.h>
#include <d3d12.h>
#include <dxgi.h>
#include <imgui.h>
#include <memory>
#include <kiero/kiero.h>
#include <spdlog/spdlog.h>

#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"


static std::shared_ptr<Overlay> s_pOverlay;

void Overlay::Initialize()
{
    if (!s_pOverlay)
    {
        s_pOverlay.reset(new (std::nothrow) Overlay);
        s_pOverlay->Hook();
    }
}

void Overlay::Shutdown()
{
    s_pOverlay = nullptr;
}

void Overlay::InitializeD3D12(IDXGISwapChain3* pSwapChain)
{
    ID3D12Device* d3d12Device = nullptr;

    auto wnd = GetActiveWindow();

    if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D12Device), (void**)&d3d12Device)))
    {
        ImGui::CreateContext();

        unsigned char* pixels;
        int width, height;
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        ImGui::StyleColorsDark();
        io.Fonts->AddFontDefault();
        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
        io.IniFilename = NULL;

        CreateEvent(nullptr, false, false, nullptr);

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

            SIZE_T rtvDescriptorSize = d3d12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
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

        ImGui_ImplWin32_Init(wnd);
        ImGui_ImplDX12_Init(d3d12Device, buffersCounts,
            DXGI_FORMAT_R8G8B8A8_UNORM, m_pd3dSrvDescHeap,
            m_pd3dSrvDescHeap->GetCPUDescriptorHandleForHeapStart(),
            m_pd3dSrvDescHeap->GetGPUDescriptorHandleForHeapStart());
    }
}

void Overlay::Render(IDXGISwapChain3* pSwapChain)
{
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("ImGui Menu");

    static bool no_titlebar = false;
    float test = 0.f;

    ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.65f);
    ImGui::PushItemWidth(-140);

    if (ImGui::CollapsingHeader("MENU"))
    {
        if (ImGui::TreeNode("SUB MENU"))
        {
            ImGui::Text("Text Test");
            if (ImGui::Button("Button Test")) {}
            ImGui::Checkbox("CheckBox Test", &no_titlebar);
            ImGui::SliderFloat("Slider Test", &test, 1.0f, 100.0f);

            ImGui::TreePop();
        }
    }

    ImGui::End();

    ImGui::Render();

    UINT bufferIndex = pSwapChain->GetCurrentBackBufferIndex();

    FrameContext& currentFrameContext = m_frameContexts[bufferIndex];
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

    auto pCommandQueue = *(ID3D12CommandQueue**)((uintptr_t)pSwapChain + kiero::getCommandQueueOffset());

    pCommandQueue->ExecuteCommandLists(1, reinterpret_cast<ID3D12CommandList**>(&m_pd3dCommandList));
}

long Overlay::PresentD3D12(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags)
{
    static bool init = false;
    if (!init)
    {
        s_pOverlay->InitializeD3D12(pSwapChain);

        init = true;
    }
    
    s_pOverlay->Render(pSwapChain);

    return s_pOverlay->m_realPresentD3D12(pSwapChain, SyncInterval, Flags);
}

void Overlay::Hook()
{
    if (kiero::bind(140, reinterpret_cast<void**>(&m_realPresentD3D12), PresentD3D12) != kiero::Status::Success)
        spdlog::error("\tD3D12 PresentD3D12 Hook failed!");

    spdlog::info("\tD3D12 hook complete");
}

Overlay::Overlay() = default;

Overlay::~Overlay() = default;

