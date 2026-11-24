#pragma once

#include <string>
#include <string_view>

namespace TextProcessor
{
/**
 * @brief Processes text for rendering (e.g. script shaping and bidirectional reordering).
 *
 * @param text The input text string to process.
 * @param out Output string to store transformed text. Reuses existing capacity.
 *            Must not be modified if no transformation is performed.
 * @return true if text was transformed and written to out; false if unchanged.
 */
bool Process(std::string_view text, std::string& out);

/**
 * @brief Convenience wrapper returning processed text or a copy of original text.
 *
 * @param text The input text string to process.
 * @return Transformed string if processing occurred, otherwise copy of input.
 */
std::string Process(std::string_view text);
} // namespace TextProcessor
