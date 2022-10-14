#include <stdafx.h>

#include "D3D12.h"
#include "Options.h"
#include "Utils.h"

#include <kiero/kiero.h>
#include <imgui_impl/dx12.h>
#include <imgui_impl/win32.h>

#include <window/window.h>

bool D3D12::ResetState(bool aClearDownlevelBackbuffers)
{
    if (m_initialized)
    {
        m_initialized = false;
        ImGui_ImplDX12_Shutdown();
        ImGui_ImplWin32_Shutdown();
    }

    m_frameContexts.clear();

    if (aClearDownlevelBackbuffers)
        m_downlevelBackbuffers.clear();

    m_pdxgiSwapChain = nullptr;
    m_pd3d12Device = nullptr;
    m_pd3dRtvDescHeap = nullptr;
    m_pd3dSrvDescHeap = nullptr;
    m_pd3dCommandList = nullptr;
    m_downlevelBufferIndex = 0;
    m_outSize = { 0, 0 };
    // NOTE: not clearing m_hWnd, m_wndProc and m_pCommandQueue, as these should be persistent once set till the EOL of D3D12
    return false;
}

bool D3D12::Initialize(IDXGISwapChain* apSwapChain)
{
    if (!apSwapChain)
        return false;

    const HWND hWnd = m_window.GetWindow();
    if (!hWnd)
    {
        Log::Warn("D3D12::InitializeDownlevel() - window not yet hooked!");
        return false;
    }

    if (m_initialized)
    {
        Microsoft::WRL::ComPtr<IDXGISwapChain3> pSwapChain3{ nullptr };
        if (FAILED(apSwapChain->QueryInterface(IID_PPV_ARGS(&pSwapChain3))))
        {
            Log::Error("D3D12::Initialize() - unable to query pSwapChain interface for IDXGISwapChain3! (pSwapChain = {:X})", reinterpret_cast<void*>(apSwapChain));
            return false;
        }
        if (m_pdxgiSwapChain != pSwapChain3)
        {
            Log::Warn("D3D12::Initialize() - multiple swap chains detected! Currently hooked to {0:X}, this call was from {1:X}.", reinterpret_cast<void*>(m_pdxgiSwapChain.Get()), reinterpret_cast<void*>(apSwapChain));
            return false;
        }
        {
            DXGI_SWAP_CHAIN_DESC sdesc;
            m_pdxgiSwapChain->GetDesc(&sdesc);

            if (hWnd != sdesc.OutputWindow)
                Log::Warn("D3D12::Initialize() - output window of current swap chain does not match hooked window! Currently hooked to {0} while swap chain output window is {1}.", reinterpret_cast<void*>(hWnd), reinterpret_cast<void*>(sdesc.OutputWindow));
        }

        return true;
    }

    if (FAILED(apSwapChain->QueryInterface(IID_PPV_ARGS(&m_pdxgiSwapChain))))
    {
        Log::Error("D3D12::Initialize() - unable to query pSwapChain interface for IDXGISwapChain3! (pSwapChain = {0})", reinterpret_cast<void*>(apSwapChain));
        return ResetState();
    }

    if (FAILED(m_pdxgiSwapChain->GetDevice(IID_PPV_ARGS(&m_pd3d12Device))))
    {
        Log::Error("D3D12::Initialize() - failed to get device!");
        return ResetState();
    }

    DXGI_SWAP_CHAIN_DESC sdesc;
    m_pdxgiSwapChain->GetDesc(&sdesc);

    if (hWnd != sdesc.OutputWindow)
        Log::Warn("D3D12::Initialize() - output window of current swap chain does not match hooked window! Currently hooked to {0} while swap chain output window is {1}.", reinterpret_cast<void*>(hWnd), reinterpret_cast<void*>(sdesc.OutputWindow));

    m_outSize = { static_cast<LONG>(sdesc.BufferDesc.Width), static_cast<LONG>(sdesc.BufferDesc.Height) };

    const auto buffersCounts = sdesc.BufferCount;
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
        Log::Error("D3D12::Initialize() - failed to create RTV descriptor heap!");
        return ResetState();
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
    srvdesc.NumDescriptors = 200;
    srvdesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    if (FAILED(m_pd3d12Device->CreateDescriptorHeap(&srvdesc, IID_PPV_ARGS(&m_pd3dSrvDescHeap))))
    {
        Log::Error("D3D12::Initialize() - failed to create SRV descriptor heap!");
        return ResetState();
    }

    for (auto& context : m_frameContexts)
        if (FAILED(m_pd3d12Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&context.CommandAllocator))))
        {
            Log::Error("D3D12::Initialize() - failed to create command allocator!");
            return ResetState();
        }

    if (FAILED(m_pd3d12Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_frameContexts[0].CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_pd3dCommandList))) ||
        FAILED(m_pd3dCommandList->Close()))
    {
        Log::Error("D3D12::Initialize() - failed to create command list!");
        return ResetState();
    }

    for (auto& context : m_frameContexts)
        m_pd3d12Device->CreateRenderTargetView(context.BackBuffer.Get(), nullptr, context.MainRenderTargetDescriptor);

    if (!InitializeImGui(buffersCounts))
    {
        Log::Error("D3D12::Initialize() - failed to initialize ImGui!");
        return ResetState();
    }

    Log::Info("D3D12::Initialize() - initialization successful!");
    m_initialized = true;

    OnInitialized.Emit();

    return true;
}

