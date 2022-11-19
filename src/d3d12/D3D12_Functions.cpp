#include <stdafx.h>

#include "D3D12.h"
#include "Options.h"
#include "Utils.h"

#include <CET.h>
#include <imgui_impl/dx12.h>
#include <imgui_impl/win32.h>
#include <window/window.h>

bool D3D12::ResetState(const bool acDestroyContext)
{
    std::lock_guard _(m_imguiLock);

    if (acDestroyContext && m_window.GetWindow())
        m_window.Hook(nullptr);

    if (m_initialized)
    {
        for (auto& drawData : m_imguiDrawDataBuffers)
        {
            for (auto i = 0; i < drawData.CmdListsCount; ++i)
                IM_DELETE(drawData.CmdLists[i]);
            delete[] drawData.CmdLists;
            drawData.CmdLists = nullptr;
            drawData.Clear();
        }

        ImGui_ImplDX12_Shutdown();
        ImGui_ImplWin32_Shutdown();
    }

    if (acDestroyContext && ImGui::GetCurrentContext())
        ImGui::DestroyContext();

    for (auto& frameContext : m_frameContexts)
    {
        frameContext.CommandAllocator.Reset();
        frameContext.CommandList.Reset();
    }

    m_resolution = ImVec2();

    m_pd3dSrvDescHeap.Reset();

    m_initialized = false;

    return false;
}

bool D3D12::Initialize(uint32_t aSwapChainDataId)
{
    if (m_initialized)
        return true;

    m_swapChainDataId = aSwapChainDataId;

    // we need valid swap chain data ID (this must be set even on Win7)
    if (m_swapChainDataId == 0)
        return false;

    // we need valid render context
    auto* pRenderContext = RenderContext::GetInstance();
    if (pRenderContext == nullptr)
        return false;

    // we need to have valid device
    auto pDevice = pRenderContext->pDevice;
    if (pDevice == nullptr)
        return false;

    // if any back buffer is nullptr, don't continue initialization
    auto swapChainData = pRenderContext->pSwapChainData[m_swapChainDataId - 1];
    for (auto& pBackBuffer : swapChainData.backBuffers)
    {
        if (pBackBuffer == nullptr)
            return false;
    }

    auto backBufferDesc = swapChainData.backBuffers[0]->GetDesc();
    m_resolution = ImVec2(static_cast<float>(backBufferDesc.Width), static_cast<float>(backBufferDesc.Height));

    // we need valid window
    auto pWindow = swapChainData.pWindow;
    if (pWindow == nullptr)
        return false;

    D3D12_DESCRIPTOR_HEAP_DESC srvdesc = {};
    srvdesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvdesc.NumDescriptors = SwapChainData_BackBufferCount;
    srvdesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    if (FAILED(pDevice->CreateDescriptorHeap(&srvdesc, IID_PPV_ARGS(&m_pd3dSrvDescHeap))))
    {
        Log::Error("D3D12::Initialize() - failed to create SRV descriptor heap!");
        return ResetState();
    }

    for (auto& frameContext : m_frameContexts)
    {
        if (FAILED(pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&frameContext.CommandAllocator))))
        {
            Log::Error("D3D12::Initialize() - failed to create command allocator!");
            return ResetState();
        }

        if (FAILED(pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, frameContext.CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&frameContext.CommandList))) ||
            FAILED(frameContext.CommandList->Close()))
        {
            Log::Error("D3D12::Initialize() - failed to create command list!");
            return ResetState();
        }
    }

    m_window.Hook(pWindow);

    if (!InitializeImGui())
    {
        Log::Error("D3D12::Initialize() - failed to initialize ImGui!");
        return ResetState(true);
    }

    OnInitialize.Emit();

    Log::Info("D3D12::Initialize() - initialization successful!");
    m_initialized = true;

    return true;
}

