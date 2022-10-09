#pragma once

using TVKBindHotkeyCallback = void();
using TVKBindInputCallback = void(bool);

using VKCodeBindDecoded = std::array<uint16_t, 4>;

struct VKModBind
{
    std::string ModName;
    std::string ID;

    [[nodiscard]] auto operator<=>(const VKModBind&) const = default;
};

struct VKBind
{
    std::string ID{};
    std::string DisplayName{};
    std::variant<std::string, std::function<void()>> Description{};
    std::variant<std::function<TVKBindHotkeyCallback>, std::function<TVKBindInputCallback>> Handler{};

    [[nodiscard]] std::function<void()> DelayedCall(bool isDown) const;
    void Call(bool isDown) const;

    [[nodiscard]] bool IsHotkey() const;
    [[nodiscard]] bool IsInput() const;

    [[nodiscard]] bool HasSimpleDescription() const;
    [[nodiscard]] bool HasComplexDescription() const;

    [[nodiscard]] bool operator==(const std::string& id) const;
};

constexpr USHORT VKBC_MWHEELUP    { RI_MOUSE_WHEEL  | 1 };
constexpr USHORT VKBC_MWHEELDOWN  { RI_MOUSE_WHEEL  | 0 };
constexpr USHORT VKBC_MWHEELRIGHT { RI_MOUSE_HWHEEL | 1 };
constexpr USHORT VKBC_MWHEELLEFT  { RI_MOUSE_HWHEEL | 0 };

struct Options;
struct Overlay;
struct D3D12;
struct LuaVM;
struct VKBindings
{
    VKBindings(Paths& aPaths, const Options& acOptions);
    ~VKBindings() = default;

    [[nodiscard]] bool IsInitialized() const noexcept;

    void InitializeMods(const TiltedPhoques::Map<std::string, std::reference_wrapper<const TiltedPhoques::Vector<VKBind>>>& acVKBinds);

    [[nodiscard]] static VKCodeBindDecoded DecodeVKCodeBind(uint64_t aVKCodeBind);
    [[nodiscard]] static uint64_t EncodeVKCodeBind(VKCodeBindDecoded aVKCodeBindDecoded);
    [[nodiscard]] static const char* GetSpecialKeyName(USHORT aVKCode);

    void Load();
    void Save();

    void Update();

    bool Bind(uint64_t aVKCodeBind, const VKModBind& acVKModBind);
    bool UnBind(uint64_t aVKCodeBind);
    bool UnBind(const VKModBind& acVKModBind);
    [[nodiscard]] bool IsBound(uint64_t aVKCodeBind) const;
    [[nodiscard]] bool IsBound(const VKModBind& acVKModBind) const;
    [[nodiscard]] bool IsFirstKeyUsed(uint64_t aVKCodeBind) const;

    [[nodiscard]] static std::string GetBindString(uint64_t aVKCodeBind);
    [[nodiscard]] std::string GetBindString(const VKModBind& acVKModBind) const;

    [[nodiscard]] uint64_t GetBindCodeForModBind(const VKModBind& acVKModBind, bool aIncludeDead = false) const;
    [[nodiscard]] const VKModBind* GetModBindForBindCode(uint64_t aVKCodeBind) const;
    [[nodiscard]] const VKModBind* GetModBindStartingWithBindCode(uint64_t aVKCodeBind) const;

    bool StartRecordingBind(const VKModBind& acVKModBind);
    bool StopRecordingBind();

    [[nodiscard]] bool IsRecordingBind() const;
    [[nodiscard]] uint64_t GetLastRecordingResult() const;

    LRESULT OnWndProc(HWND ahWnd, UINT auMsg, WPARAM awParam, LPARAM alParam);

    void ConnectUpdate(D3D12& aD3D12);
    void DisconnectUpdate(D3D12& aD3D12);

    void SetVM(const LuaVM* acpVM);

private:

    [[nodiscard]] bool IsLastRecordingKey(USHORT aVKCode) const;
    LRESULT RecordKeyDown(USHORT aVKCode);
    LRESULT RecordKeyUp(USHORT aVKCode);

    [[nodiscard]] int32_t CheckRecording();
    void ExecuteRecording(bool aLastKeyDown);

    [[nodiscard]] LRESULT HandleRAWInput(HRAWINPUT ahRAWInput);

    void ClearRecording(bool aClearBind = true);

    std::bitset<1 << 16> m_keyStates{ };

    std::map<uint64_t, VKModBind> m_binds{ }; // this map needs to be ordered!
    TiltedPhoques::Map<std::string, TiltedPhoques::Map<std::string, uint64_t>> m_modIdToBinds{ };

    TiltedPhoques::TaskQueue m_queuedCallbacks{ };

    VKCodeBindDecoded m_recording{ };
    size_t m_recordingLength{ 0 };
    bool m_recordingHKWasKeyPressed{ false };
    VKModBind m_recordingModBind{ };
    uint64_t m_recordingResult{ 0 };

    bool m_isBindRecording{ false };
    bool m_initialized{ false };

    const LuaVM* m_cpVm{ nullptr };
    Paths& m_paths;
    const Options& m_cOptions;

    size_t m_connectUpdate{ static_cast<size_t>(-1) };
};
