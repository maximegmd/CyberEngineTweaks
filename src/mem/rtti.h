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

#ifndef MEM_RTTI_BRICK_H
#define MEM_RTTI_BRICK_H

#include "mem.h"

#if defined(MEM_ARCH_X86) || defined(MEM_ARCH_X86_64)
#    if !defined(_WIN32)
#        error mem::rtti only supports windows
#    endif // !_WIN32
#else
#    error mem::rtti only supports x86 and x64
#endif

#include <functional>

namespace mem
{
    namespace rtti
    {
        struct PMD;
        struct RTTITypeDescriptor;
        struct RTTICompleteObjectLocator;
        struct RTTIClassHierarchyDescriptor;
        struct RTTIBaseClassArray;
        struct RTTIBaseClassDescriptor;

        struct PMD
        {
            std::int32_t mdisp; // member displacement
            std::int32_t pdisp; // vbtable displacement
            std::int32_t vdisp; // displacement inside vbtable
        };

        struct RTTITypeDescriptor // type_info
        {
            void* vTable;
            const char* UndecoratedName;
            const char DecoratedName[1];

#if defined(MEM_RTTI_DEMANGLE)
            std::string demangle() const;
#endif // MEM_RTTI_DEMANGLE
        };

        struct RTTICompleteObjectLocator
        {
            std::uint32_t signature;        // 0 = x86, 1 = x64
            std::uint32_t offset;           // offset of this vtable in the complete class
            std::uint32_t cdOffset;         // constructor displacement offset
            std::uint32_t pTypeDescriptor;  // TypeDescriptor of the complete class
            std::uint32_t pClassDescriptor; // describes inheritance hierarchy

#if defined(MEM_ARCH_X86_64)
            std::uint32_t pSelf;
#endif // MEM_ARCH_X86_64

            bool check_signature() const;
            RTTITypeDescriptor* get_type(const region& region) const;
            RTTIClassHierarchyDescriptor* get_class(const region& region) const;

#if defined(MEM_ARCH_X86_64)
            RTTICompleteObjectLocator* get_self(const region& region) const;
#endif // MEM_ARCH_X86_64
        };

        struct RTTIClassHierarchyDescriptor
        {
            std::uint32_t signature;      // 0 = x86, 1 = x64
            std::uint32_t attributes;     // bit 0 set = multiple inheritance, bit 1 set = virtual inheritance
            std::uint32_t numBaseClasses; // number of base classes
            std::uint32_t pBaseClassArray;

            bool check_signature() const;
            std::uint32_t get_base_count() const;
            RTTIBaseClassArray* get_base_classes(const region& region) const;

            bool inherits_from(const region& region, const RTTITypeDescriptor* type) const;
        };

        struct RTTIBaseClassArray
        {
            std::uint32_t arrayOfBaseClassDescriptors[1];

            RTTIBaseClassDescriptor* get_base_class(const region& region, std::uint32_t index) const;
        };

        struct RTTIBaseClassDescriptor
        {
            std::uint32_t pTypeDescriptor;   // type descriptor of the class
            std::uint32_t numContainedBases; // number of nested classes following in the Base Class Array
            PMD where;                       // pointer-to-member displacement info
            std::uint32_t attributes;        // flags, usually 0

            RTTITypeDescriptor* get_type(const region& region) const;
        };

        void enumerate_rtti(const region& region,
            std::function<bool(
                const void** vTable, const RTTICompleteObjectLocator* object, const RTTITypeDescriptor* type)>
                callback);
        const RTTITypeDescriptor* find_rtti_type(const region& region, const char* name);

        inline constexpr bool check_rtti_signature(std::uint32_t signature) noexcept
        {
#if defined(MEM_ARCH_X86_64)
            return signature == 1;
#elif defined(MEM_ARCH_X86)
            return signature == 0;
#else
#    error "Invalid Architecture"
#endif
        }

        template <typename T>
        inline T* get_rtti_pointer(const region& region, std::uint32_t address) noexcept
        {
#if defined(MEM_ARCH_X86_64)
            return region.start.add(address).as<T*>();
#elif defined(MEM_ARCH_X86)
            const pointer result = address;

            return region.contains(result) ? result.as<T*>() : nullptr;
#else
#    error "Invalid Architecture"
#endif
        }

#if defined(MEM_RTTI_DEMANGLE)
        inline std::string RTTITypeDescriptor::demangle() const
        {
            char buffer[1024];

            if (DWORD symbol_size = UnDecorateSymbolName(DecoratedName + 1, buffer, 1024,
                    UNDNAME_32_BIT_DECODE | UNDNAME_NAME_ONLY | UNDNAME_NO_ARGUMENTS | UNDNAME_NO_MS_KEYWORDS))
            {
                return std::string(buffer, symbol_size);
            }

            return "";
        }
#endif // MEM_RTTI_DEMANGLE