void D3D12::ReloadFonts()
{
    std::lock_guard _(m_imguiLock);

    const auto& fontSettings = m_options.Font;

    const auto dpiScale = 1.0f; // TODO - will be replaced in another PR
    const auto resolution = GetResolution();
    const auto resolutionScaleFromReference = std::min(resolution.x / 1920.0f, resolution.y / 1080.0f);

    const auto fontSize = std::floorf(fontSettings.BaseSize * dpiScale * resolutionScaleFromReference);
    const auto fontScaleFromReference = fontSize / 18.0f;

    ImGui::GetStyle() = m_imguiStyleReference;
    ImGui::GetStyle().ScaleAllSizes(fontScaleFromReference);

    auto& io = ImGui::GetIO();
    io.Fonts->Clear();

    ImFontConfig config;
    config.SizePixels = fontSize;
    config.OversampleH = fontSettings.OversampleHorizontal;
    config.OversampleV = fontSettings.OversampleVertical;
    if (config.OversampleH == 1 && config.OversampleV == 1)
        config.PixelSnapH = true;
    config.MergeMode = false;

    // add default font
    const auto customFontPath = fontSettings.Path.empty() ? std::filesystem::path{} : GetAbsolutePath(UTF8ToUTF16(fontSettings.Path), m_paths.Fonts(), false);
    auto cetFontPath = GetAbsolutePath(L"NotoSans-Regular.ttf", m_paths.Fonts(), false);
    const auto* cpGlyphRanges = io.Fonts->GetGlyphRangesDefault();
    if (customFontPath.empty())
    {
        if (!fontSettings.Path.empty())
            Log::Warn("D3D12::ReloadFonts() - Custom font path is invalid! Using default CET font.");

        if (cetFontPath.empty())
        {
            Log::Warn("D3D12::ReloadFonts() - Missing default fonts!");
            io.Fonts->AddFontDefault(&config);
        }
        else
            io.Fonts->AddFontFromFileTTF(UTF16ToUTF8(cetFontPath.native()).c_str(), config.SizePixels, &config, cpGlyphRanges);
    }
    else
        io.Fonts->AddFontFromFileTTF(UTF16ToUTF8(customFontPath.native()).c_str(), config.SizePixels, &config, cpGlyphRanges);

    if (fontSettings.Language == "ChineseFull")
    {
        cetFontPath = GetAbsolutePath(m_paths.Fonts() / L"NotoSansTC-Regular.otf", m_paths.Fonts(), false);
        cpGlyphRanges = io.Fonts->GetGlyphRangesChineseFull();
    }
    else if (fontSettings.Language == "ChineseSimplifiedCommon")
    {
        cetFontPath = GetAbsolutePath(m_paths.Fonts() / L"NotoSansSC-Regular.otf", m_paths.Fonts(), false);
        cpGlyphRanges = io.Fonts->GetGlyphRangesChineseSimplifiedCommon();
    }
    else if (fontSettings.Language == "Japanese")
    {
        cetFontPath = GetAbsolutePath(m_paths.Fonts() / L"NotoSansJP-Regular.otf", m_paths.Fonts(), false);
        cpGlyphRanges = io.Fonts->GetGlyphRangesJapanese();
    }
    else if (fontSettings.Language == "Korean")
    {
        cetFontPath = GetAbsolutePath(m_paths.Fonts() / L"NotoSansKR-Regular.otf", m_paths.Fonts(), false);
        cpGlyphRanges = io.Fonts->GetGlyphRangesKorean();
    }
    else if (fontSettings.Language == "Cyrillic")
    {
        cetFontPath = GetAbsolutePath(m_paths.Fonts() / L"NotoSans-Regular.ttf", m_paths.Fonts(), false);
        cpGlyphRanges = io.Fonts->GetGlyphRangesCyrillic();
    }
    else if (fontSettings.Language == "Thai")
    {
        cetFontPath = GetAbsolutePath(m_paths.Fonts() / L"NotoSansThai-Regular.ttf", m_paths.Fonts(), false);
        cpGlyphRanges = io.Fonts->GetGlyphRangesThai();
    }
    else if (fontSettings.Language == "Vietnamese")
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
        if (!fontSettings.Path.empty())
            Log::Warn("D3D12::ReloadFonts() - Custom font path is invalid! Using default CET font.");

        if (cetFontPath.empty())
        {
            Log::Warn("D3D12::ReloadFonts() - Missing fonts for extra language glyphs!");
            io.Fonts->AddFontDefault(&config);
        }
        else
            io.Fonts->AddFontFromFileTTF(UTF16ToUTF8(cetFontPath.native()).c_str(), config.SizePixels, &config, cpGlyphRanges);
    }
    else
        io.Fonts->AddFontFromFileTTF(UTF16ToUTF8(customFontPath.native()).c_str(), config.SizePixels, &config, cpGlyphRanges);
}

bool D3D12::InitializeImGui()
{
    // we need valid swap chain data ID (this must be set even on Win7)
    if (m_swapChainDataId == 0)
        return false;

    // we need valid render context
    auto* pRenderContext = RenderContext::GetInstance();
    if (pRenderContext == nullptr)
        return false;

    // we need to have valid device
    auto pDevice = pRenderContext->pDevice;
    if (pDevice == nullptr)
        return false;

    // we need to have valid command queue
    auto pCommandQueue = pRenderContext->pDirectCommandQueue;
    if (pCommandQueue == nullptr)
        return false;

    std::lock_guard _(m_imguiLock);

    if (ImGui::GetCurrentContext() == nullptr)
    {
        // do this once, do not repeat context creation!
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        // TODO - make this configurable eventually and overridable by mods for themselves easily
        // setup CET default style
        ImGui::StyleColorsDark(&m_imguiStyleReference);
        m_imguiStyleReference.WindowRounding = 6.0f;
        m_imguiStyleReference.WindowTitleAlign.x = 0.5f;
        m_imguiStyleReference.ChildRounding = 6.0f;
        m_imguiStyleReference.PopupRounding = 6.0f;
        m_imguiStyleReference.FrameRounding = 6.0f;
        m_imguiStyleReference.ScrollbarRounding = 12.0f;
        m_imguiStyleReference.GrabRounding = 12.0f;
        m_imguiStyleReference.TabRounding = 6.0f;
    }

    if (!ImGui_ImplWin32_Init(m_window.GetWindow()))
    {
        Log::Error("D3D12::InitializeImGui() - ImGui_ImplWin32_Init call failed!");
        return false;
    }

    if (!ImGui_ImplDX12_Init(pDevice.Get(), SwapChainData_BackBufferCount,
        DXGI_FORMAT_R8G8B8A8_UNORM, m_pd3dSrvDescHeap.Get(),
        m_pd3dSrvDescHeap->GetCPUDescriptorHandleForHeapStart(),
        m_pd3dSrvDescHeap->GetGPUDescriptorHandleForHeapStart()))
    {
        Log::Error("D3D12::InitializeImGui() - ImGui_ImplDX12_Init call failed!");
        ImGui_ImplWin32_Shutdown();
        return false;
    }

    ReloadFonts();

    if (!ImGui_ImplDX12_CreateDeviceObjects(pCommandQueue.Get()))
    {
        Log::Error("D3D12::InitializeImGui() - ImGui_ImplDX12_CreateDeviceObjects call failed!");
        ImGui_ImplDX12_Shutdown();
        ImGui_ImplWin32_Shutdown();
        return false;
    }

    return true;
}

