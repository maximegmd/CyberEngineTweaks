#pragma once

#include "Utils.h"
struct I18n
{
    ~I18n() = default;

    void LoadLanguageFiles();
    std::vector<std::string> GetLocaleOptions() const { return m_localeOptions; }
    std::string GetFormatedLanguageName(const std::string& acLocale) const;
    tinygettext::Dictionary& GetDictionary();
    std::string Translate(const std::string& aMsgid) const;
    std::string Translate(const std::string& aMsgctxt, const std::string aMsgid) const;
    std::string Translate(const std::string& aMsgid, const std::string& aMsgidPlural, int aNum) const;
    std::string Translate(const std::string& aMsgctxt, const std::string aMsgid, const std::string& aMsgidPlural, int aNum) const;

    void SetLanguageBaseOnSystemLocale();
    void SetLanguage(std::string aLocale);
    void LoadLanguageSettings();

private:
    friend struct CET;
    I18n(Options& aOptions, Paths& aPaths, Fonts& aFonts);
    std::string GetSystemLocale() const;

    static void LogInfoCallback(const std::string& acString);
    static void LogWarnCallback(const std::string& acString);
    static void LogErrorCallback(const std::string& acString);
    void PopulateOptions();

    Options& m_options;
    Paths& m_paths;
    Fonts& m_fonts;

    const std::string m_defaultLocale{"en_US"};
    std::set<tinygettext::Language> m_languages;
    std::vector<std::string> m_localeOptions{"System"};
    tinygettext::DictionaryManager m_dictManager;
    tinygettext::Dictionary* m_dict;
};