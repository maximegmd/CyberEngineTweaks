#include "ArabicText.h"

#include <fribidi.h>

#include <vector>
#include <string>

namespace ArabicText
{

static bool ContainsArabic(std::string_view text)
{
    const unsigned char* s = reinterpret_cast<const unsigned char*>(text.data());

    const unsigned char* end = s + text.size();

    while (s < end)
    {
        uint32_t cp = 0;

        if ((*s & 0x80) == 0)
        {
            ++s;
            continue;
        }
        else if ((*s & 0xE0) == 0xC0)
        {
            if (s + 1 >= end)
                break;

            cp = ((*s & 0x1F) << 6) | (s[1] & 0x3F);

            s += 2;
        }
        else if ((*s & 0xF0) == 0xE0)
        {
            if (s + 2 >= end)
                break;

            cp = ((*s & 0x0F) << 12) | ((s[1] & 0x3F) << 6) | (s[2] & 0x3F);

            s += 3;
        }
        else if ((*s & 0xF8) == 0xF0)
        {
            if (s + 3 >= end)
                break;

            cp = ((*s & 0x07) << 18) | ((s[1] & 0x3F) << 12) | ((s[2] & 0x3F) << 6) | (s[3] & 0x3F);

            s += 4;
        }
        else
        {
            ++s;
            continue;
        }

        if ((cp >= 0x0600 && cp <= 0x06FF) || (cp >= 0x0750 && cp <= 0x077F) || (cp >= 0x08A0 && cp <= 0x08FF) || (cp >= 0xFB50 && cp <= 0xFDFF) || (cp >= 0xFE70 && cp <= 0xFEFF))
        {
            return true;
        }
    }

    return false;
}

thread_local std::vector<FriBidiChar> unicodeBuffer;
thread_local std::vector<FriBidiCharType> bidiTypes;
thread_local std::vector<FriBidiArabicProp> arabicProps;
thread_local std::vector<FriBidiLevel> levels;
static std::pair<std::string_view, std::string_view> SplitImGuiLabel(std::string_view text)
{
    size_t pos = text.find("###");
    if (pos != std::string_view::npos)
        return {text.substr(0, pos), text.substr(pos)};

    pos = text.find("##");
    if (pos != std::string_view::npos)
        return {text.substr(0, pos), text.substr(pos)};

    return {text, {}};
}
std::string Process(std::string_view text)
{
    if (text.empty())
        return {};

    auto [visibleText, imguiId] = SplitImGuiLabel(text);

    // نستخدم الجزء الظاهر فقط في المعالجة
    text = visibleText;

    if (!ContainsArabic(text))
    {
        if (imguiId.empty())
            return std::string(text);

        std::string result(text);
        result += imguiId;
        return result;
    }

    unicodeBuffer.resize(text.size() + 8);

    FriBidiStrIndex length = fribidi_charset_to_unicode(FRIBIDI_CHAR_SET_UTF8, text.data(), (FriBidiStrIndex)text.size(), unicodeBuffer.data());

    if (length <= 0)
        return std::string(text);

    unicodeBuffer.resize(length);

    bidiTypes.resize(length);
    arabicProps.resize(length);
    levels.resize(length);
    fribidi_get_bidi_types(unicodeBuffer.data(), length, bidiTypes.data());

    FriBidiParType paragraphDirection = FRIBIDI_PAR_LTR;

    paragraphDirection = fribidi_get_par_direction(bidiTypes.data(), length);

    FriBidiLevel maxLevel = fribidi_get_par_embedding_levels(bidiTypes.data(), length, &paragraphDirection, levels.data());

    (void)maxLevel;

    fribidi_get_joining_types(unicodeBuffer.data(), length, arabicProps.data());

    fribidi_join_arabic(bidiTypes.data(), length, levels.data(), arabicProps.data());

#if FRIBIDI_MAJOR_VERSION >= 1
    fribidi_shape(FRIBIDI_FLAGS_DEFAULT | FRIBIDI_FLAGS_ARABIC, levels.data(), length, arabicProps.data(), unicodeBuffer.data());
#else
    fribidi_shape_arabic(FRIBIDI_FLAGS_DEFAULT | FRIBIDI_FLAGS_ARABIC, levels.data(), length, arabicProps.data(), unicodeBuffer.data());
#endif
    FriBidiStrIndex lineStart = 0;

    for (FriBidiStrIndex i = 0; i < length; ++i)
    {
        bool lineEnd = unicodeBuffer[i] == '\n' || unicodeBuffer[i] == '\r' || unicodeBuffer[i] == '\f' || unicodeBuffer[i] == '\v' || i == length - 1;

        if (!lineEnd)
            continue;

        fribidi_reorder_line(
            FRIBIDI_FLAGS_DEFAULT | FRIBIDI_FLAGS_ARABIC, bidiTypes.data(), i - lineStart + 1, lineStart, paragraphDirection, levels.data(), unicodeBuffer.data(), nullptr);

        lineStart = i + 1;
    }
    // Remove FriBidi filler characters.
    FriBidiStrIndex outLength = 0;

    for (FriBidiStrIndex i = 0; i < length; ++i)
    {
        if (unicodeBuffer[i] != FRIBIDI_CHAR_FILL)
        {
            unicodeBuffer[outLength++] = unicodeBuffer[i];
        }
    }

    std::string result;
    result.resize(static_cast<size_t>(outLength) * 4 + 1);

    FriBidiStrIndex utf8Length = fribidi_unicode_to_charset(FRIBIDI_CHAR_SET_UTF8, unicodeBuffer.data(), outLength, result.data());

    if (utf8Length < 0)
    {
        std::string fallback(text);

        if (!imguiId.empty())
            fallback += imguiId;

        return fallback;
    }

   result.resize(static_cast<size_t>(utf8Length));

    if (!imguiId.empty())
        result += imguiId;

    return result;
}

} // namespace ArabicText
