/*
    Copyright 2018 Brick

    Permission is hereby granted, free of charge, to any person obtaining a copy of this software
    and associated documentation files (the "Software"), to deal in the Software without restriction,
    including without limitation the rights to use, copy, modify, merge, publish, distribute,
    sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all copies or
    substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
    BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
    DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef MEM_BOYER_MOORE_SCANNER_BRICK_H
#define MEM_BOYER_MOORE_SCANNER_BRICK_H

#include "pattern.h"

namespace mem
{
    class boyer_moore_scanner : public scanner_base<boyer_moore_scanner>
    {
    private:
        const pattern* pattern_ {nullptr};

        // Boyer–Moore + Boyer–Moore–Horspool Implementation
        std::vector<std::size_t> bc_skips_ {};
        std::vector<std::size_t> gs_skips_ {};

        std::size_t skip_pos_ {SIZE_MAX};

        std::size_t get_longest_run(std::size_t& length) const;

        bool is_prefix(std::size_t pos) const;
        std::size_t get_suffix_length(std::size_t pos) const;

    public:
        boyer_moore_scanner() = default;

        boyer_moore_scanner(const pattern& pattern);
        boyer_moore_scanner(const pattern& pattern, std::size_t min_bc_skip, std::size_t min_gs_skip);

        pointer scan(region range) const;
    };

    static constexpr const std::size_t default_min_bc_skip {5};
    static constexpr const std::size_t default_min_gs_skip {25};

    inline boyer_moore_scanner::boyer_moore_scanner(const pattern& _pattern)
        : boyer_moore_scanner(_pattern, default_min_bc_skip, default_min_gs_skip)
    {}

    inline boyer_moore_scanner::boyer_moore_scanner(
        const pattern& _pattern, std::size_t min_bc_skip, std::size_t min_gs_skip)
        : pattern_(&_pattern)
    {
        std::size_t max_skip = 0;
        std::size_t skip_pos = get_longest_run(max_skip);

        const byte* const bytes = pattern_->bytes();
        const std::size_t trimmed_size = pattern_->trimmed_size();

        if ((min_bc_skip > 0) && (max_skip >= min_bc_skip))
        {
            bc_skips_.resize(256, max_skip);
            skip_pos_ = skip_pos + max_skip - 1;

            for (std::size_t i = skip_pos, last = skip_pos + max_skip - 1; i < last; ++i)
                bc_skips_[bytes[i]] = last - i;

            if ((skip_pos == 0) && (max_skip == trimmed_size) && (min_gs_skip > 0) && (max_skip >= min_gs_skip))
            {
                gs_skips_.resize(trimmed_size);

                const std::size_t last = trimmed_size - 1;

                std::size_t last_prefix = last;

                for (std::size_t i = trimmed_size; i--;)
                {
                    if (is_prefix(i + 1))
                        last_prefix = i + 1;

                    gs_skips_[i] = last_prefix + (last - i);
                }

                for (std::size_t i = 0; i < last; ++i)
                {
                    std::size_t suffix_length = get_suffix_length(i);
                    std::size_t pos = last - suffix_length;

                    if (bytes[i - suffix_length] != bytes[pos])
                        gs_skips_[pos] = suffix_length + (last - i);
                }
            }
            else
            {
                bc_skips_[bytes[skip_pos_]] = 0;
            }
        }
    }

    inline std::size_t boyer_moore_scanner::get_longest_run(std::size_t& length) const
    {
        std::size_t max_skip = 0;
        std::size_t skip_pos = 0;

        std::size_t current_skip = 0;

        const byte* const masks = pattern_->masks();

        for (std::size_t i = 0; i < pattern_->trimmed_size(); ++i)
        {
            if (masks[i] != 0xFF)
            {
                if (current_skip > max_skip)
                {
                    max_skip = current_skip;
                    skip_pos = i - current_skip;
                }

                current_skip = 0;
            }
            else
            {
                ++current_skip;
            }
        }

        if (current_skip > max_skip)
        {
            max_skip = current_skip;
            skip_pos = pattern_->trimmed_size() - current_skip;
        }

        length = max_skip;

        return skip_pos;
    }

    inline bool boyer_moore_scanner::is_prefix(std::size_t pos) const
    {
        const std::size_t suffix_length = pattern_->trimmed_size() - pos;

        const byte* const bytes = pattern_->bytes();

        for (std::size_t i = 0; i < suffix_length; ++i)
            if (bytes[i] != bytes[pos + i])
                return false;

        return true;
    }

    inline std::size_t boyer_moore_scanner::get_suffix_length(std::size_t pos) const
    {
        const std::size_t last = pattern_->trimmed_size() - 1;

        const byte* const bytes = pattern_->bytes();

        std::size_t i = 0;

        while ((i < pos) && (bytes[pos - i] == bytes[last - i]))
            ++i;

        return i;
    }

    inline pointer boyer_moore_scanner::scan(region range) const
    {
        const std::size_t trimmed_size = pattern_->trimmed_size();

        if (!trimmed_size)
            return nullptr;

        const std::size_t original_size = pattern_->size();
        const std::size_t region_size = range.size;

        if (original_size > region_size)
            return nullptr;

        const byte* const region_base = range.start.as<const byte*>();
        const byte* const region_end = region_base + region_size;

        const byte* current = region_base;
        const byte* const end = region_end - original_size + 1;

        const std::size_t last = trimmed_size - 1;

        const byte* const pat_bytes = pattern_->bytes();
        const std::size_t* const pat_skips = !bc_skips_.empty() ? bc_skips_.data() : nullptr;

        if (pattern_->needs_masks())
        {
            const byte* const pat_masks = pattern_->masks();

            if (pat_skips)
            {
                const std::size_t pat_skip_pos = skip_pos_;

                while (MEM_LIKELY(current < end))
                {
                    std::size_t skip = pat_skips[current[pat_skip_pos]];

                    current += skip;

                    if (MEM_LIKELY(skip != 0))
                        continue;

                    for (std::size_t i = last; MEM_LIKELY((current[i] & pat_masks[i]) == pat_bytes[i]); --i)
                    {
                        if (MEM_UNLIKELY(i == 0))
                            return current;
                    }

                    ++current;
                }

                return nullptr;
            }
            else
            {
                while (MEM_LIKELY(current < end))
                {
                    for (std::size_t i = last; MEM_LIKELY((current[i] & pat_masks[i]) == pat_bytes[i]); --i)
                    {
                        if (MEM_UNLIKELY(i == 0))
                            return current;
                    }

                    ++current;
                }

                return nullptr;
            }
        }
        else
        {
            if (pat_skips && !gs_skips_.empty())
            {
                const std::size_t* const pat_suffixes = gs_skips_.data();

                current += last;
                const byte* const end_plus_last = end + last;

                while (MEM_LIKELY(current < end_plus_last))
                {
                    std::size_t i = last;

                    while (MEM_LIKELY(*current == pat_bytes[i]))
                    {
                        if (MEM_UNLIKELY(i == 0))
                            return current;

                        --current;
                        --i;
                    }

                    const std::size_t bc_skip = pat_skips[*current];
                    const std::size_t gs_skip = pat_suffixes[i];

                    current += (bc_skip > gs_skip) ? bc_skip : gs_skip;
                }

                return nullptr;
            }
            else if (pat_skips)
            {
                while (MEM_LIKELY(current < end))
                {
                    std::size_t skip = pat_skips[current[last]];

                    current += skip;

                    if (MEM_LIKELY(skip != 0))
                        continue;

                    for (std::size_t i = last; MEM_LIKELY(current[i] == pat_bytes[i]); --i)
                    {
                        if (MEM_UNLIKELY(i == 0))
                            return current;
                    }

                    ++current;
                }

                return nullptr;
            }
            else
            {
                while (MEM_LIKELY(current < end))
                {
                    for (std::size_t i = last; MEM_LIKELY(current[i] == pat_bytes[i]); --i)
                    {
                        if (MEM_UNLIKELY(i == 0))
                            return current;
                    }

                    ++current;
                }

                return nullptr;
            }
        }
    }
} // namespace mem

#endif // MEM_BOYER_MOORE_SCANNER_BRICK_H
