#include <stdafx.h>

#include "D3D12.h"

#include <CET.h>
#include <imgui/imgui_impl_dx12.h>
#include <imgui/imgui_impl_win32.h>
#include <Utils.h>
#include <window/window.h>

void D3D12::Shutdown()
{
    std::lock_guard statePresentLock(m_statePresentMutex);
    std::lock_guard stateGameLock(m_stateGameMutex);
    std::lock_guard imguiLock(m_imguiMutex);

    m_shutdown = true;

    m_window.Hook(nullptr);

    if (ImGui::GetCurrentContext())
    {
        for (auto& drawData : m_imguiDrawDataBuffers)
        {
            for (auto i = 0; i < drawData.CmdListsCount; ++i)
                IM_DELETE(drawData.CmdLists[i]);
            delete[] drawData.CmdLists;
            drawData.Clear();
        }

        ImGui_ImplDX12_Shutdown();
        ImGui_ImplWin32_Shutdown();

        ImGui::DestroyContext();
    }

    for (auto& frameContext : m_frameContexts)
    {
        frameContext.CommandAllocator.Reset();
        frameContext.CommandList.Reset();
    }

    m_pd3dSrvDescHeap.Reset();

    m_swapChainDataId = 0;
    m_resolution = ImVec2();

    m_initialized = false;
}

void D3D12::Initialize(uint32_t aSwapChainDataId)
{
    std::lock_guard statePresentLock(m_statePresentMutex);
    std::lock_guard stateGameLock(m_stateGameMutex);
    std::lock_guard imguiLock(m_imguiMutex);

    assert(!m_initialized && !m_shutdown);
    if (m_initialized || m_shutdown)
        return;

    // we need valid swap chain data ID (this must be set even on Win7)
    if (aSwapChainDataId == 0)
        return;

    // we need valid render context
    const auto* cpRenderContext = RenderContext::GetInstance();
    if (cpRenderContext == nullptr)
        return;

    // we need to have valid device
    const auto cpDevice = cpRenderContext->pDevice;
    if (cpDevice == nullptr)
        return;

    // if any back buffer is nullptr, don't continue initialization
    auto swapChainData = cpRenderContext->pSwapChainData[aSwapChainDataId - 1];
    for (auto& pBackBuffer : swapChainData.backBuffers)
    {
        if (pBackBuffer == nullptr)
            return;
    }

    // we need valid window
    const auto cpWindow = swapChainData.pWindow;
    if (cpWindow == nullptr)
        return;

    m_swapChainDataId = aSwapChainDataId;

    const auto cBackBufferDesc = swapChainData.backBuffers[0]->GetDesc();
    m_resolution = ImVec2(static_cast<float>(cBackBufferDesc.Width), static_cast<float>(cBackBufferDesc.Height));

    D3D12_DESCRIPTOR_HEAP_DESC srvdesc = {};
    srvdesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvdesc.NumDescriptors = SwapChainData_BackBufferCount;
    srvdesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    if (FAILED(cpDevice->CreateDescriptorHeap(&srvdesc, IID_PPV_ARGS(&m_pd3dSrvDescHeap))))
    {
        Log::Error("D3D12::Initialize() - failed to create SRV descriptor heap!");
        return Shutdown();
    }

    for (auto& frameContext : m_frameContexts)
    {
        if (FAILED(cpDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&frameContext.CommandAllocator))))
        {
            Log::Error("D3D12::Initialize() - failed to create command allocator!");
            return Shutdown();
        }

        if (FAILED(cpDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, frameContext.CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&frameContext.CommandList))) ||
            FAILED(frameContext.CommandList->Close()))
        {
            Log::Error("D3D12::Initialize() - failed to create command list!");
            return Shutdown();
        }
    }

    m_window.Hook(cpWindow);

    if (!InitializeImGui())
    {
        Log::Error("D3D12::Initialize() - failed to initialize ImGui!");
        return Shutdown();
    }

    Log::Info("D3D12::Initialize() - initialization successful!");
    m_initialized = true;
}

