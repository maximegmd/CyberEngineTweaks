#pragma once

using TVKBindCallback = void();

using VKCodeBindDecoded = std::array<uint8_t, 4>;

struct VKBind
{
    std::string ID { };
    std::string Description { };
    std::function<TVKBindCallback> Handler { nullptr };
};

struct VKBindInfo
{
    VKBind Bind { };
    UINT CodeBind { 0 };
    UINT SavedCodeBind { 0 };
    bool IsBinding{ false };

    void Fill(UINT aVKCodeBind, const VKBind& aVKBind);

    UINT Apply();
};

struct Overlay;

struct VKBindings
{
    VKBindings(Paths& aPaths);
    ~VKBindings() = default;

    [[nodiscard]] bool IsInitialized() const noexcept;

    std::vector<VKBindInfo> InitializeMods(std::vector<VKBindInfo> aVKBindInfos);

    static VKCodeBindDecoded DecodeVKCodeBind(UINT aVKCodeBind);
    static UINT EncodeVKCodeBind(const VKCodeBindDecoded& aVKCodeBindDecoded);
    static const char* GetSpecialKeyName(UINT aVKCode);
    
    void Load(Overlay& aOverlay);
    void Save();
    
    bool Bind(UINT aVKCodeBind, const VKBind& aBind);
    bool UnBind(UINT aVKCodeBind);
    bool UnBind(const std::string& aID);
    bool IsBound(UINT aVKCodeBind) const;
    bool IsBound(const std::string& aID) const;
    
    UINT GetBindCodeForID(const std::string& aID);
    std::string GetIDForBindCode(UINT aVKCodeBind);
    
    bool StartRecordingBind(const VKBind& aBind);
    bool StopRecordingBind();

    bool IsRecordingBind() const;
    UINT GetLastRecordingResult() const;

    LRESULT OnWndProc(HWND ahWnd, UINT auMsg, WPARAM awParam, LPARAM alParam);

private:

    bool IsLastRecordingKey(UINT aVKCode);
    LRESULT RecordKeyDown(UINT aVKCode);
    LRESULT RecordKeyUp(UINT aVKCode);
    bool VerifyRecording();

    LRESULT HandleRAWInput(HRAWINPUT ahRAWInput);
    
    TiltedPhoques::Map<UINT, VKBind> m_binds{};
    TiltedPhoques::Map<std::string, UINT> m_idToBind{ };
    
    VKCodeBindDecoded m_recording{ };
    size_t m_recordingLength{ 0 };
    VKBind m_recordingBind { };
    UINT m_recordingResult{ 0 };
    bool m_isBindRecording{ false };
    bool m_initialized{ false };

    Paths& m_paths;
    Overlay* m_pOverlay{nullptr};
};
