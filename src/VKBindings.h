#pragma once

using TVKBindHotkeyCallback = void();
using TVKBindInputCallback = void(bool);

using VKCodeBindDecoded = std::array<uint16_t, 4>;

struct VKBind
{
    std::string ID { };
    std::string Description { };
    std::variant<std::function<TVKBindHotkeyCallback>, std::function<TVKBindInputCallback>> Handler{};

    [[nodiscard]] std::function<void()> DelayedCall(bool isDown) const
    {
        if (!isDown) // hotkeys only on key up
        {
            const auto* fn = std::get_if<std::function<TVKBindHotkeyCallback>>(&Handler);
            if (fn)
                return *fn;
        }

        {
            const auto* fn = std::get_if<std::function<TVKBindInputCallback>>(&Handler);
            if (fn)
                return std::bind(*fn, isDown);
        }

        assert(isDown); // nullptr should ever return only for key down events, in case binding is a hotkey
        return nullptr;
    }

    void Call(bool isDown) const
    {
        auto fn { DelayedCall(isDown) };
        if (fn)
            fn();
    }

    [[nodiscard]] bool IsValid() const
    {
        return (Handler.index() != std::variant_npos);
    }

    [[nodiscard]] bool IsHotkey() const
    {
        return std::holds_alternative<std::function<TVKBindHotkeyCallback>>(Handler);
    }

    [[nodiscard]] bool IsInput() const
    {
        return std::holds_alternative<std::function<TVKBindInputCallback>>(Handler);
    }
};

struct VKBindInfo
{
    VKBind Bind { };
    uint64_t CodeBind { 0 };
    uint64_t SavedCodeBind{ 0 };
    bool IsBinding{ false };

    void Fill(uint64_t aVKCodeBind, const VKBind& aVKBind);

    uint64_t Apply();
};

static const VKBind* VKBRecord_OK { reinterpret_cast<const VKBind*>(true) };

constexpr USHORT VKBC_MWHEELUP    { RI_MOUSE_WHEEL  | 1 };
constexpr USHORT VKBC_MWHEELDOWN  { RI_MOUSE_WHEEL  | 0 };
constexpr USHORT VKBC_MWHEELRIGHT { RI_MOUSE_HWHEEL | 1 };
constexpr USHORT VKBC_MWHEELLEFT  { RI_MOUSE_HWHEEL | 0 };

struct Overlay;
struct D3D12;
struct VKBindings
{
    VKBindings(Paths& aPaths);
    ~VKBindings() = default;

    [[nodiscard]] bool IsInitialized() const noexcept;

    std::vector<VKBindInfo> InitializeMods(std::vector<VKBindInfo> aVKBindInfos);

    static VKCodeBindDecoded DecodeVKCodeBind(uint64_t aVKCodeBind);
    static uint64_t EncodeVKCodeBind(VKCodeBindDecoded aVKCodeBindDecoded);
    static const char* GetSpecialKeyName(USHORT aVKCode);
    
    void Load(const Overlay& aOverlay);
    void Save();

    void Update();

    void Clear();
    bool Bind(uint64_t aVKCodeBind, const VKBind& aBind);
    bool UnBind(uint64_t aVKCodeBind);
    bool UnBind(const std::string& aID);
    bool IsBound(uint64_t aVKCodeBind) const;
    bool IsBound(const std::string& aID) const;

    static std::string GetBindString(uint64_t aVKCodeBind);
    std::string GetBindString(const std::string aID) const;

    uint64_t GetBindCodeForID(const std::string& aID) const;
    std::string GetIDForBindCode(uint64_t aVKCodeBind) const;
    
    bool StartRecordingBind(const VKBind& aBind);
    bool StopRecordingBind();

    bool IsRecordingBind() const;
    uint64_t GetLastRecordingResult() const;

    LRESULT OnWndProc(HWND ahWnd, UINT auMsg, WPARAM awParam, LPARAM alParam);

    void ConnectUpdate(D3D12& aD3D12);
    void DisconnectUpdate(D3D12& aD3D12);

private:

    bool IsLastRecordingKey(USHORT aVKCode);
    LRESULT RecordKeyDown(USHORT aVKCode);
    LRESULT RecordKeyUp(USHORT aVKCode);

    const VKBind* VerifyRecording();

    LRESULT HandleRAWInput(HRAWINPUT ahRAWInput);

    std::map<uint64_t, VKBind> m_binds{ };
    TiltedPhoques::Map<std::string, uint64_t> m_idToBind{ };

    std::mutex m_queuedCallbacksLock{ };
    std::queue<std::function<void()>> m_queuedCallbacks{};
    
    VKCodeBindDecoded m_recording{ };
    size_t m_recordingLength{ 0 };
    VKBind m_recordingBind { };
    uint64_t m_recordingResult{ 0 };
    bool m_isBindRecording{ false };
    bool m_initialized{ false };

    Paths& m_paths;
    const Overlay* m_pOverlay{ nullptr };
    
    size_t m_connectUpdate{ static_cast<size_t>(-1) };
};
