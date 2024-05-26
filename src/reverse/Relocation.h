#pragma once

#include <filesystem>
#include <mutex>
#include <sstream>

#include <Windows.h>

#include <RED4ext/Common.hpp>
#include <RED4ext/Detail/Memory.hpp>

template<>
struct RED4ext::Detail::AddressResolverOverride<uint32_t> : std::true_type
{
    inline static uintptr_t Resolve(uint32_t aHash)
    {
        using functionType = void* (*)(uint32_t);
        static functionType resolveFunc = nullptr;

        static std::once_flag flag;
        std::call_once(flag,
           []()
           {
               char exePath[4096];
               GetModuleFileNameA(GetModuleHandle(nullptr), exePath, 4096);

               std::filesystem::path exe = exePath;

               auto dllName = exe.parent_path() / "version.dll";
               constexpr auto functionName = "ResolveAddress";

               auto handle = LoadLibraryA(dllName.string().c_str());
               if (!handle)
               {
                   std::stringstream stream;
                   stream << "Failed to get '" << dllName
                          << "' handle.\nProcess will now close.\n\nLast error: " << GetLastError();

                   MessageBoxA(nullptr, stream.str().c_str(), "Cyber Engine Tweaks", MB_ICONERROR | MB_OK);
                   TerminateProcess(GetCurrentProcess(), 1);
                   return; // Disable stupid warning
               }

               resolveFunc = reinterpret_cast<functionType>(GetProcAddress(handle, functionName));
               if (resolveFunc == nullptr)
               {
                   std::stringstream stream;
                   stream << "Failed to get '" << functionName
                          << "' address.\nProcess will now close.\n\nLast error: " << GetLastError();

                   MessageBoxA(nullptr, stream.str().c_str(), "Cyber Engine Tweaks", MB_ICONERROR | MB_OK);
                   TerminateProcess(GetCurrentProcess(), 1);
               }
           });

        auto address = resolveFunc(aHash);
        if (address == nullptr)
        {
            std::stringstream stream;
            stream << "Failed to resolve address for hash " << std::hex << std::showbase << aHash << ".\nProcess will now close.";

            MessageBoxA(nullptr, stream.str().c_str(), "Cyber Engine Tweaks", MB_ICONERROR | MB_OK);
            TerminateProcess(GetCurrentProcess(), 1);
        }

        return reinterpret_cast<uintptr_t>(address);
    }
};