void D3D12::ReloadFonts(bool aForce)
{
    std::lock_guard statePresentLock(m_statePresentMutex);
    std::lock_guard stateGameLock(m_stateGameMutex);
    std::lock_guard imguiLock(m_imguiMutex);

    const auto& cFontSettings = m_options.Font;

    if (!aForce && cFontSettings == m_fontSettings)
        return;

    const auto cDPIScale = ImGui_ImplWin32_GetDpiScaleForHwnd(m_window.GetWindow());
    const auto cResolution = GetResolution();
    const auto cResolutionScaleFromReference = std::min(cResolution.x / 1920.0f, cResolution.y / 1080.0f);

    const auto cFontSize = std::floorf(cFontSettings.BaseSize * cDPIScale * cResolutionScaleFromReference);
    const auto cFontScaleFromReference = cFontSize / 18.0f;

    ImGui::GetStyle() = m_imguiStyleReference;
    ImGui::GetStyle().ScaleAllSizes(cFontScaleFromReference);

    ImGui_ImplDX12_InvalidateDeviceObjects();

    auto& io = ImGui::GetIO();
    io.Fonts->Clear();

    ImFontConfig config;
    config.SizePixels = cFontSize;
    config.OversampleH = cFontSettings.OversampleHorizontal;
    config.OversampleV = cFontSettings.OversampleVertical;
    if (config.OversampleH == 1 && config.OversampleV == 1)
        config.PixelSnapH = true;
    config.MergeMode = false;

    // add default font
    const auto cCustomFontPath = cFontSettings.Path.empty() ? std::filesystem::path{} : GetAbsolutePath(UTF8ToUTF16(cFontSettings.Path), m_paths.Fonts(), false);
    auto cetFontPath = GetAbsolutePath(L"NotoSans-Regular.ttf", m_paths.Fonts(), false);
    const auto* cpGlyphRanges = io.Fonts->GetGlyphRangesDefault();
    if (cCustomFontPath.empty())
    {
        if (!cFontSettings.Path.empty())
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
        io.Fonts->AddFontFromFileTTF(UTF16ToUTF8(cCustomFontPath.native()).c_str(), config.SizePixels, &config, cpGlyphRanges);

    if (cFontSettings.Language == "ChineseFull")
    {
        cetFontPath = GetAbsolutePath(m_paths.Fonts() / L"NotoSansTC-Regular.otf", m_paths.Fonts(), false);
        cpGlyphRanges = io.Fonts->GetGlyphRangesChineseFull();
    }
    else if (cFontSettings.Language == "ChineseSimplifiedCommon")
    {
        cetFontPath = GetAbsolutePath(m_paths.Fonts() / L"NotoSansSC-Regular.otf", m_paths.Fonts(), false);
        cpGlyphRanges = io.Fonts->GetGlyphRangesChineseSimplifiedCommon();
    }
    else if (cFontSettings.Language == "Japanese")
    {
        cetFontPath = GetAbsolutePath(m_paths.Fonts() / L"NotoSansJP-Regular.otf", m_paths.Fonts(), false);
        cpGlyphRanges = io.Fonts->GetGlyphRangesJapanese();
    }
    else if (cFontSettings.Language == "Korean")
    {
        cetFontPath = GetAbsolutePath(m_paths.Fonts() / L"NotoSansKR-Regular.otf", m_paths.Fonts(), false);
        cpGlyphRanges = io.Fonts->GetGlyphRangesKorean();
    }
    else if (cFontSettings.Language == "Cyrillic")
    {
        cetFontPath = GetAbsolutePath(m_paths.Fonts() / L"NotoSans-Regular.ttf", m_paths.Fonts(), false);
        cpGlyphRanges = io.Fonts->GetGlyphRangesCyrillic();
    }
    else if (cFontSettings.Language == "Thai")
    {
        cetFontPath = GetAbsolutePath(m_paths.Fonts() / L"NotoSansThai-Regular.ttf", m_paths.Fonts(), false);
        cpGlyphRanges = io.Fonts->GetGlyphRangesThai();
    }
    else if (cFontSettings.Language == "Vietnamese")
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
    if (cCustomFontPath.empty())
    {
        if (!cFontSettings.Path.empty())
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
        io.Fonts->AddFontFromFileTTF(UTF16ToUTF8(cCustomFontPath.native()).c_str(), config.SizePixels, &config, cpGlyphRanges);

    m_fontSettings = cFontSettings;

    if (!ImGui_ImplDX12_CreateDeviceObjects())
        Log::Error("D3D12::InitializeImGui() - ImGui_ImplDX12_CreateDeviceObjects call failed!");
}

bool D3D12::InitializeImGui()
{
    assert(!m_initialized);

    // we need valid swap chain data ID (this must be set even on Win7)
    if (m_swapChainDataId == 0)
        return false;

    // we need valid render context
    const auto* cpRenderContext = RenderContext::GetInstance();
    if (cpRenderContext == nullptr)
        return false;

    // we need to have valid device
    const auto cpDevice = cpRenderContext->pDevice;
    if (cpDevice == nullptr)
        return false;

    // we need to have valid command queue
    const auto cpCommandQueue = cpRenderContext->pDirectCommandQueue;
    if (cpCommandQueue == nullptr)
        return false;

    // do this once, do not repeat context creation!
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    auto& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
    //io.ConfigViewportsNoAutoMerge = true;
    //io.ConfigViewportsNoTaskBarIcon = true;

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

    io.DisplaySize = GetResolution();

	  if (!ImGui_ImplWin32_Init(m_window.GetWindow()))
	  {
	      Log::Error("D3D12::InitializeImGui() - ImGui_ImplWin32_Init call failed!");
	      return false;
	  }

	  if (!ImGui_ImplDX12_Init(cpDevice.Get(), cpCommandQueue->GetDesc().NodeMask,
          static_cast<uint32_t>(SwapChainData_BackBufferCount),
	      DXGI_FORMAT_R8G8B8A8_UNORM, m_pd3dSrvDescHeap.Get(),
	      m_pd3dSrvDescHeap->GetCPUDescriptorHandleForHeapStart(),
	      m_pd3dSrvDescHeap->GetGPUDescriptorHandleForHeapStart()))
	  {
	      Log::Error("D3D12::InitializeImGui() - ImGui_ImplDX12_Init call failed!");
	      ImGui_ImplWin32_Shutdown();
	      return false;
	  }

    ReloadFonts(true);

    return true;
}

void D3D12::PrepareUpdate()
{
    std::lock_guard stateGameLock(m_stateGameMutex);

    if (!m_initialized || m_shutdown)
        return;

    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGui::PushStyleColor(ImGuiCol_WindowBg, {0.0f, 0.0f, 0.0f, 0.0f});
    ImGui::PushStyleColor(ImGuiCol_DockingEmptyBg, {0.0f, 0.0f, 0.0f, 0.0f});
    ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
    ImGui::PopStyleColor(2);

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

    std::lock_guard imguiLock(m_imguiMutex);

    std::swap(m_imguiDrawDataBuffers[1], m_imguiDrawDataBuffers[2]);
}

void D3D12::Update(uint32_t aSwapChainDataId)
{
    std::lock_guard statePresentLock(m_statePresentMutex);

    if (!m_initialized || m_shutdown)
        return;

    if (m_swapChainDataId != aSwapChainDataId)
        return;

    // we need valid swap chain data ID (this must be set even on Win7)
    if (aSwapChainDataId == 0)
        return;

    // we need valid render context
    const auto* cpRenderContext = RenderContext::GetInstance();
    if (cpRenderContext == nullptr)
        return;

    // if any back buffer is nullptr, don't continue update
    auto swapChainData = cpRenderContext->pSwapChainData[aSwapChainDataId - 1];
    for (auto& pBackBuffer : swapChainData.backBuffers)
    {
        if (pBackBuffer == nullptr)
            return;
    }

    // we need to have valid command queue
    const auto cpCommandQueue = cpRenderContext->pDirectCommandQueue;
    if (cpCommandQueue == nullptr)
        return;

    const auto cBackBufferIndex = swapChainData.backBufferIndex;
    const auto cpBackBuffer = swapChainData.backBuffers[cBackBufferIndex];
    const auto cpRenderTargetView = swapChainData.renderTargetViews[cBackBufferIndex];

    const auto& cFrameContext = m_frameContexts[cBackBufferIndex];

    // swap staging ImGui buffer with render ImGui buffer
    {
        std::lock_guard imguiLock(m_imguiMutex);

        if (m_imguiDrawDataBuffers[1].Valid)
        {
            std::swap(m_imguiDrawDataBuffers[0], m_imguiDrawDataBuffers[1]);
            m_imguiDrawDataBuffers[1].Valid = false;
        }
    }

    if (!m_imguiDrawDataBuffers[0].Valid)
        return;

    const auto cResolution = GetResolution();
    const auto cDrawDataResolution = m_imguiDrawDataBuffers[0].DisplaySize;
    if (cDrawDataResolution.x != cResolution.x || cDrawDataResolution.y != cResolution.y)
        return;

    cFrameContext.CommandAllocator->Reset();

    D3D12_RESOURCE_BARRIER barrier;
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = cpBackBuffer.Get();
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

    ID3D12DescriptorHeap* heaps[] = { m_pd3dSrvDescHeap.Get() };

    cFrameContext.CommandList->Reset(cFrameContext.CommandAllocator.Get(), nullptr);
    cFrameContext.CommandList->ResourceBarrier(1, &barrier);
    cFrameContext.CommandList->SetDescriptorHeaps(1, heaps);
    cFrameContext.CommandList->OMSetRenderTargets(1, &cpRenderTargetView, FALSE, nullptr);

    ImGui_ImplDX12_NewFrame();
    ImGui_ImplDX12_RenderDrawData(&m_imguiDrawDataBuffers[0], cFrameContext.CommandList.Get());

    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    cFrameContext.CommandList->ResourceBarrier(1, &barrier);
    cFrameContext.CommandList->Close();

    ID3D12CommandList* commandLists[] = { cFrameContext.CommandList.Get() };
    cpCommandQueue->ExecuteCommandLists(1, commandLists);
}
