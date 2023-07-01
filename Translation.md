# Translation

Instructions on how to create and update the language files for CET.

## For translators

The language files have the extension name of ".po". The file names are generally using [two-letter language codes](https://en.wikipedia.org/wiki/List_of_ISO_639-1_codes) (e.g. en, fr, de). For languages that have different writing systems or to differentiate by country, the file name needs to be a locale with an underscore (e.g. zh_CN, zh_TW).

### Use Crowdin

Todo


### Use commandline scripts:

### Create a new language file
To create a new language, use the following command:
```sh
xmake i18n-po create filename
```
This will generate a new po file under the `languages/` directory. Make sure the filename follows the rules above. Otherwise, the language name won't be parsed by CET correctly.

### Update the language files
To update those files, use the following command:
```sh
xmake i18n-po update
```
This will add new translatable strings from `template.pot` file to all `.po` files.

### Edit the language files
To edit the language files, you can either edit them on Crowdin or open them in a text editor or a PO editor.

### Test the translations
You can reload the translations from `Settings->CET Development Settings->Reload translation`.

If you are editing the language files from the project directory. Use the following command to copy the language files to your game directory:
```sh
xmake i18n-install
```

## Format

The language files are using the GNU gettext PO format. Detailed documentation can be found [here](https://www.gnu.org/software/gettext/manual/html_node/PO-Files.html).
```po
white-space
#  translator-comments
#. extracted-comments
#: reference…
#, flag…
#| msgid previous-untranslated-string
msgid untranslated-string
msgstr translated-string
```
## For developers

There are two macros you can use the mark the translatable strings.

To translate normal strings, use:
```cpp
_t("This string will be translated.");
```

To translate strings with plural forms:
```cpp
_t("There is %d apple.", "There are %d apples.", number);
```

If you have strings inside a container that you want to translate. You can use the `_noop("string")` to mark it for translation. For example:
```cpp
std::vector<std::string> fruits = {
    _noop("apple"),
    _noop("banana"),
    _noop("orange")
};

for (const auto& fruit : list)
{
    Log::Info(_t(fruit));
}
```

Always remember to update the `template.pot` file before you commit the changes you made to the translatable strings. So translators can start to work on updating the translations.

### Update the template file

The template file `template.pot` is auto-generated from the source files. It includes all the translatable strings in the source files.

To update this file, use the following command:
```sh
xmake i18n-pot
```
This will also auto-update the `en.po` file. `en.po` is always auto-generated from the template file, so there's no need to edit it.