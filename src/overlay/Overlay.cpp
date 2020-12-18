#include "Overlay.h"

#include <atlcomcli.h>
#include <d3d12.h>
#include <dxgi.h>
#include <Image.h>
#include <imgui.h>
#include <memory>
#include <MinHook.h>
#include <Pattern.h>
#include <kiero/kiero.h>
#include <spdlog/spdlog.h>

#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"


static std::shared_ptr<Overlay> s_pOverlay;

void Overlay::Initialize(Image* apImage)
{
    if (!s_pOverlay)
    {
        s_pOverlay.reset(new (std::nothrow) Overlay);
        s_pOverlay->EarlyHooks(apImage);
    }
}

void Overlay::Shutdown()
{
    s_pOverlay = nullptr;
}

Overlay& Overlay::Get()
{
    return *s_pOverlay;
}

void Overlay::EarlyHooks(Image* apImage)
{
    uint8_t* pLocation = FindSignature(apImage->pTextStart, apImage->pTextEnd, {
        0x48, 0x83, 0xEC, 0x38, 0x48, 0x8B, 0xCA, 0x44, 0x89, 0x44, 0x24,
        0x20, 0x48, 0x8D, 0x54, 0x24
    });

    if (pLocation)
    {
       if(MH_CreateHook(pLocation, &SetMousePosition, nullptr) != MH_OK || MH_EnableHook(pLocation) != MH_OK)
       {
           spdlog::error("\tCould not hook mouse center function!");
       }
       else
           spdlog::info("\tHook mouse center function!");
    }

    pLocation = FindSignature(apImage->pTextStart, apImage->pTextEnd, {
        0x48, 0x89, 0x5C, 0x24, 0x08, 0x57, 0x48, 0x83, 0xEC, 0x30, 0x48, 0x8B,
        0x99, 0x68, 0x01, 0x00, 0x00, 0x48, 0x8B, 0xF9, 0xFF });

    if (pLocation)
    {
        if (MH_CreateHook(pLocation, &ClipToCenter, (void**)&m_realClipToCenter) != MH_OK || MH_EnableHook(pLocation) != MH_OK)
        {
            spdlog::error("\tCould not hook mouse clip function!");
        }
        else
            spdlog::info("\tHook mouse clip function!");
    }

    
}

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

WNDPROC	oWndProc;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT APIENTRY WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))
        return true;

    switch (uMsg)
    {
    case WM_KEYDOWN:
        switch (wParam)
        {
        case VK_END:
            s_pOverlay->Toggle();
            break;
        }
    default:
        break;
    }

    if(s_pOverlay->IsEnabled() && uMsg != WM_PAINT && uMsg != WM_ACTIVATE && uMsg != WM_QUIT)
    {
        return true;
    }

    return CallWindowProc(oWndProc, hwnd, uMsg, wParam, lParam);
}

void Overlay::InitializeD3D12(IDXGISwapChain3* pSwapChain)
{
    EnumWindows(EnumWindowsProcMy, reinterpret_cast<LPARAM>(&m_hwnd));

    oWndProc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(m_hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WndProc)));

    ID3D12Device* d3d12Device = nullptr;

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

        //CreateEvent(nullptr, false, false, nullptr);

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

        ImGui_ImplWin32_Init(m_hwnd);
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

    ImGui::PushItemWidth(600.f);
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

    auto* pCommandQueue = *reinterpret_cast<ID3D12CommandQueue**>(reinterpret_cast<uintptr_t>(pSwapChain) + kiero::getCommandQueueOffset());

    pCommandQueue->ExecuteCommandLists(1, reinterpret_cast<ID3D12CommandList**>(&m_pd3dCommandList));
}

long Overlay::PresentD3D12(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags)
{
    static std::once_flag s_init;
    std::call_once(s_init, [pSwapChain]()
    {
        s_pOverlay->InitializeD3D12(pSwapChain);
    });

    s_pOverlay->Render(pSwapChain);

    return s_pOverlay->m_realPresentD3D12(pSwapChain, SyncInterval, Flags);
}

BOOL Overlay::SetMousePosition(void* apThis, HWND Wnd, long X, long Y)
{
    if(s_pOverlay->IsEnabled())
        return TRUE;

    POINT point;
    point.x = X;
    point.y = Y;
    ClientToScreen(Wnd, &point);
    return SetCursorPos(point.x, point.y);
}

struct SomeStruct
{
    uint8_t pad0[0x140];
    uint32_t unk140;
    uint8_t pad144[0x164 - 0x144];
    uint32_t unk164;
    HWND Wnd;
    uint8_t pad170[0x9];
    uint8_t isClipped; 
};

static_assert(offsetof(SomeStruct, unk140) == 0x140);
static_assert(offsetof(SomeStruct, unk164) == 0x164);
static_assert(offsetof(SomeStruct, Wnd) == 0x168);
static_assert(offsetof(SomeStruct, isClipped) == 0x179);

BOOL Overlay::ClipToCenter(SomeStruct* apThis)
{
    auto wnd = apThis->Wnd;
    auto foreground = GetForegroundWindow();
    if(wnd == foreground && apThis->unk164 && !apThis->unk140 && !s_pOverlay->IsEnabled())
    {
        RECT rect;
        GetClientRect(wnd, &rect);
        ClientToScreen(wnd, (POINT*)&rect.left);
        ClientToScreen(wnd, (POINT*)&rect.right);
        rect.left = (rect.left + rect.right) / 2;
        rect.right = rect.left;
        rect.bottom = (rect.bottom + rect.top) / 2;
        rect.top = rect.bottom;
        apThis->isClipped = true;
        return ClipCursor(&rect);
    }

    if(apThis->isClipped)
    {
        apThis->isClipped = false;
        return ClipCursor(nullptr);
    }

    return 1;
}

void Overlay::Hook()
{
    if (kiero::bind(140, reinterpret_cast<void**>(&m_realPresentD3D12), PresentD3D12) != kiero::Status::Success)
        spdlog::error("\tD3D12 PresentD3D12 Hook failed!");

    spdlog::info("\tD3D12 hook complete");
}

void Overlay::Toggle()
{
    struct Singleton
    {
        uint8_t pad0[0xC0];
        SomeStruct* pSomeStruct;
    };

    m_enabled = !m_enabled;

    ShowCursor(m_enabled);

    Singleton* pSingleton = *(Singleton**)(0x40689D8ull + (uintptr_t)GetModuleHandleA(nullptr));
    ClipToCenter(pSingleton->pSomeStruct);
}

bool Overlay::IsEnabled()
{
    return m_enabled;
}


Overlay::Overlay() = default;

Overlay::~Overlay() = default;

