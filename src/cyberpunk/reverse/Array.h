#pragma once

template <class T> struct Array
{
    T* entries{nullptr};
    uint32_t capacity{0};
    uint32_t count{0};
};