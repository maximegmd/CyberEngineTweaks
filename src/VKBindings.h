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
    bool m_isOverlayHotkey{false};

    [[nodiscard]] std::function<void()> DelayedCall(const bool acIsDown) const;
    void Call(const bool acIsDown) const;

    [[nodiscard]] bool IsHotkey() const;
    [[nodiscard]] bool IsInput() const;

    [[nodiscard]] bool HasSimpleDescription() const;
    [[nodiscard]] bool HasComplexDescription() const;

    [[nodiscard]] bool operator==(const std::string& acpId) const;
};

constexpr USHORT VKBC_MWHEELUP{RI_MOUSE_WHEEL | 1};
constexpr USHORT VKBC_MWHEELDOWN{RI_MOUSE_WHEEL | 0};
constexpr USHORT VKBC_MWHEELRIGHT{RI_MOUSE_HWHEEL | 1};
constexpr USHORT VKBC_MWHEELLEFT{RI_MOUSE_HWHEEL | 0};

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

    [[nodiscard]] static VKCodeBindDecoded DecodeVKCodeBind(const uint64_t acVKCodeBind);
    [[nodiscard]] static uint64_t EncodeVKCodeBind(VKCodeBindDecoded aVKCodeBindDecoded);
    [[nodiscard]] static const char* GetSpecialKeyName(const USHORT acVKCode);

    void Load();
    void Save();

    void Update();

    bool Bind(const uint64_t acVKCodeBind, const VKModBind& acVKModBind);
    bool UnBind(const uint64_t acVKCodeBind);
    bool UnBind(const VKModBind& acVKModBind);
    [[nodiscard]] bool IsBound(const uint64_t acVKCodeBind) const;
    [[nodiscard]] bool IsBound(const VKModBind& acVKModBind) const;
    [[nodiscard]] bool IsFirstKeyUsed(const uint64_t acVKCodeBind) const;

    [[nodiscard]] static std::string GetBindString(const uint64_t acVKCodeBind);
    [[nodiscard]] std::string GetBindString(const VKModBind& acVKModBind) const;

    [[nodiscard]] uint64_t GetBindCodeForModBind(const VKModBind& acVKModBind, const bool acIncludeDead = false) const;
    [[nodiscard]] const VKModBind* GetModBindForBindCode(const uint64_t acVKCodeBind) const;
    [[nodiscard]] const VKModBind* GetModBindStartingWithBindCode(const uint64_t acVKCodeBind) const;

    bool StartRecordingBind(const VKModBind& acVKModBind);
    bool StopRecordingBind();

    [[nodiscard]] bool IsRecordingBind() const;
    [[nodiscard]] uint64_t GetLastRecordingResult() const;

    LRESULT OnWndProc(HWND ahWnd, UINT auMsg, WPARAM awParam, LPARAM alParam);

    void SetVM(const LuaVM* acpVm);

private:
    [[nodiscard]] LRESULT HandleRAWInput(HRAWINPUT achRAWInput);

    LRESULT RecordKeyDown(const USHORT acVKCode);
    LRESULT RecordKeyUp(const USHORT acVKCode);

    void ExecuteSingleInput(const USHORT acVKCode, const bool acKeyDown);
    void ExecuteRecording();
    void ClearRecording(const bool acClearBind);

    std::map<uint64_t, VKModBind> m_binds{}; // this map needs to be ordered!
    TiltedPhoques::Map<std::string, TiltedPhoques::Map<std::string, uint64_t>> m_modIdToBinds{};
    TiltedPhoques::TaskQueue m_queuedCallbacks{};

    std::bitset<1 << 16> m_keyStates{};

    VKCodeBindDecoded m_recording{};
    uint64_t m_recordingResult{0};
    size_t m_recordingLength{0};
    bool m_recordingWasKeyPressed{false};

    VKModBind m_recordingModBind{};
    bool m_isBindRecording{false};

    bool m_initialized{false};

    const LuaVM* m_cpVm{nullptr};
    Paths& m_paths;
    const Options& m_cOptions;
};