bool D3D12::InitializeDownlevel(ID3D12CommandQueue* apCommandQueue, ID3D12Resource* apSourceTex2D, HWND ahWindow)
{
    if (!apCommandQueue || !apSourceTex2D)
        return false;

    HWND hWnd = m_window.GetWindow();
    if (!hWnd)
    {
        Log::Warn("D3D12::InitializeDownlevel() - window not yet hooked!");
        return false;
    }

    if (m_initialized)
    {
        if (hWnd != ahWindow)
            Log::Warn("D3D12::InitializeDownlevel() - current output window does not match hooked window! Currently hooked to {0} while current output window is {1}.", reinterpret_cast<void*>(hWnd), reinterpret_cast<void*>(ahWindow));

        return true;
    }

    auto cmdQueueDesc = apCommandQueue->GetDesc();
    if(cmdQueueDesc.Type != D3D12_COMMAND_LIST_TYPE_DIRECT)
    {
        Log::Warn("D3D12::InitializeDownlevel() - ignoring command queue - invalid type of command list!");
        return false;
    }

    m_pCommandQueue = apCommandQueue;

    auto st2DDesc = apSourceTex2D->GetDesc();
    m_outSize = { static_cast<LONG>(st2DDesc.Width), static_cast<LONG>(st2DDesc.Height) };

    if (hWnd != ahWindow)
        Log::Warn("D3D12::InitializeDownlevel() - current output window does not match hooked window! Currently hooked to {0} while current output window is {1}.", reinterpret_cast<void*>(hWnd), reinterpret_cast<void*>(ahWindow));

    if (FAILED(apSourceTex2D->GetDevice(IID_PPV_ARGS(&m_pd3d12Device))))
    {
        Log::Error("D3D12::InitializeDownlevel() - failed to get device!");
        return ResetState();
    }

    const size_t buffersCounts = m_downlevelBackbuffers.size();
    m_frameContexts.resize(buffersCounts);
    if (buffersCounts == 0)
    {
        Log::Error("D3D12::InitializeDownlevel() - no backbuffers were found!");
        return ResetState();
    }
    if (buffersCounts < g_numDownlevelBackbuffersRequired)
    {
        Log::Info("D3D12::InitializeDownlevel() - backbuffer list is not complete yet; assuming window was resized");
        return false;
    }

    D3D12_DESCRIPTOR_HEAP_DESC rtvdesc;
    rtvdesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvdesc.NumDescriptors = static_cast<UINT>(buffersCounts);
    rtvdesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    rtvdesc.NodeMask = 1;
    if (FAILED(m_pd3d12Device->CreateDescriptorHeap(&rtvdesc, IID_PPV_ARGS(&m_pd3dRtvDescHeap))))
    {
        Log::Error("D3D12::InitializeDownlevel() - failed to create RTV descriptor heap!");
        return ResetState();
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
    srvdesc.NumDescriptors = 2;
    srvdesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    if (FAILED(m_pd3d12Device->CreateDescriptorHeap(&srvdesc, IID_PPV_ARGS(&m_pd3dSrvDescHeap))))
    {
        Log::Error("D3D12::InitializeDownlevel() - failed to create SRV descriptor heap!");
        return ResetState();
    }

    for (auto& context : m_frameContexts)
    {
        if (FAILED(m_pd3d12Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&context.CommandAllocator))))
        {
            Log::Error("D3D12::InitializeDownlevel() - failed to create command allocator!");
            return ResetState();
        }
    }

    if (FAILED(m_pd3d12Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_frameContexts[0].CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_pd3dCommandList))))
    {
        Log::Error("D3D12::InitializeDownlevel() - failed to create command list!");
        return ResetState();
    }

    if (FAILED(m_pd3dCommandList->Close()))
    {
        Log::Error("D3D12::InitializeDownlevel() - failed to close command list!");
        return ResetState();
    }

    for (size_t i = 0; i < buffersCounts; i++)
    {
        auto& context = m_frameContexts[i];
        context.BackBuffer = m_downlevelBackbuffers[i];
        m_pd3d12Device->CreateRenderTargetView(context.BackBuffer.Get(), nullptr, context.MainRenderTargetDescriptor);
    }

    if (!InitializeImGui(buffersCounts))
    {
        Log::Error("D3D12::InitializeDownlevel() - failed to initialize ImGui!");
        return ResetState();
    }

    Log::Info("D3D12::InitializeDownlevel() - initialization successful!");
    m_initialized = true;

    OnInitialized.Emit();

    return true;
}

