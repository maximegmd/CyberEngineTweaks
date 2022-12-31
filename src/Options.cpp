#include "stdafx.h"

#include "Paths.h"
#include "Utils.h"

void PatchesSettings::Load(const nlohmann::json& aConfig)
{
    RemovePedestrians = aConfig.value("remove_pedestrians", RemovePedestrians);
    SkipStartMenu = aConfig.value("skip_start_menu", SkipStartMenu);
    AmdSmt = aConfig.value("amd_smt", AmdSmt);
    AsyncCompute = aConfig.value("disable_async_compute", AsyncCompute);
    Antialiasing = aConfig.value("disable_antialiasing", Antialiasing);
    DisableIntroMovies = aConfig.value("disable_intro_movies", DisableIntroMovies);
    DisableVignette = aConfig.value("disable_vignette", DisableVignette);
    DisableBoundaryTeleport = aConfig.value("disable_boundary_teleport", DisableBoundaryTeleport);
    DisableWin7Vsync = aConfig.value("disable_win7_vsync", DisableWin7Vsync);
    MinimapFlicker = aConfig.value("minimap_flicker", MinimapFlicker);
}

nlohmann::json PatchesSettings::Save() const
{
    return {
        {"remove_pedestrians", RemovePedestrians},
        {"disable_async_compute", AsyncCompute},
        {"disable_antialiasing", Antialiasing},
        {"skip_start_menu", SkipStartMenu},
        {"amd_smt", AmdSmt},
        {"disable_intro_movies", DisableIntroMovies},
        {"disable_vignette", DisableVignette},
        {"disable_boundary_teleport", DisableBoundaryTeleport},
        {"disable_win7_vsync", DisableWin7Vsync},
        {"minimap_flicker", MinimapFlicker},
    };
}

void PatchesSettings::ResetToDefaults()
{
    *this = {};
}

void FontSettings::Load(const nlohmann::json& aConfig)
{
    Path = aConfig.value("path", Path);
    Language = aConfig.value("language", Language);
    BaseSize = aConfig.value("base_size", BaseSize);
    OversampleHorizontal = aConfig.value("oversample_horizontal", OversampleHorizontal);
    OversampleVertical = aConfig.value("oversample_vertical", OversampleVertical);
}

nlohmann::json FontSettings::Save() const
{
    return {{"path", Path}, {"language", Language}, {"base_size", BaseSize}, {"oversample_horizontal", OversampleHorizontal}, {"oversample_vertical", OversampleVertical}};
}

void FontSettings::ResetToDefaults()
{
    *this = {};
}

void DeveloperSettings::Load(const nlohmann::json& aConfig)
{
    RemoveDeadBindings = aConfig.value("remove_dead_bindings", RemoveDeadBindings);
    EnableImGuiAssertions = aConfig.value("enable_imgui_assertions", EnableImGuiAssertions);
    EnableDebug = aConfig.value("enable_debug", EnableDebug);
    DumpGameOptions = aConfig.value("dump_game_options", DumpGameOptions);
    MaxLinesConsoleHistory = aConfig.value("max_lines_console_history", MaxLinesConsoleHistory);
    PersistentConsole = aConfig.value("persistent_console", PersistentConsole);

    // set global "Enable ImGui Assertions"
    g_ImGuiAssertionsEnabled = EnableImGuiAssertions;
}

nlohmann::json DeveloperSettings::Save() const
{
    // set global "Enable ImGui Assertions"
    g_ImGuiAssertionsEnabled = EnableImGuiAssertions;

    return {{"remove_dead_bindings", RemoveDeadBindings}, {"enable_imgui_assertions", EnableImGuiAssertions},    {"enable_debug", EnableDebug},
            {"dump_game_options", DumpGameOptions},       {"max_lines_console_history", MaxLinesConsoleHistory}, {"persistent_console", PersistentConsole}};
}

void DeveloperSettings::ResetToDefaults()
{
    *this = {};

    // set global "Enable ImGui Assertions"
    g_ImGuiAssertionsEnabled = EnableImGuiAssertions;
}

