// CET macOS: the Metal renderer. See metal/MetalRenderer.h for the design.

#import <AppKit/AppKit.h>
#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>
#import <objc/message.h>

#pragma push_macro("nil")
#undef nil
#define BOOL WINBOOL // the Win32 BOOL of the shared headers, not Objective-C's
#include <stdafx.h>

#include "MetalRenderer.h"

#include <CET.h>
#include <MacShims.hpp>
#include <Utils.h>
#undef BOOL
#pragma pop_macro("nil")

#include <imgui_impl_metal.h>

#include <pthread.h>

namespace
{
void RunOnMainSync(void (^aBlock)())
{
    if (pthread_main_np())
        aBlock();
    else
        dispatch_sync(dispatch_get_main_queue(), aBlock);
}

void RunOnMainAsync(void (^aBlock)())
{
    if (pthread_main_np())
        aBlock();
    else
        dispatch_async(dispatch_get_main_queue(), aBlock);
}

// The Windows build shows the cursor through the game's input system (InputSystemWin32Base::ForceCursor, a CName reason and a flag).
// On macOS the cursor is shown and released through AppKit while the overlay is open, and put back as it was when it closes.
struct CursorFallback
{
    bool Active{false};
    bool WasHidden{false};

    // The game view keeps the cursor state itself (macos/PLATFORM_SURVEY.md): -[GameView ShowCursor] clears its hidden flag and
    // re-associates the mouse; -[GameView HideCursor] hides and captures it again. While the overlay is open CET's monitor keeps the
    // mouse events from the view, so the view's mouseMoved:/mouseDown: (the code that would hide and re-centre it) do not run.
    static bool ViewHidesCursor(NSView* aView)
    {
        SEL getter = NSSelectorFromString(@"cursorIsHidden");
        if (aView && [aView respondsToSelector:getter])
            return ((BOOL(*)(id, SEL))objc_msgSend)(aView, getter);
        return !CGCursorIsVisible();
    }

    static void Call(NSView* aView, NSString* aName)
    {
        SEL sel = NSSelectorFromString(aName);
        if (aView && [aView respondsToSelector:sel])
            ((void (*)(id, SEL))objc_msgSend)(aView, sel);
    }

    void Apply(NSView* aView, bool aShow)
    {
        RunOnMainAsync(^{
            const bool hasView = aView && [aView respondsToSelector:NSSelectorFromString(@"ShowCursor")];
            if (aShow && !Active)
            {
                WasHidden = ViewHidesCursor(aView);
                if (WasHidden)
                {
                    if (hasView)
                        Call(aView, @"ShowCursor");
                    else
                    {
                        CGAssociateMouseAndMouseCursorPosition(true);
                        [NSCursor unhide];
                    }
                }
                Active = true;
                Log::Info("[cet] cursor: overlay open, the game had it {}; {}; now hidden by the view: {}, visible on screen: {}",
                          WasHidden ? "hidden" : "shown", WasHidden ? (hasView ? "GameView ShowCursor" : "AppKit unhide") : "left as it was",
                          ViewHidesCursor(aView), static_cast<bool>(CGCursorIsVisible()));
            }
            else if (!aShow && Active)
            {
                if (WasHidden)
                {
                    if (hasView)
                        Call(aView, @"HideCursor");
                    else
                    {
                        [NSCursor hide];
                        CGAssociateMouseAndMouseCursorPosition(false);
                    }
                }
                Active = false;
                Log::Info("[cet] cursor: overlay closed; {}; now hidden by the view: {}, visible on screen: {}",
                          WasHidden ? (hasView ? "GameView HideCursor" : "AppKit hide") : "left as it was", ViewHidesCursor(aView),
                          static_cast<bool>(CGCursorIsVisible()));
            }
        });
    }
};
CursorFallback s_cursorFallback;
} // namespace

D3D12::D3D12(Window& aWindow, Paths& aPaths, Options& aOptions)
    : m_paths(aPaths)
    , m_window(aWindow)
    , m_options(aOptions)
{
    // As on Windows: a repeated game-thread task builds each ImGui frame. On macOS it also renders it (no present hook).
    GameMainThread::Get().AddGenericTask(
        [this]
        {
            PrepareUpdate();
            return false;
        });
}

