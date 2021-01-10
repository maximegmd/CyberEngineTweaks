#pragma once

using TVKBindCallback = void();

struct VKBindings
{
    static void Initialize();
    
    static void Load();
    static void Save();
    
    static bool Bind(UINT aVKCodeBind, TVKBindCallback* aHandler);
    static bool IsBound(UINT aVKCodeBind);

    static bool StartRecordingBind(TVKBindCallback* aHandler);
    static void RecordKeyDown(UINT aVKCode);
    static void RecordKeyUp(UINT aVKCode);
    static bool IsRecordingBind();

private:
    static UINT CreateVKCodeBindFromRecording();
    
    static inline std::map<UINT, TVKBindCallback*> Binds{ };

    static inline std::array<UINT, 4> Recording{ };
    static inline size_t RecordingLength{ 0 };
    static inline TVKBindCallback* RecordingHandler { nullptr };
    static inline bool IsRecording{ false };
};
