#pragma once

#include "Utils.h"

struct Language
{
    Language(const tinygettext::Language& aLanguage, const std::string& acLocalizedName);
    Language(const std::string& acLocale, const std::string& acName = "", const std::string& acLocalizedName = "");
    std::string GetLocale() const;
    std::string GetName() const;
    std::string GetLocalizedName() const;
    std::string GetFormatedName() const;

private:
    void FormatName();
    std::string m_locale{};
    std::string m_name{};
    std::string m_formatedName{};
    std::string m_localizedName{};
};

struct I18n
{
    ~I18n() = default;

    void LoadLanguageSettings();
    std::vector<Language> GetLanguages() const;
    Language GetLanguage(const std::string& acLocale) const;
    std::string GetSystemLocale() const;
    std::string GetCurrentLocale() const;
    void Reload();

    std::string Translate(const std::string& aMsgid) const;
    std::string Translate(const std::string& aMsgctxt, const std::string aMsgid) const;
    std::string Translate(const std::string& aMsgid, const std::string& aMsgidPlural, int aNum) const;
    std::string Translate(const std::string& aMsgctxt, const std::string aMsgid, const std::string& aMsgidPlural, int aNum) const;

private:
    friend struct CET;
    I18n(Options& aOptions, Paths& aPaths, Fonts& aFonts);
    void FetchSystemLocale();
    void LoadLanguageFiles();
    void SetLanguage(const std::string& aLocale);
    void SetLanguageBaseOnSystemLocale();

    static void LogInfoCallback(const std::string& acString);
    static void LogWarnCallback(const std::string& acString);
    static void LogErrorCallback(const std::string& acString);

    Options& m_options;
    Paths& m_paths;
    Fonts& m_fonts;

    const std::string m_defaultLocale{"en"};
    std::string m_systemLocale{};
    std::vector<Language> m_languages;
    tinygettext::DictionaryManager m_dictManager;
    tinygettext::Dictionary* m_dict;
};