D3D12::~D3D12()
{
    ResetState(true);
}

void D3D12::SetTrapInputInImGui(const bool acEnabled)
{
    // Windows calls the game's InputSystemWin32Base::ForceCursor on CGameEngine::unkD0. On macOS the engine global is not mapped and
    // the game view hides the cursor itself (macos/PLATFORM_SURVEY.md), so the cursor is shown and released through AppKit while the
    // overlay is open and put back as it was when it closes.
    s_cursorFallback.Apply((__bridge NSView*)m_window.GetView(), acEnabled);

    m_trapInputInImGui = acEnabled;
}

void D3D12::DelayedSetTrapInputInImGui(const bool acEnabled)
{
    m_delayedTrapInputState = acEnabled;
    m_delayedTrapInput = true;
}

void* D3D12::GetDevice() const
{
    return m_device;
}

bool D3D12::ResetState(const bool acDestroyContext)
{
    if (m_initialized)
    {
        std::lock_guard _(m_imguiLock);
        ImGui_ImplMetal_Shutdown();
        if (acDestroyContext)
            ImGui::DestroyContext();
    }

    void* layer = m_layer;
    m_layer = nullptr;
    if (layer)
    {
        RunOnMainAsync(^{
            CAMetalLayer* metalLayer = (__bridge_transfer CAMetalLayer*)layer;
            [metalLayer removeFromSuperlayer];
        });
    }
    if (m_commandQueue)
        CFRelease(m_commandQueue);
    if (m_device)
        CFRelease(m_device);
    m_commandQueue = m_device = nullptr;

    m_outSize = {0, 0};
    m_initialized = false;
    return false;
}

bool D3D12::Initialize()
{
    if (m_initialized)
        return true;

    if (!m_window.IsInitialized())
        return false;

    const SIZE size = m_window.GetClientSize();
    if (size.cx <= 0 || size.cy <= 0)
        return false;

    __block bool created = false;
    __block void* device = nullptr;
    __block void* queue = nullptr;
    __block void* layer = nullptr;
    NSView* view = (__bridge NSView*)m_window.GetView();
    const float scale = m_window.GetBackingScale();
    RunOnMainSync(^{
        id<MTLDevice> mtlDevice = nullptr;
        if ([view.layer isKindOfClass:[CAMetalLayer class]])
            mtlDevice = ((CAMetalLayer*)view.layer).device;
        if (!mtlDevice)
            mtlDevice = MTLCreateSystemDefaultDevice();
        if (!mtlDevice || !view.layer)
            return;

        id<MTLCommandQueue> mtlQueue = [mtlDevice newCommandQueue];
        mtlQueue.label = @"CET overlay";

        CAMetalLayer* metalLayer = [CAMetalLayer layer];
        metalLayer.name = @"CET overlay";
        metalLayer.device = mtlDevice;
        metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
        metalLayer.framebufferOnly = YES;
        metalLayer.opaque = NO;
        metalLayer.displaySyncEnabled = NO;
        metalLayer.maximumDrawableCount = 3;
        metalLayer.allowsNextDrawableTimeout = YES;
        metalLayer.contentsScale = scale;
        metalLayer.frame = view.layer.bounds;
        metalLayer.autoresizingMask = kCALayerWidthSizable | kCALayerHeightSizable;
        metalLayer.drawableSize = CGSizeMake(size.cx, size.cy);
        metalLayer.zPosition = 1000.0;
        metalLayer.hidden = YES;
        [view.layer addSublayer:metalLayer];

        device = (__bridge_retained void*)mtlDevice;
        queue = (__bridge_retained void*)mtlQueue;
        layer = (__bridge_retained void*)metalLayer;
        created = true;
    });

    if (!created)
    {
        Log::Error("D3D12::Initialize() (macOS) - could not create the Metal device, queue or overlay layer");
        m_initFailed = true;
        return false;
    }

    m_device = device;
    m_commandQueue = queue;
    m_layer = layer;
    m_outSize = size;
    if (!m_inFlight)
        m_inFlight = (__bridge_retained void*)dispatch_semaphore_create(2);

    if (!InitializeImGui())
    {
        Log::Error("D3D12::Initialize() (macOS) - failed to initialize ImGui!");
        m_initFailed = true;
        return ResetState();
    }

    Log::Info("D3D12::Initialize() (macOS) - Metal overlay layer {}x{} px (scale {:.1f}) on the game view; initialization successful!", size.cx,
              size.cy, scale);
    m_initialized = true;
    OnInitialized.Emit();
    return true;
}

