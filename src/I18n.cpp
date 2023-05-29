#include <stdafx.h>

#include "I18n.h"

#include <Utils.h>

void I18n::PopulateOptions()
{
    for (const auto& language : m_languages)
    {
        m_dictManager.set_language(language);
        // Translators: Translate this to your language's native name. (e.g. French -> Français)
        const std::string nativeName = m_dictManager.get_dictionary().translate("Native Language Name");
        std::string name = language.get_name();
        std::string formatedName;
        if (name.empty())
            name = language.str();
        if (nativeName == "Native Language Name" || nativeName == name)
            formatedName = name;
        else
            formatedName = std::format("{}\n{}", name, nativeName);
        m_localeOptions.emplace_back(formatedName);
    }
}

void I18n::LoadLanguageFiles()
{
    m_dictManager.add_directory(UTF16ToUTF8(m_paths.Languages().native()));
    m_languages = m_dictManager.get_languages();
}

tinygettext::Dictionary& I18n::GetDictionary()
{
    return m_dictManager.get_dictionary();
}

std::string I18n::Translate(const std::string& aMsgid) const
{
    return m_dict->translate(aMsgid);
}

std::string I18n::Translate(const std::string& aMsgctxt, const std::string aMsgid) const
{
    return m_dict->translate_ctxt(aMsgctxt, aMsgid);
}

std::string I18n::Translate(const std::string& aMsgid, const std::string& aMsgidPlural, int aNum) const
{
    return m_dict->translate_plural(aMsgid, aMsgidPlural, aNum);
}

std::string I18n::Translate(const std::string& aMsgctxt, const std::string aMsgid, const std::string& aMsgidPlural, int aNum) const
{
    return m_dict->translate_ctxt_plural(aMsgctxt, aMsgid, aMsgidPlural, aNum);
}

// Set language closest to the system display locale
void I18n::SetLanguageBaseOnSystemLocale()
{
    auto systemLanguage = tinygettext::Language::from_env(GetSystemLocale());
    int highScore = 0;
    auto closestMatch = tinygettext::Language::from_env(m_defaultLocale);
    for (const auto& language : m_languages)
    {
        int score = language.match(systemLanguage, language);
        if (score > highScore)
        {
            closestMatch = language;
            highScore = score;
        }
    }

    m_dictManager.set_language(closestMatch);
    m_dict = &m_dictManager.get_dictionary();
}

// Set language base on given locale string
void I18n::SetLanguage(std::string aLocale)
{
    auto language = tinygettext::Language::from_env(aLocale);
    if (m_languages.find(language) == m_languages.end())
    {
        SetLanguageBaseOnSystemLocale();
        m_options.Language.Locale = "System";
        m_options.Save();
        Log::Error("Translation for {} not found, using system locale instead.", aLocale, m_defaultLocale);
    }

    m_dictManager.set_language(language);
    m_dict = &m_dictManager.get_dictionary();
}

// Load setting from the config and set the language
void I18n::LoadLanguageSettings()
{
    const auto& locale = m_options.Language.Locale;
    if (locale == "System")
        SetLanguageBaseOnSystemLocale();
    else
        SetLanguage(locale);
}

// Get system display locale
std::string I18n::GetSystemLocale() const
{
    LCID localeId = GetUserDefaultLCID(); // Get the user default locale identifier
    WCHAR localeName[LOCALE_NAME_MAX_LENGTH];
    int result = GetLocaleInfoEx(LOCALE_NAME_USER_DEFAULT, LOCALE_SNAME, localeName, LOCALE_NAME_MAX_LENGTH); // Get the locale name using locale identifier

    if (result == 0)
    {
        Log::Error("Error when getting system display locale: {}", GetLastError());
        return "en_US";
    }

    return UTF16ToUTF8(localeName);
}

void I18n::LogInfoCallback(const std::string& acString)
{
    Log::Info((!acString.empty() && acString.back() == '\n') ? acString.substr(0, acString.size() - 1) : acString);
}
void I18n::LogWarnCallback(const std::string& acString)
{
    Log::Warn((!acString.empty() && acString.back() == '\n') ? acString.substr(0, acString.size() - 1) : acString);
}
void I18n::LogErrorCallback(const std::string& acString)
{
    Log::Error((!acString.empty() && acString.back() == '\n') ? acString.substr(0, acString.size() - 1) : acString);
}

I18n::I18n(Options& aOptions, Paths& aPaths, Fonts& aFonts)
    : m_options(aOptions)
    , m_paths(aPaths)
    , m_fonts(aFonts)
{
    tinygettext::Log::set_log_info_callback(&LogInfoCallback);
    tinygettext::Log::set_log_warning_callback(&LogWarnCallback);
    tinygettext::Log::set_log_error_callback(&LogErrorCallback);
    LoadLanguageFiles();
    PopulateOptions();
    LoadLanguageSettings();
}