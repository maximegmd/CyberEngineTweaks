#pragma once

#include <sol/sol.hpp>
#include <spdlog/spdlog.h>
#include <imgui.h>
#include <MinHook.h>
#include <nlohmann/json.hpp>

#include <RED4ext/REDfunc.hpp>
#include <RED4ext/DynArray.hpp>
#include <RED4ext/REDptr.hpp>
#include <RED4ext/REDhash.hpp>
#include <RED4ext/CName.hpp>
#include <RED4ext/CString.hpp>
#include <RED4ext/ISerializable.hpp>
#include <RED4ext/Types/InstanceType.hpp>
#include <RED4ext/Types/SimpleTypes.hpp>
#include <RED4ext/Types/CharacterCustomization.hpp>
#include <RED4ext/RTTISystem.hpp>
#include <RED4ext/RTTITypes.hpp>
#include <RED4ext/GameEngine.hpp>
#include <RED4ext/Scripting/Stack.hpp>
#include <RED4ext/Scripting/CProperty.hpp>
#include <RED4ext/Scripting/Functions.hpp>
#include <RED4ext/Scripting/OpcodeHandlers.hpp>
#include <RED4ext/MemoryAllocators.hpp>

#include <filesystem>
#include <iostream>
#include <cstdint>
#include <algorithm>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <shared_mutex>
#include <memory>
#include <fstream>
#include <array>
#include <bitset>

#include <atlcomcli.h>
#include <d3d12.h>
#include <d3d11.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>
#include <Windows.h>
#include <SDKDDKVer.h>
#include <DbgHelp.h>

#include <TiltedCore/Allocator.hpp>
#include <TiltedCore/ScratchAllocator.hpp>
#include <TiltedCore/StackAllocator.hpp>
#include <TiltedCore/TaskQueue.hpp>
#include <TiltedCore/Platform.hpp>
#include <TiltedCore/Signal.hpp>
#include <TiltedCore/Lockable.hpp>

#include <mem/module.h>
#include <mem/pattern.h>

#include "Paths.h"
#include "VKBindings.h"
#include "Options.h"
#include "CETVersion.h"