void D3D12::ReloadFonts()
{
    std::lock_guard _(m_imguiLock);

    // TODO - scale also by DPI
    const auto [resx, resy] = m_outSize;
    const auto scaleFromReference = std::min(static_cast<float>(resx) / 1920.0f, static_cast<float>(resy) / 1080.0f);

    auto& io = ImGui::GetIO();
    io.Fonts->Clear();

    ImFontConfig config;
    const auto& fontSettings = m_options.Font;
    config.SizePixels = std::floor(fontSettings.BaseSize * scaleFromReference);
    config.OversampleH = fontSettings.OversampleHorizontal;
    config.OversampleV = fontSettings.OversampleVertical;
    if (config.OversampleH == 1 && config.OversampleV == 1)
        config.PixelSnapH = true;
    config.MergeMode = false;

    // add default font
    const auto customFontPath = fontSettings.Path.empty() ? std::filesystem::path{} : GetAbsolutePath(fontSettings.Path, m_paths.Fonts(), false);
    auto cetFontPath = GetAbsolutePath(std::string("NotoSans-Regular.ttf"), m_paths.Fonts(), false);
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
            io.Fonts->AddFontFromFileTTF(cetFontPath.c_str(), config.SizePixels, &config, cpGlyphRanges);
    }
    else
        io.Fonts->AddFontFromFileTTF(customFontPath.c_str(), config.SizePixels, &config, cpGlyphRanges);

    // the language font: from the settings, else from the system's preferred language (Windows: GetSystemDefaultLangID)
    std::string language = fontSettings.Language;
    if (language.empty() || language == "Default")
    {
        NSString* preferred = [NSLocale preferredLanguages].firstObject;
        const std::string code = preferred ? preferred.UTF8String : "";
        if (code.rfind("zh-Hant", 0) == 0 || code.rfind("zh-TW", 0) == 0 || code.rfind("zh-HK", 0) == 0)
            language = "ChineseFull";
        else if (code.rfind("zh", 0) == 0)
            language = "ChineseSimplifiedCommon";
        else if (code.rfind("ja", 0) == 0)
            language = "Japanese";
        else if (code.rfind("ko", 0) == 0)
            language = "Korean";
        else if (code.rfind("ru", 0) == 0 || code.rfind("be", 0) == 0)
            language = "Cyrillic";
        else if (code.rfind("th", 0) == 0)
            language = "Thai";
        else if (code.rfind("vi", 0) == 0)
            language = "Vietnamese";
    }
    if (language == "ChineseFull")
    {
        cetFontPath = GetAbsolutePath(std::string("NotoSansTC-Regular.otf"), m_paths.Fonts(), false);
        cpGlyphRanges = io.Fonts->GetGlyphRangesChineseFull();
    }
    else if (language == "ChineseSimplifiedCommon")
    {
        cetFontPath = GetAbsolutePath(std::string("NotoSansSC-Regular.otf"), m_paths.Fonts(), false);
        cpGlyphRanges = io.Fonts->GetGlyphRangesChineseSimplifiedCommon();
    }
    else if (language == "Japanese")
    {
        cetFontPath = GetAbsolutePath(std::string("NotoSansJP-Regular.otf"), m_paths.Fonts(), false);
        cpGlyphRanges = io.Fonts->GetGlyphRangesJapanese();
    }
    else if (language == "Korean")
    {
        cetFontPath = GetAbsolutePath(std::string("NotoSansKR-Regular.otf"), m_paths.Fonts(), false);
        cpGlyphRanges = io.Fonts->GetGlyphRangesKorean();
    }
    else if (language == "Cyrillic")
    {
        cetFontPath = GetAbsolutePath(std::string("NotoSans-Regular.ttf"), m_paths.Fonts(), false);
        cpGlyphRanges = io.Fonts->GetGlyphRangesCyrillic();
    }
    else if (language == "Thai")
    {
        cetFontPath = GetAbsolutePath(std::string("NotoSansThai-Regular.ttf"), m_paths.Fonts(), false);
        cpGlyphRanges = io.Fonts->GetGlyphRangesThai();
    }
    else if (language == "Vietnamese")
    {
        cetFontPath = GetAbsolutePath(std::string("NotoSans-Regular.ttf"), m_paths.Fonts(), false);
        cpGlyphRanges = io.Fonts->GetGlyphRangesVietnamese();
    }

    // add extra glyphs from language font
    config.MergeMode = true;
    if (customFontPath.empty())
    {
        if (cetFontPath.empty())
        {
            Log::Warn("D3D12::ReloadFonts() - Missing fonts for extra language glyphs!");
            io.Fonts->AddFontDefault(&config);
        }
        else
            io.Fonts->AddFontFromFileTTF(cetFontPath.c_str(), config.SizePixels, &config, cpGlyphRanges);
    }
    else
        io.Fonts->AddFontFromFileTTF(customFontPath.c_str(), config.SizePixels, &config, cpGlyphRanges);

    // add icons from fontawesome4
    config.GlyphMinAdvanceX = config.SizePixels;
    static const ImWchar icon_ranges[] = {ICON_MIN_MD, ICON_MAX_MD, 0};
    auto cetIconPath = GetAbsolutePath(std::string("materialdesignicons.ttf"), m_paths.Fonts(), false);
    if (!cetIconPath.empty())
        io.Fonts->AddFontFromFileTTF(cetIconPath.c_str(), config.SizePixels, &config, icon_ranges);
}

