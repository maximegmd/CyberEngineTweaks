#include <stdafx.h>

#include "I18n.h"

#include <Utils.h>

Language::Language(const tinygettext::Language& aLanguage, const std::string& acLocalizedName)
    : m_locale(aLanguage.str())
    , m_name(aLanguage.get_name())
    , m_localizedName(acLocalizedName)
{
    FormatName();
}
Language::Language(const std::string& acLocale, const std::string& acName, const std::string& acLocalizedName)
    : m_locale(acLocale)
    , m_name(acName)
    , m_localizedName(acLocalizedName)
{
    FormatName();
}
std::string Language::GetLocale() const
{
    return m_locale;
}
std::string Language::GetName() const
{
    return m_name;
}
std::string Language::GetLocalizedName() const
{
    return m_localizedName;
}
std::string Language::GetFormatedName() const
{
    return m_formatedName;
}

void Language::FormatName()
{
    if (m_name.empty())
        m_name = m_locale;
    if (m_localizedName.empty())
        m_localizedName = m_name;
    if (m_localizedName == m_name)
        m_formatedName = m_name;
    else
        m_formatedName = std::format("{}\n{}", m_name, m_localizedName);
}

// Load all po files
void I18n::LoadLanguageFiles()
{
    m_languages.clear();
    m_languages.emplace_back(Language("System"));
    m_dictManager.add_directory(UTF16ToUTF8(m_paths.Languages().native()));
    auto languages = m_dictManager.get_languages();
    for (const auto& language : languages)
    {
        m_dictManager.set_language(language);
        // Translators: Type in your language name in it's own language. (e.g. French -> Français)
        auto localizedName = m_dictManager.get_dictionary().translate("Localized Language Name");
        if (localizedName.empty() || localizedName == "Localized Language Name")
            localizedName = language.get_localized_name();
        m_languages.emplace_back(Language(language, localizedName));
    }
}

// Get the list of language loaded.
std::vector<Language> I18n::GetLanguages() const
{
    return m_languages;
}

// Get language from given locale
Language I18n::GetLanguage(const std::string& acLocale) const
{
    auto languageIterator = std::find_if(m_languages.begin(), m_languages.end(), [acLocale](const Language& language) { return language.GetLocale() == acLocale; });
    if (languageIterator == m_languages.end())
        return Language("");
    else
        return *languageIterator;
}

// Basic translate
std::string I18n::Translate(const std::string& aMsgid) const
{
    return m_dict->translate(aMsgid);
}

// Translate with context id
std::string I18n::Translate(const std::string& aMsgctxt, const std::string aMsgid) const
{
    return m_dict->translate_ctxt(aMsgctxt, aMsgid);
}

// Translate with plural forms
std::string I18n::Translate(const std::string& aMsgid, const std::string& aMsgidPlural, int aNum) const
{
    return m_dict->translate_plural(aMsgid, aMsgidPlural, aNum);
}

// Translate with context id and plural forms
std::string I18n::Translate(const std::string& aMsgctxt, const std::string aMsgid, const std::string& aMsgidPlural, int aNum) const
{
    return m_dict->translate_ctxt_plural(aMsgctxt, aMsgid, aMsgidPlural, aNum);
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
    auto localeUTF8 = UTF16ToUTF8(localeName);
    std::replace(localeUTF8.begin(), localeUTF8.end(), '-', '_');
    return localeUTF8;
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

// Set language base on given locale string
void I18n::SetLanguage(const std::string& acLocale)
{
    const auto& language = GetLanguage(acLocale);
    if (language.GetLocale() == "")
    {
        SetLanguageBaseOnSystemLocale();
        m_options.Language.Locale = "System";
        m_options.Save();
        Log::Error("Translation for {} not found, using system locale instead.", acLocale, m_defaultLocale);
    }
    else
    {
        m_dictManager.set_language(tinygettext::Language::from_env(language.GetLocale()));
        m_dict = &m_dictManager.get_dictionary();
    }
}

// Set language closest to the system display locale
void I18n::SetLanguageBaseOnSystemLocale()
{
    auto systemLanguage = tinygettext::Language::from_env(GetSystemLocale());
    int highScore = 0;
    auto closestMatch = tinygettext::Language::from_env(m_defaultLocale);
    for (const auto& language : m_dictManager.get_languages())
    {
        int score = tinygettext::Language::match(systemLanguage, language);
        if (score > highScore)
        {
            closestMatch = language;
            highScore = score;
        }
    }
    Log::Info("Using system language setting. System Locale: {}, setting CET language to {}.", systemLanguage.str(), closestMatch.str());
    m_dictManager.set_language(closestMatch);
    m_dict = &m_dictManager.get_dictionary();
}

// Info log callback function for tinygettext
void I18n::LogInfoCallback(const std::string& acString)
{
    Log::Info((!acString.empty() && acString.back() == '\n') ? acString.substr(0, acString.size() - 1) : acString);
}

// Warning log callback function for tinygettext
void I18n::LogWarnCallback(const std::string& acString)
{
    Log::Warn((!acString.empty() && acString.back() == '\n') ? acString.substr(0, acString.size() - 1) : acString);
}

// Error log callback function for tinygettext
void I18n::LogErrorCallback(const std::string& acString)
{
    Log::Error((!acString.empty() && acString.back() == '\n') ? acString.substr(0, acString.size() - 1) : acString);
}

I18n::I18n(Options& aOptions, Paths& aPaths, Fonts& aFonts)
    : m_options(aOptions)
    , m_paths(aPaths)
    , m_fonts(aFonts)
{
    if (m_options.Developer.EnableI18nLog)
        tinygettext::Log::set_log_info_callback(&LogInfoCallback);
    else
        tinygettext::Log::set_log_info_callback(NULL);
    tinygettext::Log::set_log_warning_callback(&LogWarnCallback);
    tinygettext::Log::set_log_error_callback(&LogErrorCallback);
    LoadLanguageFiles();
    LoadLanguageSettings();
}