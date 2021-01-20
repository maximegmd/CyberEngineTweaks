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

    void Fill(UINT aVKCodeBind, const VKBind& aVKBind)
    {
        Bind = aVKBind;
        CodeBind = aVKCodeBind;
        SavedCodeBind = aVKCodeBind;
        IsBinding = false;
    }

    UINT Apply()
    {
        SavedCodeBind = CodeBind;
        return CodeBind;
    }
};

struct VKBindings
{
    static void Initialize();
    static std::vector<VKBindInfo> InitializeMods(std::vector<VKBindInfo> aVKBindInfos);
    static void Shutdown();
    static bool IsInitialized();
    static VKBindings& Get();

    static VKCodeBindDecoded DecodeVKCodeBind(UINT aVKCodeBind);
    static UINT EncodeVKCodeBind(const VKCodeBindDecoded& aVKCodeBindDecoded);

    static const char* GetSpecialKeyName(UINT aVKCode);
    
    void Load();
    void Save();
    
    bool Bind(UINT aVKCodeBind, const VKBind& aBind);
    bool UnBind(UINT aVKCodeBind);
    bool UnBind(const std::string& aID);
    bool IsBound(UINT aVKCodeBind);
    bool IsBound(const std::string& aID);
    
    UINT GetBindCodeForID(const std::string& aID);
    std::string GetIDForBindCode(UINT aVKCodeBind);
    
    bool StartRecordingBind(const VKBind& aBind);
    bool StopRecordingBind();

    bool IsRecordingBind();
    UINT GetLastRecordingResult();

    LRESULT OnWndProc(HWND ahWnd, UINT auMsg, WPARAM awParam, LPARAM alParam);

private:
    bool IsLastRecordingKey(UINT aVKCode);
    LRESULT RecordKeyDown(UINT aVKCode);
    LRESULT RecordKeyUp(UINT aVKCode);
    bool VerifyRecording();

    LRESULT HandleRAWInput(HRAWINPUT ahRAWInput);
    
    std::map<UINT, VKBind> Binds{ };
    std::unordered_map<std::string, UINT> IDToBinds{ };
    
    VKCodeBindDecoded m_recording{ };
    size_t m_recordingLength{ 0 };
    VKBind m_recordingBind { };
    UINT m_recordingResult{ 0 };
    bool m_isBindRecording{ false };

    bool m_initialized{ false };
};