bool D3D12::InitializeImGui()
{
    std::lock_guard _(m_imguiLock);

    // TODO - scale also by DPI
    const auto [resx, resy] = m_outSize;
    const auto scaleFromReference = std::min(static_cast<float>(resx) / 1920.0f, static_cast<float>(resy) / 1080.0f);

    if (ImGui::GetCurrentContext() == nullptr)
    {
        // do this once, do not repeat context creation!
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        // TODO - make this configurable eventually and overridable by mods for themselves easily
        // setup CET default style
        ImGui::StyleColorsDark(&m_styleReference);
        m_styleReference.WindowRounding = 6.0f;
        m_styleReference.WindowTitleAlign.x = 0.5f;
        m_styleReference.ChildRounding = 6.0f;
        m_styleReference.PopupRounding = 6.0f;
        m_styleReference.FrameRounding = 6.0f;
        m_styleReference.ScrollbarRounding = 12.0f;
        m_styleReference.GrabRounding = 12.0f;
        m_styleReference.TabRounding = 6.0f;
    }

    ImGui::GetStyle() = m_styleReference;
    ImGui::GetStyle().ScaleAllSizes(scaleFromReference);

    auto& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange; // Do not modify cursor from ImGui backend
    io.BackendPlatformName = "cet_macos";
    // ImGui sizes are in pixels as on Windows (the drawable is the view size times the backing scale), so CET's 1080p-relative scaling
    // gives the same layout at the same resolution.
    io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

    static std::string s_layoutPath;
    s_layoutPath = GetAbsolutePath(std::string("layout.ini"), m_paths.CETRoot(), true).string();
    io.IniFilename = s_layoutPath.c_str();

    if (!ImGui_ImplMetal_Init((__bridge id<MTLDevice>)m_device))
    {
        Log::Error("D3D12::InitializeImGui() - ImGui_ImplMetal_Init call failed!");
        return false;
    }

    ReloadFonts();

    if (!ImGui_ImplMetal_CreateDeviceObjects((__bridge id<MTLDevice>)m_device))
    {
        Log::Error("D3D12::InitializeImGui() - ImGui_ImplMetal_CreateDeviceObjects call failed!");
        ImGui_ImplMetal_Shutdown();
        return false;
    }

    return true;
}

