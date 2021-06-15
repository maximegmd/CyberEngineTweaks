#pragma once

#include <sol/sol.hpp>
#include <spdlog/spdlog.h>
#include <imgui.h>
#include <MinHook.h>
#include <nlohmann/json.hpp>

#include <TiltedCore/Allocator.hpp>
#include <TiltedCore/ScratchAllocator.hpp>
#include <TiltedCore/StackAllocator.hpp>
#include <TiltedCore/TaskQueue.hpp>
#include <TiltedCore/Platform.hpp>
#include <TiltedCore/Signal.hpp>
#include <TiltedCore/Lockable.hpp>

#include <mem/module.h>
#include <mem/pattern.h>

#include "RED4ext.h"

#include <filesystem>
#include <iostream>
#include <cstdint>
#include <algorithm>
#include <vector>
#include <set>
#include <unordered_map>
#include <mutex>
#include <shared_mutex>
#include <memory>
#include <fstream>
#include <array>
#include <bitset>
#include <regex>
#include <execution>

#include <atlcomcli.h>
#include <d3d12.h>
#include <d3d11.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>
#include <Windows.h>
#include <SDKDDKVer.h>
#include <DbgHelp.h>

#include "Paths.h"
#include "VKBindings.h"
#include "Options.h"
#include "CETVersion.h"
