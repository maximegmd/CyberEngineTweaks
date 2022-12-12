#pragma once

#include <sol/sol.hpp>
#include <spdlog/spdlog.h>
#include <imgui.h>
#include <MinHook.h>
#include <mem/module.h>
#include <mem/pattern.h>
#include <nlohmann/json.hpp>

#include <common/Relocation.h>

#if GAME_CYBERPUNK
#include <stdafx.cyberpunk.h>
#else
#include <stdafx.witcher3.h>
#endif

#include <TiltedCore/Allocator.hpp>
#include <TiltedCore/Lockable.hpp>
#include <TiltedCore/Platform.hpp>
#include <TiltedCore/ScratchAllocator.hpp>
#include <TiltedCore/Signal.hpp>
#include <TiltedCore/StackAllocator.hpp>
#include <TiltedCore/TaskQueue.hpp>

#include <atlcomcli.h>
#include <d3d12.h>
#include <d3d11.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DbgHelp.h>
#include <dxgi1_4.h>
#include <SDKDDKVer.h>
#include <Windows.h>
#include <wrl.h>

#include <algorithm>
#include <array>
#include <bitset>
#include <compare>
#include <cstdint>
#include <execution>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <regex>
#include <set>
#include <shared_mutex>
#include <source_location>
#include <unordered_map>
#include <vector>

#include "EngineTweaksVersion.h"
#include "common/Logging.h"
#include "common/FontMaterialDesignIcons.h"
#include "Options.h"
#include "Paths.h"
#include "PersistentState.h"
#include "scripting/GameHooks.h"
#include "VKBindings.h"