        inline bool RTTICompleteObjectLocator::check_signature() const
        {
            return check_rtti_signature(signature);
        }

        inline RTTITypeDescriptor* RTTICompleteObjectLocator::get_type(const region& region) const
        {
            return get_rtti_pointer<RTTITypeDescriptor>(region, pTypeDescriptor);
        }

        inline RTTIClassHierarchyDescriptor* RTTICompleteObjectLocator::get_class(const region& region) const
        {
            return get_rtti_pointer<RTTIClassHierarchyDescriptor>(region, pClassDescriptor);
        }

#if defined(MEM_ARCH_X86_64)
        inline RTTICompleteObjectLocator* RTTICompleteObjectLocator::get_self(const region& region) const
        {
            return get_rtti_pointer<RTTICompleteObjectLocator>(region, pSelf);
        }
#endif // MEM_ARCH_X86_64

        inline bool RTTIClassHierarchyDescriptor::check_signature() const
        {
            return check_rtti_signature(signature);
        }

        inline std::uint32_t RTTIClassHierarchyDescriptor::get_base_count() const
        {
            return numBaseClasses;
        }

        inline RTTIBaseClassArray* RTTIClassHierarchyDescriptor::get_base_classes(const region& region) const
        {
            return get_rtti_pointer<RTTIBaseClassArray>(region, pBaseClassArray);
        }

        inline bool RTTIClassHierarchyDescriptor::inherits_from(
            const region& region, const RTTITypeDescriptor* type) const
        {
            const RTTIBaseClassArray* base_classes = get_base_classes(region);

            for (std::uint32_t i = 1; i < get_base_count(); ++i)
            {
                const RTTIBaseClassDescriptor* base_class = base_classes->get_base_class(region, i);
                const RTTITypeDescriptor* base_type = base_class->get_type(region);

                if (!std::strcmp(base_type->DecoratedName, type->DecoratedName))
                {
                    return true;
                }
            }

            return false;
        }

        inline RTTIBaseClassDescriptor* RTTIBaseClassArray::get_base_class(
            const region& region, std::uint32_t index) const
        {
            return get_rtti_pointer<RTTIBaseClassDescriptor>(region, arrayOfBaseClassDescriptors[index]);
        }

        inline RTTITypeDescriptor* RTTIBaseClassDescriptor::get_type(const region& region) const
        {
            return get_rtti_pointer<RTTITypeDescriptor>(region, pTypeDescriptor);
        }

        inline void enumerate_rtti(const region& region,
            std::function<bool(
                const void** vTable, const RTTICompleteObjectLocator* object, const RTTITypeDescriptor* type)>
                callback)
        {
            for (std::size_t i = 0; i < region.size; i += sizeof(void*))
            {
                const RTTICompleteObjectLocator*& locator = region.start.at<const RTTICompleteObjectLocator*>(i);

                if (!region.contains<RTTICompleteObjectLocator>(locator))
                {
                    continue;
                }

                if (!locator->check_signature())
                {
                    continue;
                }

#if defined(MEM_ARCH_X86_64)
                if (locator->get_self(region) != locator)
                {
                    continue;
                }
#endif // MEM_ARCH_X86_64

                const RTTITypeDescriptor* type = locator->get_type(region);

                if (!type)
                {
                    continue;
                }

                if (!region.contains<void*>(type->vTable))
                {
                    continue;
                }

                if (strncmp(type->DecoratedName, ".?", 2))
                {
                    continue;
                }

                if (callback(reinterpret_cast<const void**>(&locator + 1), locator, type))
                {
                    break;
                }
            }
        }

#if defined(MEM_RTTI_DEMANGLE)
        inline const RTTITypeDescriptor* find_rtti_type(const region& region, const char* name)
        {
            const RTTITypeDescriptor* result = nullptr;

            enumerate_rtti(region,
                [&result, name](
                    const void**, const RTTICompleteObjectLocator*, const RTTITypeDescriptor* type) -> bool {
                    if (type->demangle() == name)
                    {
                        result = type;

                        return true;
                    }

                    return false;
                });

            return result;
        }
#endif // MEM_RTTI_DEMANGLE
    }  // namespace rtti
} // namespace mem
#endif // !MEM_RTTI_BRICK_H
