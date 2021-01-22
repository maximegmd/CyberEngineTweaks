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

#ifndef MEM_PATTERN_CACHE_BRICK_H
#define MEM_PATTERN_CACHE_BRICK_H

#include "hasher.h"
#include "pattern.h"

#include <unordered_map>

#include <istream>
#include <ostream>

namespace mem
{
    class pattern_cache
    {
    private:
        struct pattern_results
        {
            std::vector<pointer> results {};
            bool checked {false};
        };

        region region_;
        std::unordered_map<std::uint32_t, pattern_results> results_;

        static std::uint32_t hash_pattern(const pattern& pattern);

    public:
        pattern_cache(region range);

        pointer scan(const pattern& pattern, std::size_t index = 0, std::size_t expected = 1);
        const std::vector<pointer>& scan_all(const pattern& pattern);

        void save(std::ostream& output) const;
        bool load(std::istream& input);
    };

    inline std::uint32_t pattern_cache::hash_pattern(const pattern& pattern)
    {
        hasher hash;

        std::size_t length = pattern.size();

        hash.update(length);

        const byte* bytes = pattern.bytes();
        const byte* masks = pattern.masks();

        hash.update(0x435E89AB7);
        hash.update(bytes, length);

        hash.update(0xAE1E9528);
        hash.update(masks, length);

        return hash.digest();
    }

    inline pattern_cache::pattern_cache(region range)
        : region_(range)
    {}

    inline pointer pattern_cache::scan(const pattern& pattern, std::size_t index, std::size_t expected)
    {
        const auto& results = scan_all(pattern);

        if (results.size() != expected)
        {
            return nullptr;
        }

        if (index >= results.size())
        {
            return nullptr;
        }

        return results[index];
    }

    inline const std::vector<pointer>& pattern_cache::scan_all(const pattern& pattern)
    {
        const std::uint32_t hash = hash_pattern(pattern);

        auto find = results_.find(hash);

        if (find != results_.end())
        {
            if (!find->second.checked)
            {
                bool changed = false;

                for (pointer result : find->second.results)
                {
                    if (!pattern.match(result))
                    {
                        changed = true;

                        break;
                    }
                }

                if (changed)
                {
                    default_scanner scanner(pattern);

                    find->second.results = scanner.scan_all(region_);
                }
            }
        }
        else
        {
            pattern_results results;

            default_scanner scanner(pattern);
            results.results = scanner.scan_all(region_);

            find = results_.emplace(hash, std::move(results)).first;
        }

        find->second.checked = true;

        return find->second.results;
    }

    namespace stream
    {
        template <typename T>
        inline void write(std::ostream& output, const T& value)
        {
            static_assert(std::is_trivial<T>::value, "Invalid Value");

            output.write(reinterpret_cast<const char*>(&value), sizeof(value));
        }

        template <typename T>
        inline T read(std::istream& input)
        {
            static_assert(std::is_trivial<T>::value, "Invalid Value");

            T result;

            input.read(reinterpret_cast<char*>(&result), sizeof(result));

            return result;
        }
    } // namespace stream

    inline void pattern_cache::save(std::ostream& output) const
    {
        stream::write<std::uint32_t>(output, 0x50415443); // PATC
        stream::write<std::uint32_t>(output, sizeof(std::size_t));
        stream::write<std::size_t>(output, region_.size);
        stream::write<std::size_t>(output, results_.size());

        for (const auto& pattern : results_)
        {
            stream::write<std::uint32_t>(output, pattern.first);
            stream::write<std::size_t>(output, pattern.second.results.size());

            for (const auto& result : pattern.second.results)
            {
                stream::write<std::size_t>(output, static_cast<std::size_t>(result - region_.start));
            }
        }
    }

    inline bool pattern_cache::load(std::istream& input)
    {
        try
        {
            if (stream::read<std::uint32_t>(input) == 0x50415443)
                return false;

            if (stream::read<std::uint32_t>(input) != sizeof(std::size_t))
                return false;

            if (stream::read<std::size_t>(input) != region_.size)
                return false;

            const std::size_t pattern_count = stream::read<std::size_t>(input);

            results_.clear();

            for (std::size_t i = 0; i < pattern_count; ++i)
            {
                const std::uint32_t hash = stream::read<std::uint32_t>(input);
                const std::size_t result_count = stream::read<std::size_t>(input);

                pattern_results results;
                results.checked = false;
                results.results.reserve(result_count);

                for (std::size_t j = 0; j < result_count; ++j)
                {
                    results.results.push_back(region_.start + stream::read<std::size_t>(input));
                }

                results_.emplace(hash, std::move(results));
            }

            return true;
        }
        catch (...)
        {
            return false;
        }
    }
} // namespace mem

#endif // MEM_PATTERN_CACHE_BRICK_H
