#pragma once

struct I18n {
  ~I18n() = default;

  void LoadLanguages(const std::filesystem::path& aLanguageDir);
  void SetLanguage(const tinygettext::Language& aLanguage);
  tinygettext::Dictionary& GetDictionary();
  const char* Translate(const std::string& aMsgid);
  const char* TranslatePlural(const std::string& aMsgid, const std::string& aMsgidPlural, int aNum);
  const char* TranslateWithContext(const std::string& aMsgctxt, const std::string aMsgid);
  const char* TranslatePluralWithContext(const std::string& aMsgctxt, const std::string aMsgid, const std::string& aMsgidPlural, int aNum);

private:
  friend struct CET;
  I18n(Options& aOptions, Paths& aPaths);

  Options& m_options;
  Paths& m_paths;

  tinygettext::DictionaryManager* m_dict_manager;
  tinygettext::Dictionary* m_dict;
};