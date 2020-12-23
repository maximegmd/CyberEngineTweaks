#pragma once

template<class T>
struct Array
{
	T* entries;
	uint32_t capacity;
	uint32_t count;
};