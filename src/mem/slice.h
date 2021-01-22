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

#ifndef MEM_SLICE_BRICK_H
#define MEM_SLICE_BRICK_H

#include "defines.h"

namespace mem
{
    template <typename T, typename U>
    struct copy_cv
    {
        typedef U type;
    };

    template <typename T, typename U>
    struct copy_cv<const T, U>
    {
        typedef const U type;
    };

    template <typename T, typename U>
    struct copy_cv<volatile T, U>
    {
        typedef volatile U type;
    };

    template <typename T, typename U>
    struct copy_cv<const volatile T, U>
    {
        typedef const volatile U type;
    };

    template <typename T>
    class slice
    {
    private:
        T* start_ {nullptr};
        std::size_t size_ {0};

    public:
        constexpr slice() noexcept = default;
        constexpr slice(T* begin, T* end) noexcept;
        constexpr slice(T* start_, std::size_t size_) noexcept;

        constexpr T& operator[](std::size_t index) const noexcept;

        constexpr T* data() const noexcept;
        constexpr T* begin() const noexcept;
        constexpr T* end() const noexcept;

        constexpr std::size_t size() const noexcept;
        constexpr bool empty() const noexcept;

        slice<typename copy_cv<T, byte>::type> as_bytes() const noexcept;

        using value_type = T;

        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;

        using reference = value_type&;
        using const_reference = const value_type&;

        using pointer = value_type*;
        using const_pointer = const value_type*;

        using iterator = value_type*;
        using const_iterator = const value_type*;
    };

    template <typename T>
    MEM_STRONG_INLINE constexpr slice<T>::slice(T* begin, T* end) noexcept
        : start_(begin)
        , size_(end - begin)
    {}

    template <typename T>
    MEM_STRONG_INLINE constexpr slice<T>::slice(T* start, std::size_t size) noexcept
        : start_(start)
        , size_(size)
    {}

    template <typename T>
    MEM_STRONG_INLINE constexpr T& slice<T>::operator[](std::size_t index) const noexcept
    {
        return start_[index];
    }

    template <typename T>
    MEM_STRONG_INLINE constexpr T* slice<T>::data() const noexcept
    {
        return start_;
    }

    template <typename T>
    MEM_STRONG_INLINE constexpr T* slice<T>::begin() const noexcept
    {
        return start_;
    }

    template <typename T>
    MEM_STRONG_INLINE constexpr T* slice<T>::end() const noexcept
    {
        return start_ + size_;
    }

    template <typename T>
    MEM_STRONG_INLINE constexpr std::size_t slice<T>::size() const noexcept
    {
        return size_;
    }

    template <typename T>
    MEM_STRONG_INLINE constexpr bool slice<T>::empty() const noexcept
    {
        return size_ == 0;
    }

    template <typename T>
    MEM_STRONG_INLINE slice<typename copy_cv<T, byte>::type> slice<T>::as_bytes() const noexcept
    {
        return {reinterpret_cast<typename copy_cv<T, byte>::type*>(start_), size_ * sizeof(T)};
    }
} // namespace mem

#endif // MEM_SLICE_BRICK_H
