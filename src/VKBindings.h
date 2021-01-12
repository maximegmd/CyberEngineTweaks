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
    static void InitializeMods(std::vector<VKBindInfo>& aVKBindInfos);
    static void Shutdown();
    static bool IsInitialized();
    
    static void Load();
    static void Save();
    
    static bool Bind(UINT aVKCodeBind, const VKBind& aBind);
    static bool UnBind(UINT aVKCodeBind);
    static bool UnBind(const std::string& aID);
    static bool IsBound(UINT aVKCodeBind);
    static bool IsBound(const std::string& aID);
    
    static UINT GetBindCodeForID(const std::string& aID);
    static std::string GetIDForBindCode(UINT aVKCodeBind);

    static VKCodeBindDecoded DecodeVKCodeBind(UINT aVKCodeBind);
    static UINT EncodeVKCodeBind(const VKCodeBindDecoded& aVKCodeBindDecoded);
    
    static bool StartRecordingBind(const VKBind& aBind);
    static bool StopRecordingBind();

    static bool IsRecordingBind();
    static UINT GetLastRecordingResult();
    
    static LRESULT OnWndProc(HWND ahWnd, UINT auMsg, WPARAM awParam, LPARAM alParam);

private:
    static bool IsLastRecordingKey(UINT aVKCode);
    static bool RecordKeyDown(UINT aVKCode);
    static bool RecordKeyUp(UINT aVKCode);
    static UINT CreateVKCodeBindFromRecording();

    static LRESULT HandleRAWInput(HRAWINPUT ahRAWInput);
    
    static inline std::map<UINT, VKBind> Binds{ };
    static inline std::unordered_map<std::string, UINT> IDToBinds{ };
    
    static inline VKCodeBindDecoded Recording{ };
    static inline size_t RecordingLength{ 0 };
    static inline VKBind RecordingBind { };
    static inline UINT RecordingResult{ 0 };
    static inline bool IsBindRecording{ false };

    static inline bool Initialized{ false };
};