void D3D12::PrepareUpdate()
{
    if (!m_initialized)
    {
        if (m_initFailed || !Initialize())
            return;
    }

    std::lock_guard _(m_imguiLock);

    // the game view changed size (window mode, fullscreen, display): resize the layer, rescale the style and rebuild the fonts
    const SIZE size = m_window.GetClientSize();
    if (size.cx > 0 && size.cy > 0 && (size.cx != m_outSize.cx || size.cy != m_outSize.cy))
    {
        Log::Info("D3D12 (macOS) - game view resized to {}x{} px", size.cx, size.cy);
        m_outSize = size;
        void* layer = m_layer;
        const float scale = m_window.GetBackingScale();
        RunOnMainAsync(^{
            CAMetalLayer* metalLayer = (__bridge CAMetalLayer*)layer;
            metalLayer.contentsScale = scale;
            metalLayer.drawableSize = CGSizeMake(size.cx, size.cy);
        });
        const auto scaleFromReference = std::min(static_cast<float>(size.cx) / 1920.0f, static_cast<float>(size.cy) / 1080.0f);
        ImGui::GetStyle() = m_styleReference;
        ImGui::GetStyle().ScaleAllSizes(scaleFromReference);
        ImGui_ImplMetal_DestroyFontsTexture();
        ReloadFonts();
        ImGui_ImplMetal_CreateFontsTexture((__bridge id<MTLDevice>)m_device);
    }

    if (m_delayedTrapInput)
    {
        SetTrapInputInImGui(m_delayedTrapInputState);
        m_delayedTrapInput = false;
    }

    // input queued by the AppKit monitor: ImGui gets it always (it only acts on it while a window wants it), VKBindings gets keys and
    // buttons as it gets raw input on Windows
    static std::vector<MacInputEvent> s_events;
    s_events.clear();
    m_window.DrainInput(s_events);
    auto& io = ImGui::GetIO();
    auto& bindings = CET::Get().GetBindings();
    for (const auto& e : s_events)
    {
        switch (e.EventType)
        {
        case MacInputEvent::Type::KeyDown:
        case MacInputEvent::Type::KeyUp:
        {
            const bool down = e.EventType == MacInputEvent::Type::KeyDown;
            io.AddKeyEvent(ImGuiMod_Shift, e.Shift);
            io.AddKeyEvent(ImGuiMod_Ctrl, e.Ctrl);
            io.AddKeyEvent(ImGuiMod_Alt, e.Alt);
            io.AddKeyEvent(ImGuiMod_Super, e.Super);
            if (e.ImGuiKeyCode != ImGuiKey_None)
                io.AddKeyEvent(static_cast<ImGuiKey>(e.ImGuiKeyCode), down);
            if (e.VKCode != 0)
                bindings.OnMacKey(e.VKCode, down);
            break;
        }
        case MacInputEvent::Type::Text: io.AddInputCharacter(e.Character); break;
        case MacInputEvent::Type::MouseMove: io.AddMousePosEvent(e.X, e.Y); break;
        case MacInputEvent::Type::MouseDown:
        case MacInputEvent::Type::MouseUp:
        {
            const bool down = e.EventType == MacInputEvent::Type::MouseDown;
            io.AddMouseButtonEvent(e.MouseButton, down);
            bindings.OnMacKey(e.VKCode, down);
            break;
        }
        case MacInputEvent::Type::Wheel:
            io.AddMouseWheelEvent(e.WheelX, e.WheelY);
            if (e.WheelY != 0.0f)
                bindings.OnMacWheel(e.WheelY > 0.0f ? 1 : -1);
            if (e.WheelX != 0.0f)
                bindings.OnMacWheel(e.WheelX > 0.0f ? 2 : -2);
            break;
        case MacInputEvent::Type::FocusLost:
            io.AddFocusEvent(false);
            bindings.OnMacFocusLost();
            break;
        }
    }

    static auto s_lastFrame = std::chrono::steady_clock::now();
    const auto now = std::chrono::steady_clock::now();
    const float delta = std::chrono::duration<float>(now - s_lastFrame).count();
    s_lastFrame = now;
    io.DisplaySize = ImVec2(static_cast<float>(m_outSize.cx), static_cast<float>(m_outSize.cy));
    io.DeltaTime = delta > 0.0f ? delta : 1.0f / 60.0f;

    ImGui::NewFrame();

    CET::Get().GetOverlay().Update();

    CET::Get().GetVM().Draw();

    ImGui::Render();
    ++m_framesBuilt;

    Render();
}