void Options::Load()
{
    const auto path = GetAbsolutePath(m_paths.Config(), "", false);
    if (path.empty())
        return;

    std::ifstream configFile(path);
    if (!configFile)
        return;

    auto config = nlohmann::json::parse(configFile);

    // patches config
    const auto& patchesConfig = config["patches"];
    if (!patchesConfig.empty())
        Patches.Load(patchesConfig);

    // font config
    const auto& fontConfig = config["font"];
    if (!fontConfig.empty())
        Font.Load(fontConfig);

    // developer config
    const auto& developerConfig = config["developer"];
    if (!developerConfig.empty())
        Developer.Load(developerConfig);
}

void Options::Save() const
{
    nlohmann::json config = {{"patches", Patches.Save()}, {"font", Font.Save()}, {"developer", Developer.Save()}};

    const auto path = GetAbsolutePath(m_paths.Config(), "", true);
    std::ofstream o(path);
    o << config.dump(4) << std::endl;
}

void Options::ResetToDefaults()
{
    Patches.ResetToDefaults();
    Font.ResetToDefaults();
    Developer.ResetToDefaults();

    Save();
}

Options::Options(Paths& aPaths)
    : m_paths(aPaths)
{
    const auto* exePathStr = aPaths.Executable().native().c_str();
    const auto verInfoSz = GetFileVersionInfoSize(exePathStr, nullptr);
    if (verInfoSz)
    {
        const auto verInfo = std::make_unique<BYTE[]>(verInfoSz);
        if (GetFileVersionInfo(exePathStr, 0, verInfoSz, verInfo.get()))
        {
            struct
            {
                WORD Language;
                WORD CodePage;
            }* pTranslations;

            UINT transBytes = 0;
            if (VerQueryValue(verInfo.get(), _T("\\VarFileInfo\\Translation"), reinterpret_cast<void**>(&pTranslations), &transBytes))
            {
                UINT dummy;
                TCHAR* productName = nullptr;
                TCHAR subBlock[64];
                for (UINT i = 0; i < (transBytes / sizeof(*pTranslations)); i++)
                {
                    _stprintf(subBlock, _T("\\StringFileInfo\\%04x%04x\\ProductName"), pTranslations[i].Language, pTranslations[i].CodePage);
                    if (VerQueryValue(verInfo.get(), subBlock, reinterpret_cast<void**>(&productName), &dummy))
                        if (_tcscmp(productName, _T("Cyberpunk 2077")) == 0)
                        {
                            ExeValid = true;
                            break;
                        }
                }
            }
        }
    }
    // check if exe name matches in case previous check fails
    ExeValid = ExeValid || aPaths.Executable().filename() == "Cyberpunk2077.exe";

    if (!ExeValid)
        throw std::runtime_error("Not Cyberpunk2077.exe");

    set_default_logger(CreateLogger(GetAbsolutePath(L"cyber_engine_tweaks.log", m_paths.CETRoot(), true), "main", nullptr, "[%Y-%m-%d %H:%M:%S UTC%z] [%l] [%!] [%t] %v"));

    Log::Info("Cyber Engine Tweaks is starting...");

    GameImage.Initialize();

    if (GameImage.version)
    {
        Log::Info("CET version {} [{}]", CET_BUILD_COMMIT, CET_BUILD_BRANCH);
        auto [major, minor] = GameImage.GetVersion();
        Log::Info("Game version {}.{:02d}", major, minor);
        Log::Info("Root path: \"{}\"", UTF16ToUTF8(aPaths.GameRoot().native()));
        Log::Info("Cyber Engine Tweaks path: \"{}\"", UTF16ToUTF8(aPaths.CETRoot().native()));
        Log::Info("Lua scripts search path: \"{}\"", UTF16ToUTF8(aPaths.ModsRoot().native()));

        if (GameImage.GetVersion() != Image::GetSupportedVersion())
        {
            const auto [smajor, sminor] = Image::GetSupportedVersion();
            Log::Error("Unsupported game version! Only {}.{:02d} is supported.", smajor, sminor);
            throw std::runtime_error("Unsupported version");
        }
    }
    else
    {
        Log::Info("Unknown Game Version, update the mod");
        throw std::runtime_error("Unknown version");
    }

    Load();
    Save();
}
