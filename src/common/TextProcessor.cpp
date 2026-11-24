#include "TextProcessor.h"

#include <fribidi.h>

#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace TextProcessor
{

namespace
{
thread_local std::vector<FriBidiChar> s_unicodeBuffer;
thread_local std::vector<FriBidiCharType> s_bidiTypes;
thread_local std::vector<FriBidiArabicProp> s_arabicProps;
thread_local std::vector<FriBidiLevel> s_levels;
#if FRIBIDI_MAJOR_VERSION >= 1
thread_local std::vector<FriBidiBracketType> s_bracketTypes;
#endif
thread_local std::vector<char> s_utf8Buffer;

// Fast ASCII check to bypass Unicode decoding when all characters are ASCII
bool IsAllAscii(std::string_view text) noexcept
{
    for (const char c : text)
    {
        if (static_cast<unsigned char>(c) >= 0x80)
            return false;
    }
    return true;
}

// Split Dear ImGui label at the first occurrence of "##" (matching ImGui::FindRenderedTextEnd)
std::pair<std::string_view, std::string_view> SplitImGuiLabel(std::string_view text) noexcept
{
    const size_t pos = text.find("##");
    if (pos != std::string_view::npos)
        return {text.substr(0, pos), text.substr(pos)};

    return {text, {}};
}

// Check whether the UTF-8 text contains Arabic codepoints requiring shaping/BiDi
bool ContainsArabic(std::string_view text) noexcept
{
    const auto* s = reinterpret_cast<const unsigned char*>(text.data());
    const auto* end = s + text.size();

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

        if ((cp >= 0x0600 && cp <= 0x06FF) || (cp >= 0x0750 && cp <= 0x077F) ||
            (cp >= 0x08A0 && cp <= 0x08FF) || (cp >= 0xFB50 && cp <= 0xFDFF) ||
            (cp >= 0xFE70 && cp <= 0xFEFF))
        {
            return true;
        }
    }

    return false;
}

// Handle Arabic shaping and bidirectional reordering using FriBidi
bool ProcessArabicBiDi(std::string_view visibleText, std::string_view idSuffix, std::string& out)
{
    s_unicodeBuffer.resize(visibleText.size() + 1);

    const FriBidiStrIndex length = fribidi_charset_to_unicode(
        FRIBIDI_CHAR_SET_UTF8,
        visibleText.data(),
        static_cast<FriBidiStrIndex>(visibleText.size()),
        s_unicodeBuffer.data());

    if (length <= 0)
        return false;

    s_unicodeBuffer.resize(length);
    s_bidiTypes.resize(length);
    s_levels.resize(length);

    fribidi_get_bidi_types(s_unicodeBuffer.data(), length, s_bidiTypes.data());

    FriBidiParType paragraphDirection = fribidi_get_par_direction(s_bidiTypes.data(), length);

#if FRIBIDI_MAJOR_VERSION >= 1
    s_bracketTypes.resize(length);
    fribidi_get_bracket_types(s_unicodeBuffer.data(), length, s_bidiTypes.data(), s_bracketTypes.data());
    const FriBidiLevel maxLevel = fribidi_get_par_embedding_levels_ex(
        s_bidiTypes.data(), s_bracketTypes.data(), length, &paragraphDirection, s_levels.data());
#else
    const FriBidiLevel maxLevel = fribidi_get_par_embedding_levels(
        s_bidiTypes.data(), length, &paragraphDirection, s_levels.data());
#endif

    if (maxLevel == 0)
        return false;

    s_arabicProps.resize(length);
    fribidi_get_joining_types(s_unicodeBuffer.data(), length, s_arabicProps.data());
    fribidi_join_arabic(s_bidiTypes.data(), length, s_levels.data(), s_arabicProps.data());

#if FRIBIDI_MAJOR_VERSION >= 1
    fribidi_shape(FRIBIDI_FLAGS_DEFAULT | FRIBIDI_FLAGS_ARABIC, s_levels.data(), length, s_arabicProps.data(), s_unicodeBuffer.data());
#else
    fribidi_shape_arabic(FRIBIDI_FLAGS_DEFAULT | FRIBIDI_FLAGS_ARABIC, s_levels.data(), length, s_arabicProps.data(), s_unicodeBuffer.data());
#endif

    // Reorder each line individually, excluding line breaks from reordering
    FriBidiStrIndex lineStart = 0;
    while (lineStart < length)
    {
        FriBidiStrIndex lineEnd = lineStart;
        while (lineEnd < length)
        {
            const FriBidiChar ch = s_unicodeBuffer[lineEnd];
            if (ch == '\n' || ch == '\r' || ch == '\f' || ch == '\v')
                break;
            ++lineEnd;
        }

        const FriBidiStrIndex lineLength = lineEnd - lineStart;
        if (lineLength > 0)
        {
            FriBidiParType lineDirection = fribidi_get_par_direction(s_bidiTypes.data() + lineStart, lineLength);
            if (lineDirection == FRIBIDI_PAR_ON)
                lineDirection = paragraphDirection;

            fribidi_reorder_line(
                FRIBIDI_FLAGS_DEFAULT | FRIBIDI_FLAGS_ARABIC,
                s_bidiTypes.data(),
                lineLength,
                lineStart,
                lineDirection,
                s_levels.data(),
                s_unicodeBuffer.data(),
                nullptr);
        }

        lineStart = lineEnd;
        if (lineStart < length)
        {
            // Skip the line-break character so it remains untouched at this position
            ++lineStart;
        }
    }

    // Remove FriBidi filler characters
    FriBidiStrIndex outLength = 0;
    for (FriBidiStrIndex i = 0; i < length; ++i)
    {
        if (s_unicodeBuffer[i] != FRIBIDI_CHAR_FILL)
        {
            s_unicodeBuffer[outLength++] = s_unicodeBuffer[i];
        }
    }

    s_utf8Buffer.resize(static_cast<size_t>(outLength) * 4 + 1);
    const FriBidiStrIndex utf8Length = fribidi_unicode_to_charset(
        FRIBIDI_CHAR_SET_UTF8, s_unicodeBuffer.data(), outLength, s_utf8Buffer.data());

    if (utf8Length < 0)
        return false;

    out.assign(s_utf8Buffer.data(), static_cast<size_t>(utf8Length));
    if (!idSuffix.empty())
        out.append(idSuffix.data(), idSuffix.size());

    return true;
}

} // namespace

bool Process(std::string_view text, std::string& out)
{
    // Fast-path: empty text or ASCII-only text requires no transformation
    if (text.empty() || IsAllAscii(text))
        return false;

    // Split ImGui ID to avoid processing non-visible identifier labels
    const auto [visibleText, idSuffix] = SplitImGuiLabel(text);
    if (visibleText.empty() || IsAllAscii(visibleText))
        return false;

    // Script transformation pipeline:
    // 1. Arabic shaping and bidirectional reordering
    if (ContainsArabic(visibleText))
    {
        return ProcessArabicBiDi(visibleText, idSuffix, out);
    }

    return false;
}

std::string Process(std::string_view text)
{
    std::string result;
    if (Process(text, result))
        return result;

    return std::string(text);
}

} // namespace TextProcessor