bool D3D12::InitializeImGui(size_t aBuffersCounts)
{
    // TODO - scale also by DPI
    const auto [resx, resy] = GetResolution();
    const auto scaleFromReference = std::min(static_cast<float>(resx) / 1920.0f, static_cast<float>(resy) / 1080.0f);

    if (ImGui::GetCurrentContext() == nullptr)
    {
        // do this once, do not repeat context creation!
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGui::StyleColorsDark();
        m_styleReference = ImGui::GetStyle();
    }

    ImGui::GetStyle() = m_styleReference;
    ImGui::GetStyle().ScaleAllSizes(scaleFromReference);

    auto& io = ImGui::GetIO();
    io.Fonts->Clear();

    ImFontConfig config;
    config.SizePixels = std::floorf(m_options.FontSize * scaleFromReference);
    config.OversampleH = config.OversampleV = 2;
    config.PixelSnapH = true;
    config.MergeMode = false;

    // add default font
    const auto customFontPath = m_options.FontPath.empty() ? std::filesystem::path{} : GetAbsolutePath(UTF8ToUTF16(m_options.FontPath), m_paths.Fonts(), false);
    auto cetFontPath = GetAbsolutePath(L"NotoSans-Regular.ttf", m_paths.Fonts(), false);
    const auto* cpGlyphRanges = io.Fonts->GetGlyphRangesDefault();
    if (customFontPath.empty())
    {
        if (!m_options.FontPath.empty())
            Log::Warn("D3D12::InitializeImGui() - Custom font path is invalid! Using default CET font.");

        if (cetFontPath.empty())
        {
            Log::Warn("D3D12::InitializeImGui() - Missing default fonts!");
            io.Fonts->AddFontDefault(&config);
        }
        else
            io.Fonts->AddFontFromFileTTF(UTF16ToUTF8(cetFontPath.native()).c_str(), config.SizePixels, &config, cpGlyphRanges);
    }
    else
        io.Fonts->AddFontFromFileTTF(UTF16ToUTF8(customFontPath.native()).c_str(), config.SizePixels, &config, cpGlyphRanges);

    if (m_options.FontGlyphRanges == "ChineseFull")
    {
        cetFontPath = GetAbsolutePath(m_paths.Fonts() / L"NotoSansTC-Regular.otf", m_paths.Fonts(), false);
        cpGlyphRanges = io.Fonts->GetGlyphRangesChineseFull();
    }
    else if (m_options.FontGlyphRanges == "ChineseSimplifiedCommon")
    {
        cetFontPath = GetAbsolutePath(m_paths.Fonts() / L"NotoSansSC-Regular.otf", m_paths.Fonts(), false);
        cpGlyphRanges = io.Fonts->GetGlyphRangesChineseSimplifiedCommon();
    }
    else if (m_options.FontGlyphRanges == "Japanese")
    {
        cetFontPath = GetAbsolutePath(m_paths.Fonts() / L"NotoSansJP-Regular.otf", m_paths.Fonts(), false);
        cpGlyphRanges = io.Fonts->GetGlyphRangesJapanese();
    }
    else if (m_options.FontGlyphRanges == "Korean")
    {
        cetFontPath = GetAbsolutePath(m_paths.Fonts() / L"NotoSansKR-Regular.otf", m_paths.Fonts(), false);
        cpGlyphRanges = io.Fonts->GetGlyphRangesKorean();
    }
    else if (m_options.FontGlyphRanges == "Cyrillic")
    {
        cetFontPath = GetAbsolutePath(m_paths.Fonts() / L"NotoSans-Regular.ttf", m_paths.Fonts(), false);
        cpGlyphRanges = io.Fonts->GetGlyphRangesCyrillic();
    }
    else if (m_options.FontGlyphRanges == "Thai")
    {
        cetFontPath = GetAbsolutePath(m_paths.Fonts() / L"NotoSansThai-Regular.ttf", m_paths.Fonts(), false);
        cpGlyphRanges = io.Fonts->GetGlyphRangesThai();
    }
    else if (m_options.FontGlyphRanges == "Vietnamese")
    {
        cetFontPath = GetAbsolutePath(m_paths.Fonts() / L"NotoSans-Regular.ttf", m_paths.Fonts(), false);
        cpGlyphRanges = io.Fonts->GetGlyphRangesVietnamese();
    }
    else
    {
        switch (GetSystemDefaultLangID())
        {
        case MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_TRADITIONAL):
            cetFontPath = GetAbsolutePath(L"NotoSansTC-Regular.otf", m_paths.Fonts(), false);
            cpGlyphRanges = io.Fonts->GetGlyphRangesChineseFull();
            break;

        case MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED):
            cetFontPath = GetAbsolutePath(m_paths.Fonts() / L"NotoSansSC-Regular.otf", m_paths.Fonts(), false);
            cpGlyphRanges = io.Fonts->GetGlyphRangesChineseSimplifiedCommon();
            break;

        case MAKELANGID(LANG_JAPANESE, SUBLANG_DEFAULT):
            cetFontPath = GetAbsolutePath(m_paths.Fonts() / L"NotoSansJP-Regular.otf", m_paths.Fonts(), false);
            cpGlyphRanges = io.Fonts->GetGlyphRangesJapanese();
            break;

        case MAKELANGID(LANG_KOREAN, SUBLANG_DEFAULT):
            cetFontPath = GetAbsolutePath(m_paths.Fonts() / L"NotoSansKR-Regular.otf", m_paths.Fonts(), false);
            cpGlyphRanges = io.Fonts->GetGlyphRangesKorean();
            break;

        case MAKELANGID(LANG_BELARUSIAN, SUBLANG_DEFAULT):
        case MAKELANGID(LANG_RUSSIAN, SUBLANG_DEFAULT):
            cetFontPath = GetAbsolutePath(m_paths.Fonts() / L"NotoSans-Regular.ttf", m_paths.Fonts(), false);
            cpGlyphRanges = io.Fonts->GetGlyphRangesCyrillic();
            break;

        case MAKELANGID(LANG_THAI, SUBLANG_DEFAULT):
            cetFontPath = GetAbsolutePath(m_paths.Fonts() / L"NotoSansThai-Regular.ttf", m_paths.Fonts(), false);
            cpGlyphRanges = io.Fonts->GetGlyphRangesThai();
            break;

        case MAKELANGID(LANG_VIETNAMESE, SUBLANG_DEFAULT):
            cetFontPath = GetAbsolutePath(m_paths.Fonts() / L"NotoSans-Regular.ttf", m_paths.Fonts(), false);
            cpGlyphRanges = io.Fonts->GetGlyphRangesVietnamese();
            break;
        }
    }

    // add extra glyphs from language font
    config.MergeMode = true;
    if (customFontPath.empty())
    {
        if (!m_options.FontPath.empty())
            Log::Warn("D3D12::InitializeImGui() - Custom font path is invalid! Using default CET font.");

        if (cetFontPath.empty())
        {
            Log::Warn("D3D12::InitializeImGui() - Missing fonts for extra language glyphs!");
            io.Fonts->AddFontDefault(&config);
        }
        else
            io.Fonts->AddFontFromFileTTF(UTF16ToUTF8(cetFontPath.native()).c_str(), config.SizePixels, &config, cpGlyphRanges);
    }
    else
        io.Fonts->AddFontFromFileTTF(UTF16ToUTF8(customFontPath.native()).c_str(), config.SizePixels, &config, cpGlyphRanges);

    if (!ImGui_ImplWin32_Init(m_window.GetWindow()))
    {
        Log::Error("D3D12::InitializeImGui() - ImGui_ImplWin32_Init call failed!");
        return false;
    }

    if (!ImGui_ImplDX12_Init(m_pd3d12Device.Get(), static_cast<int>(aBuffersCounts),
        DXGI_FORMAT_R8G8B8A8_UNORM, m_pd3dSrvDescHeap.Get(),
        m_pd3dSrvDescHeap->GetCPUDescriptorHandleForHeapStart(),
        m_pd3dSrvDescHeap->GetGPUDescriptorHandleForHeapStart()))
    {
        Log::Error("D3D12::InitializeImGui() - ImGui_ImplDX12_Init call failed!");
        ImGui_ImplWin32_Shutdown();
        return false;
    }

    if (!ImGui_ImplDX12_CreateDeviceObjects(m_pCommandQueue))
    {
        Log::Error("D3D12::InitializeImGui() - ImGui_ImplDX12_CreateDeviceObjects call failed!");
        ImGui_ImplDX12_Shutdown();
        ImGui_ImplWin32_Shutdown();
        return false;
    }

    return true;
}