void D3D12::PrepareUpdate()
{
    if (!m_initialized)
        return;

    std::lock_guard _(m_imguiLock);

    const auto resolution = GetResolution();
    ImGui_ImplWin32_NewFrame({
        static_cast<LONG>(resolution.x),
        static_cast<LONG>(resolution.y)
    });
    ImGui::NewFrame();

    CET::Get().GetOverlay().Update();

    CET::Get().GetVM().Draw();

    ImGui::Render();

    auto& drawData = m_imguiDrawDataBuffers[2];

    for (auto i = 0; i < drawData.CmdListsCount; ++i)
        IM_DELETE(drawData.CmdLists[i]);
    delete[] drawData.CmdLists;
    drawData.CmdLists = nullptr;
    drawData.Clear();

    drawData = *ImGui::GetDrawData();

    auto** copiedDrawLists = new ImDrawList*[drawData.CmdListsCount];
    for (auto i = 0; i < drawData.CmdListsCount; ++i)
        copiedDrawLists[i] = drawData.CmdLists[i]->CloneOutput();
    drawData.CmdLists = copiedDrawLists;

    std::swap(m_imguiDrawDataBuffers[1], m_imguiDrawDataBuffers[2]);
}

void D3D12::Update()
{
    if (!m_initialized)
        return;

    // we need valid swap chain data ID (this must be set even on Win7)
    if (m_swapChainDataId == 0)
        return;

    // we need valid render context
    auto* pRenderContext = RenderContext::GetInstance();
    if (pRenderContext == nullptr)
        return;

    // if any back buffer is nullptr, don't continue initialization
    auto swapChainData = pRenderContext->pSwapChainData[m_swapChainDataId - 1];
    for (auto& pBackBuffer : swapChainData.backBuffers)
    {
        if (pBackBuffer == nullptr)
            return;
    }

    // we need to have valid command queue
    auto pCommandQueue = pRenderContext->pDirectCommandQueue;
    if (pCommandQueue == nullptr)
        return;

    const auto backBufferIndex = swapChainData.backBufferIndex;
    auto pBackBuffer = swapChainData.backBuffers[backBufferIndex];
    auto pRenderTargetView = swapChainData.renderTargetViews[backBufferIndex];

    auto& frameContext = m_frameContexts[backBufferIndex];

    // swap staging ImGui buffer with render ImGui buffer
    {
        std::lock_guard _(m_imguiLock);

        ImGui_ImplDX12_NewFrame(pCommandQueue.Get());

        if (m_imguiDrawDataBuffers[1].Valid)
        {
            std::swap(m_imguiDrawDataBuffers[0], m_imguiDrawDataBuffers[1]);
            m_imguiDrawDataBuffers[1].Valid = false;
        }
    }

    if (!m_imguiDrawDataBuffers[0].Valid)
        return;

    const auto resolution = GetResolution();
    const auto drawDataResolution = m_imguiDrawDataBuffers[0].DisplaySize;
    if (drawDataResolution.x != resolution.x || drawDataResolution.y != resolution.y)
        return;

    frameContext.CommandAllocator->Reset();

    D3D12_RESOURCE_BARRIER barrier;
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = pBackBuffer.Get();
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

    ID3D12DescriptorHeap* heaps[] = { m_pd3dSrvDescHeap.Get() };

    frameContext.CommandList->Reset(frameContext.CommandAllocator.Get(), nullptr);
    frameContext.CommandList->ResourceBarrier(1, &barrier);
    frameContext.CommandList->SetDescriptorHeaps(1, heaps);
    frameContext.CommandList->OMSetRenderTargets(1, &pRenderTargetView, FALSE, nullptr);

    ImGui_ImplDX12_RenderDrawData(&m_imguiDrawDataBuffers[0], frameContext.CommandList.Get());

    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    frameContext.CommandList->ResourceBarrier(1, &barrier);
    frameContext.CommandList->Close();

    ID3D12CommandList* commandLists[] = { frameContext.CommandList.Get() };
    pCommandQueue->ExecuteCommandLists(1, commandLists);
}

