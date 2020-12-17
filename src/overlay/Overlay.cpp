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

ID3D12GraphicsCommandList* g_pd3dCommandList;
ID3D12CommandQueue* d3d12CommandQueue = nullptr;
ID3D12Fence* d3d12Fence = nullptr;
UINT64 d3d12FenceValue = 0;

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

uint32_t frameId = 0;

void Overlay::ExecuteCommandListsD3D12(ID3D12CommandQueue* apCommandQueue, UINT NumCommandLists, ID3D12CommandList** ppCommandLists)
{
    /*D3D12_COMMAND_QUEUE_DESC desc = apCommandQueue->GetDesc();
    spdlog::info("Queue {:X} Type: {} Flags: {} Node Mask: {}", (uint64_t)apCommandQueue, desc.Type, desc.Flags, desc.NodeMask);

    //spdlog::info("Command queue {:X}", (uint64_t)apCommandQueue);
    if(frameId < 90)
    {
        if(desc.Type == D3D12_COMMAND_LIST_TYPE_DIRECT && desc.Flags == D3D12_COMMAND_QUEUE_FLAG_NONE)
            d3d12CommandQueue = apCommandQueue;
    }*/

    s_pOverlay->m_realExecuteCommandListsD3D12(apCommandQueue, NumCommandLists, ppCommandLists);
}

HRESULT Overlay::SignalD3D12(ID3D12CommandQueue* apCommandQueue, ID3D12Fence* pFence, UINT64 Value)
{
    if (d3d12CommandQueue != nullptr && apCommandQueue == d3d12CommandQueue)
    {
        d3d12Fence = pFence;
        d3d12FenceValue = Value;
    }

    return s_pOverlay->m_realSignalD3D12(apCommandQueue, pFence, Value);
}

void Overlay::DrawInstancedD3D12(ID3D12GraphicsCommandList* dCommandList, UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation, UINT StartInstanceLocation)
{
    s_pOverlay->m_realDrawInstancedD3D12(dCommandList, VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);
}

void Overlay::DrawIndexedInstancedD3D12(ID3D12GraphicsCommandList* dCommandList, UINT IndexCount, UINT InstanceCount, UINT StartIndex, INT BaseVertex)
{
    s_pOverlay->m_realDrawIndexedInstancedD3D12(dCommandList, IndexCount, InstanceCount, StartIndex, BaseVertex);
}

ID3D12Device* d3d12Device = nullptr;
bool init = false;

struct FrameContext
{
    ID3D12CommandAllocator* CommandAllocator = nullptr;
    ID3D12Resource* main_render_target_resource = nullptr;
    D3D12_CPU_DESCRIPTOR_HANDLE main_render_target_descriptor;
};

uint32_t buffersCounts = -1;
std::vector<FrameContext> frameContext;

ID3D12DescriptorHeap* g_pd3dRtvDescHeap = nullptr;
ID3D12DescriptorHeap* g_pd3dSrvDescHeap;

