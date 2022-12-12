#pragma once


#include <RED4ext/CName.hpp>
#include <RED4ext/CString.hpp>
#include <RED4ext/DynArray.hpp>
#include "RED4ext/GameApplication.hpp"
#include <RED4ext/GameEngine.hpp>
#include <RED4ext/Hashing/CRC.hpp>
#include <RED4ext/ISerializable.hpp>
#include <RED4ext/Relocation.hpp>
#include <RED4ext/ResourceDepot.hpp>
#include <RED4ext/RTTISystem.hpp>
#include <RED4ext/RTTITypes.hpp>
#include <RED4ext/Scripting/CProperty.hpp>
#include <RED4ext/Scripting/Functions.hpp>
#include <RED4ext/Scripting/OpcodeHandlers.hpp>
#include <RED4ext/Scripting/Stack.hpp>
#include <RED4ext/TweakDB.hpp>

#include <cyberpunk/reverse/Addresses.h>

template <> struct std::hash<RED4ext::CName>
{
    std::size_t operator()(RED4ext::CName aKey) const noexcept { return static_cast<size_t>(aKey.hash); }
};

namespace Game = CyberEngineTweaks;