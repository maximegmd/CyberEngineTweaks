#include "stdafx.h"

using TInitScriptMemberVariable = void*(void* a1, void* a2, uint64_t a3, uint64_t nameHash, void* a5, void* a6, void* a7);
TInitScriptMemberVariable* RealInitScriptMemberVariable = nullptr;

void* HookInitScriptMemberVariable(void* a1, void* a2, uint64_t a3, uint64_t nameHash, void* a5, void* a6, void* a7)
{
    // Break the nameHash of some SplashScreenLoadingScreenLogicController variables
    // Should prevent the intro screen scripts from finding the intro bink, which makes it show a loading screen instead
    // (intro movie audio still plays though - this can be stopped by disabling more script vars, but unfortunately
    // that'll also make it load infinitely) For me the loading screen takes almost as much time as the intro movie
    // itself did, but the audio shows that a few seconds are saved with this, maybe faster machines can save even more
    // time.

    // Ideally I think the real solution is to change GameFramework/InitialState INI variable from "Initialization" to
    // "PreGameSession" or "MainMenu" instead Unfortunately that causes a black screen on launch though, likely only
    // works properly on non-shipping builds

    if (nameHash == RED4ext::FNV1a64("logoTrainWBBink") || nameHash == RED4ext::FNV1a64("logoTrainNamcoBink") || nameHash == RED4ext::FNV1a64("logoTrainStadiaBink") ||
        nameHash == RED4ext::FNV1a64("logoTrainNoRTXBink") || nameHash == RED4ext::FNV1a64("logoTrainRTXBink") || nameHash == RED4ext::FNV1a64("introMessageBink"))
    {
        nameHash = ~nameHash;
    }

    return RealInitScriptMemberVariable(a1, a2, a3, nameHash, a5, a6, a7);
}

void DisableIntroMoviesPatch()
{
    const RED4ext::RelocPtr<void> func(CyberEngineTweaks::Addresses::CPatches_IntroMovie);
    RealInitScriptMemberVariable = reinterpret_cast<TInitScriptMemberVariable*>(func.GetAddr());

    if (RealInitScriptMemberVariable == nullptr)
    {
        Log::Warn("Disable intro movies patch: failed, could not be found");
        return;
    }

    MH_CreateHook(
        reinterpret_cast<void*>(RealInitScriptMemberVariable), reinterpret_cast<void*>(&HookInitScriptMemberVariable), reinterpret_cast<void**>(&RealInitScriptMemberVariable));
    Log::Info("Disable intro movies patch: success");
}
