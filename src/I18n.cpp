#include <stdafx.h>

#include <tinygettext/unix_file_system.hpp>

#include "I18n.h"

void I18n::LoadLanguages(const std::filesystem::path &aLanguageDir)
{
  m_dict_manager->add_directory(aLanguageDir.string());
}

void I18n::SetLanguage(const tinygettext::Language &aLanguage)
{
  m_dict_manager->set_language(aLanguage);
}

tinygettext::Dictionary &I18n::GetDictionary()
{
  return m_dict_manager->get_dictionary();
}

const char* I18n::Translate(const std::string& aMsgid)
{
  return m_dict->translate(aMsgid).c_str();
}

const char* I18n::TranslatePlural(const std::string& aMsgid, const std::string& aMsgidPlural, int aNum)
{
  return m_dict->translate_plural(aMsgid, aMsgidPlural, aNum).c_str();
}

const char* I18n::TranslateWithContext(const std::string& aMsgctxt, const std::string aMsgid)
{
  return m_dict->translate_ctxt(aMsgctxt, aMsgid).c_str();
}

const char* I18n::TranslatePluralWithContext(const std::string& aMsgctxt, const std::string aMsgid, const std::string& aMsgidPlural, int aNum)
{
  return m_dict->translate_ctxt_plural(aMsgctxt, aMsgid, aMsgidPlural, aNum).c_str();
}

I18n::I18n(Options& aOptions, Paths& aPaths)
    : m_options(aOptions)
    , m_paths(aPaths)
{
  tinygettext::DictionaryManager dict_manager(std::unique_ptr<tinygettext::FileSystem>(new tinygettext::UnixFileSystem));

  m_dict_manager = &dict_manager;
  LoadLanguages(m_paths.Languages());
  auto languages = m_dict_manager->get_languages();
  if (languages.size() < 1)
  {
    std::cout << "No languages loaded"
              << "\n";
    return;
  }
  std::cout << "Number of languages: " << languages.size() << std::endl;
  for (std::set<tinygettext::Language>::const_iterator it = languages.begin(); it != languages.end(); ++it)
  {
    const tinygettext::Language &language = *it;
    std::cout << "Env:       " << language.str() << std::endl
              << "Name:      " << language.get_name() << std::endl
              << "Language:  " << language.get_language() << std::endl
              << "Country:   " << language.get_country() << std::endl
              << "Modifier:  " << language.get_modifier() << std::endl
              << std::endl;
    if (language.get_language() == m_options.Language.Locale)
    {
      SetLanguage(language);
      break;
    }
    m_dict = &m_dict_manager->get_dictionary();
  }
}