void D3D12::Update()
{
    ImGui_ImplDX12_NewFrame(m_pCommandQueue);
    ImGui_ImplWin32_NewFrame(m_outSize);
    ImGui::NewFrame();

    OnUpdate.Emit();

    const auto bufferIndex = (m_pdxgiSwapChain != nullptr) ? (m_pdxgiSwapChain->GetCurrentBackBufferIndex()) : (m_downlevelBufferIndex);
    auto& frameContext = m_frameContexts[bufferIndex];
    frameContext.CommandAllocator->Reset();

    m_pd3dCommandList->Reset(frameContext.CommandAllocator.Get(), nullptr);

    D3D12_RESOURCE_BARRIER barrier;
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = frameContext.BackBuffer.Get();
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    m_pd3dCommandList->ResourceBarrier(1, &barrier);

    ID3D12DescriptorHeap* heaps[] = { m_pd3dSrvDescHeap.Get() };
    m_pd3dCommandList->SetDescriptorHeaps(1, heaps);

    m_pd3dCommandList->OMSetRenderTargets(1, &frameContext.MainRenderTargetDescriptor, FALSE, nullptr);

    ImGui::Render();
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_pd3dCommandList.Get());

    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    m_pd3dCommandList->ResourceBarrier(1, &barrier);

    m_pd3dCommandList->Close();

    ID3D12CommandList* commandLists[] = { m_pd3dCommandList.Get() };
    m_pCommandQueue->ExecuteCommandLists(1, commandLists);
}

