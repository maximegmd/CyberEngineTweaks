#pragma once

using TVKBindCallback = void();

struct VKBind
{
    std::string ID { };
    TVKBindCallback* Handler { nullptr };
};

struct VKBindings
{
    static void Initialize();
    
    static void Load();
    static void Save();
    
    static bool Bind(UINT aVKCodeBind, const VKBind& aBind);
    static bool UnBind(UINT aVKCodeBind);
    static bool UnBind(const std::string& aID);
    static bool IsBound(UINT aVKCodeBind);
    static bool IsBound(const std::string& aID);

    static void StartRecordingBind(const VKBind& aBind);

    static bool IsRecordingBind();
    static UINT GetLastRecordingResult();
    
    static LRESULT OnWndProc(HWND ahWnd, UINT auMsg, WPARAM awParam, LPARAM alParam);

private:
    static bool IsLastRecordingKey(UINT aVKCode);
    static void RecordKeyDown(UINT aVKCode);
    static void RecordKeyUp(UINT aVKCode);
    static UINT CreateVKCodeBindFromRecording();
    
    static inline std::map<UINT, VKBind> Binds{ };
    static inline std::unordered_map<std::string, UINT> IDToBinds{ };
    
    static inline std::array<UINT, 4> Recording{ };
    static inline std::array<bool, 4> RecordingUp{ };
    static inline size_t RecordingLength{ 0 };
    static inline VKBind RecordingBind { };
    static inline UINT RecordingResult{ 0 };
    static inline bool IsRecording{ false };

    static inline bool IsInitialized{ true }; // TODO -> to false!
};