long Overlay::PresentD3D12(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags)
{
    if(frameId++ < 90)
        return s_pOverlay->m_realPresentD3D12(pSwapChain, SyncInterval, Flags);

    if (!init)
    {
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

            buffersCounts = sdesc.BufferCount;
            frameContext.resize(buffersCounts);

            {
                D3D12_DESCRIPTOR_HEAP_DESC desc = {};
                desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
                desc.NumDescriptors = buffersCounts;
                desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
                desc.NodeMask = 1;
                if (d3d12Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pd3dRtvDescHeap)) != S_OK)
                    return false;

                SIZE_T rtvDescriptorSize = d3d12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
                D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = g_pd3dRtvDescHeap->GetCPUDescriptorHandleForHeapStart();
                for (UINT i = 0; i < buffersCounts; i++)
                {
                    frameContext[i].main_render_target_descriptor = rtvHandle;
                    rtvHandle.ptr += rtvDescriptorSize;
                }
            }

            {
                D3D12_DESCRIPTOR_HEAP_DESC desc = {};
                desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
                desc.NumDescriptors = 1;
                desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
                if (d3d12Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pd3dSrvDescHeap)) != S_OK)
                    return false;
            }

            for (UINT i = 0; i < buffersCounts; i++)
                if (d3d12Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&frameContext[i].CommandAllocator)) != S_OK)
                    return false;

            if (d3d12Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, frameContext[0].CommandAllocator, nullptr, IID_PPV_ARGS(&g_pd3dCommandList)) != S_OK ||
                g_pd3dCommandList->Close() != S_OK)
                return false;

            D3D12_COMMAND_QUEUE_DESC desc = {};
            desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
            desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
            desc.NodeMask = 1;
            if (d3d12Device->CreateCommandQueue(&desc, IID_PPV_ARGS(&d3d12CommandQueue)) != S_OK)
                return false;

            ImGui_ImplWin32_Init(wnd);
            ImGui_ImplDX12_Init(d3d12Device, buffersCounts,
                DXGI_FORMAT_R8G8B8A8_UNORM, g_pd3dSrvDescHeap,
                g_pd3dSrvDescHeap->GetCPUDescriptorHandleForHeapStart(),
                g_pd3dSrvDescHeap->GetGPUDescriptorHandleForHeapStart());

            ImGui_ImplDX12_CreateDeviceObjects();
        }
        init = true;
    }
    
    {
        if (d3d12CommandQueue == nullptr)
            return s_pOverlay->m_realPresentD3D12(pSwapChain, SyncInterval, Flags);

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
        
        FrameContext& currentFrameContext = frameContext[pSwapChain->GetCurrentBackBufferIndex()];
        currentFrameContext.CommandAllocator->Reset();

        D3D12_RESOURCE_BARRIER barrier;
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = currentFrameContext.main_render_target_resource;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

        g_pd3dCommandList->Reset(currentFrameContext.CommandAllocator, nullptr);
        g_pd3dCommandList->ResourceBarrier(1, &barrier);
        g_pd3dCommandList->OMSetRenderTargets(1, &currentFrameContext.main_render_target_descriptor, FALSE, nullptr);
        g_pd3dCommandList->SetDescriptorHeaps(1, &g_pd3dSrvDescHeap);
        
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), g_pd3dCommandList);

        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

        g_pd3dCommandList->ResourceBarrier(1, &barrier);
        g_pd3dCommandList->Close();

        spdlog::info("{:X}", (uint64_t)d3d12CommandQueue);

        s_pOverlay->m_realExecuteCommandListsD3D12(d3d12CommandQueue, 1, reinterpret_cast<ID3D12CommandList**>(&g_pd3dCommandList));

        spdlog::info("Test {}", pSwapChain->GetCurrentBackBufferIndex());
        spdlog::default_logger()->flush();
    }

    return s_pOverlay->m_realPresentD3D12(pSwapChain, SyncInterval, Flags);
}

void Overlay::Hook()
{
    if(kiero::bind(54, reinterpret_cast<void**>(&m_realExecuteCommandListsD3D12), ExecuteCommandListsD3D12) != kiero::Status::Success)
        spdlog::error("\tD3D12 ExecuteCommandListsD3D12 Hook failed!");

    if(kiero::bind(58, reinterpret_cast<void**>(&m_realSignalD3D12), SignalD3D12) != kiero::Status::Success)
        spdlog::error("\tD3D12 SignalD3D12 Hook failed!");

    if(kiero::bind(84, reinterpret_cast<void**>(&m_realDrawInstancedD3D12), DrawInstancedD3D12) != kiero::Status::Success)
        spdlog::error("\tD3D12 DrawInstancedD3D12 Hook failed!");

    if(kiero::bind(85, reinterpret_cast<void**>(&m_realDrawIndexedInstancedD3D12), DrawIndexedInstancedD3D12) != kiero::Status::Success)
        spdlog::error("\tD3D12 DrawIndexedInstancedD3D12 Hook failed!");

    if(kiero::bind(140, reinterpret_cast<void**>(&m_realPresentD3D12), PresentD3D12) != kiero::Status::Success)
        spdlog::error("\tD3D12 PresentD3D12 Hook failed!");

    spdlog::info("\tD3D12 hook complete");
}

Overlay::Overlay() = default;

Overlay::~Overlay() = default;

