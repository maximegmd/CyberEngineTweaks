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

#ifndef MEM_DATA_BUFFER_BRICK_H
#define MEM_DATA_BUFFER_BRICK_H

#include "defines.h"

#include <cstdlib>
#include <cstring>

#include <type_traits>

namespace mem
{
    template <typename T>
    class data_buffer
    {
    private:
        static_assert(std::is_trivial<T>::value, "Type is not trivially copyable");

        T* data_ {nullptr};
        std::size_t size_ {0};
        std::size_t capacity_ {0};

        std::size_t calculate_growth(std::size_t new_size) const noexcept;
        void reallocate(std::size_t length);

    public:
        data_buffer() noexcept = default;

        data_buffer(std::size_t length);

        data_buffer(const data_buffer& other);
        data_buffer(data_buffer&& other) noexcept;

        ~data_buffer();

        data_buffer& operator=(const data_buffer& other);
        data_buffer& operator=(data_buffer&& other);

        void swap(data_buffer& other) noexcept;

        void reserve(std::size_t length);
        void resize(std::size_t length);
        void reset(std::size_t length = 0);

        void assign(const T* source, std::size_t length);
        void append(const T* source, std::size_t length);

        void push_back(const T& value);

        void clear() noexcept;
        void shrink_to_fit();

        T& operator[](std::size_t index) noexcept;
        const T& operator[](std::size_t index) const noexcept;

        T* data() noexcept;
        T* begin() noexcept;
        T* end() noexcept;

        const T* data() const noexcept;
        const T* begin() const noexcept;
        const T* end() const noexcept;

        std::size_t size() const noexcept;
        std::size_t capacity() const noexcept;
        bool empty() const noexcept;

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

    using byte_buffer = data_buffer<byte>;

    template <typename T>
    inline data_buffer<T>::data_buffer(std::size_t length)
    {
        resize(length);
    }

    template <typename T>
    inline data_buffer<T>::data_buffer(const data_buffer<T>& other)
    {
        if (this != &other)
        {
            assign(other.data(), other.size());
        }
    }

    template <typename T>
    inline data_buffer<T>::data_buffer(data_buffer<T>&& other) noexcept
    {
        if (this != &other)
        {
            swap(other);
        }
    }

    template <typename T>
    inline data_buffer<T>::~data_buffer()
    {
        clear();
    }

    template <typename T>
    inline data_buffer<T>& data_buffer<T>::operator=(const data_buffer<T>& other)
    {
        if (this != &other)
        {
            assign(other.data(), other.size());
        }

        return *this;
    }

    template <typename T>
    inline data_buffer<T>& data_buffer<T>::operator=(data_buffer<T>&& other)
    {
        if (this != &other)
        {
            clear();

            swap(other);
        }

        return *this;
    }

    template <typename T>
    inline std::size_t data_buffer<T>::calculate_growth(std::size_t new_size) const noexcept
    {
        std::size_t old_capacity = capacity();

        if (new_size > old_capacity)
        {
            std::size_t new_capacity = old_capacity + (old_capacity >> 1);

            if (new_capacity < new_size)
            {
                new_capacity = new_size;
            }

            return new_capacity;
        }
        else
        {
            return old_capacity;
        }
    }

    template <typename T>
    inline void data_buffer<T>::reallocate(std::size_t length)
    {
        if (length != capacity_)
        {
            void* new_data = nullptr;

            if (length)
            {
                new_data = std::realloc(data_, length * sizeof(T));

                if (new_data == nullptr)
                {
                    std::abort();
                }
            }
            else
            {
                std::free(data_);
            }

            data_ = static_cast<T*>(new_data);
            capacity_ = length;

            if (size_ > capacity_)
            {
                size_ = capacity_;
            }
        }
    }

    template <typename T>
    inline void data_buffer<T>::swap(data_buffer<T>& other) noexcept
    {
        if (this != &other)
        {
            T* temp_data = data_;
            std::size_t temp_size = size_;
            std::size_t temp_capacity = capacity_;

            data_ = other.data_;
            size_ = other.size_;
            capacity_ = other.capacity_;

            other.data_ = temp_data;
            other.size_ = temp_size;
            other.capacity_ = temp_capacity;
        }
    }

    template <typename T>
    inline void data_buffer<T>::reserve(std::size_t length)
    {
        if (length > capacity_)
        {
            reallocate(length);
        }
    }

    template <typename T>
    inline void data_buffer<T>::resize(std::size_t length)
    {
        reallocate(length);

        size_ = length;
    }

    template <typename T>
    inline void data_buffer<T>::reset(std::size_t length)
    {
        clear();

        resize(length);
    }

    template <typename T>
    inline void data_buffer<T>::assign(const T* source, std::size_t length)
    {
        clear();

        append(source, length);
    }

    template <typename T>
    inline void data_buffer<T>::append(const T* source, std::size_t length)
    {
        std::size_t old_size = size_;
        std::size_t new_size = old_size + length;

        reserve(calculate_growth(new_size));
        std::memcpy(data_ + old_size, source, length * sizeof(T));

        size_ = new_size;
    }

    template <typename T>
    inline void data_buffer<T>::push_back(const T& value)
    {
        append(&value, 1);
    }

    template <typename T>
    inline void data_buffer<T>::clear() noexcept
    {
        size_ = 0;
    }

    template <typename T>
    inline void data_buffer<T>::shrink_to_fit()
    {
        resize(size_);
    }

    template <typename T>
    inline T& data_buffer<T>::operator[](std::size_t index) noexcept
    {
        return data_[index];
    }

    template <typename T>
    inline const T& data_buffer<T>::operator[](std::size_t index) const noexcept
    {
        return data_[index];
    }

    template <typename T>
    inline T* data_buffer<T>::data() noexcept
    {
        return data_;
    }

    template <typename T>
    inline T* data_buffer<T>::begin() noexcept
    {
        return data_;
    }

    template <typename T>
    inline T* data_buffer<T>::end() noexcept
    {
        return data_ + size_;
    }

    template <typename T>
    inline const T* data_buffer<T>::data() const noexcept
    {
        return data_;
    }

    template <typename T>
    inline const T* data_buffer<T>::begin() const noexcept
    {
        return data_;
    }

    template <typename T>
    inline const T* data_buffer<T>::end() const noexcept
    {
        return data_ + size_;
    }

    template <typename T>
    inline std::size_t data_buffer<T>::size() const noexcept
    {
        return size_;
    }

    template <typename T>
    inline std::size_t data_buffer<T>::capacity() const noexcept
    {
        return capacity_;
    }

    template <typename T>
    inline bool data_buffer<T>::empty() const noexcept
    {
        return size_ == 0;
    }
} // namespace mem

#endif // MEM_DATA_BUFFER_BRICK_H