void D3D12::Render()
{
    ImDrawData* drawData = ImGui::GetDrawData();
    const bool needed = CET::Get().GetOverlay().IsEnabled() || (drawData && drawData->TotalVtxCount > 0);

    CAMetalLayer* layer = (__bridge CAMetalLayer*)m_layer;
    if (!needed)
    {
        if (m_layerVisible.exchange(false))
        {
            RunOnMainAsync(^{
                layer.hidden = YES;
            });
        }
        return;
    }

    dispatch_semaphore_t inFlight = (__bridge dispatch_semaphore_t)m_inFlight;
    if (dispatch_semaphore_wait(inFlight, DISPATCH_TIME_NOW) != 0)
        return; // two frames still on the GPU: skip this one rather than block the game thread

    @autoreleasepool
    {
        id<CAMetalDrawable> drawable = [layer nextDrawable];
        if (!drawable)
        {
            dispatch_semaphore_signal(inFlight);
            return;
        }

        MTLRenderPassDescriptor* pass = [MTLRenderPassDescriptor renderPassDescriptor];
        pass.colorAttachments[0].texture = drawable.texture;
        pass.colorAttachments[0].loadAction = MTLLoadActionClear;
        pass.colorAttachments[0].clearColor = MTLClearColorMake(0, 0, 0, 0);
        pass.colorAttachments[0].storeAction = MTLStoreActionStore;

        id<MTLCommandQueue> queue = (__bridge id<MTLCommandQueue>)m_commandQueue;
        id<MTLCommandBuffer> commandBuffer = [queue commandBuffer];
        commandBuffer.label = @"CET overlay";

        ImGui_ImplMetal_NewFrame(pass);
        id<MTLRenderCommandEncoder> encoder = [commandBuffer renderCommandEncoderWithDescriptor:pass];
        encoder.label = @"CET ImGui";
        ImGui_ImplMetal_RenderDrawData(drawData, commandBuffer, encoder);
        [encoder endEncoding];

        [commandBuffer presentDrawable:drawable];
        [commandBuffer addCompletedHandler:^(id<MTLCommandBuffer>) {
            dispatch_semaphore_signal(inFlight);
        }];
        [commandBuffer commit];
    }
    ++m_framesPresented;

    if (!m_layerVisible.exchange(true))
    {
        RunOnMainAsync(^{
            layer.hidden = NO;
        });
    }
}

// Texture.cpp (Lua ImGui images): an RGBA8 texture with the decoded pixels, retained for the caller. ImGui's Metal backend draws it
// when the id<MTLTexture> is passed as the ImTextureID.
void* CetMacCreateRGBA8Texture(void* apDevice, const unsigned char* apPixels, int aWidth, int aHeight)
{
    id<MTLDevice> device = (__bridge id<MTLDevice>)apDevice;
    if (!device || !apPixels || aWidth <= 0 || aHeight <= 0)
        return nullptr;

    MTLTextureDescriptor* descriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm
                                                                                          width:static_cast<NSUInteger>(aWidth)
                                                                                         height:static_cast<NSUInteger>(aHeight)
                                                                                      mipmapped:NO];
    descriptor.usage = MTLTextureUsageShaderRead;
    descriptor.storageMode = MTLStorageModeShared;
    id<MTLTexture> texture = [device newTextureWithDescriptor:descriptor];
    if (!texture)
        return nullptr;
    [texture replaceRegion:MTLRegionMake2D(0, 0, static_cast<NSUInteger>(aWidth), static_cast<NSUInteger>(aHeight))
               mipmapLevel:0
                 withBytes:apPixels
               bytesPerRow:static_cast<NSUInteger>(aWidth) * 4];
    return (__bridge_retained void*)texture;
}

void CetMacReleaseTexture(void* apTexture)
{
    if (apTexture)
        CFRelease(apTexture);
}
