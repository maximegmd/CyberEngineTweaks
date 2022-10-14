#pragma once

#include <overlay/widgets/Widget.h>

void ltrim(std::string& s);
void rtrim(std::string& s);
void trim(std::string& s);

std::string UTF16ToUTF8(std::wstring_view utf16);
std::wstring UTF8ToUTF16(std::string_view utf8);

spdlog::sink_ptr CreateCustomSinkST(const std::function<void(const std::string&)>& acpSinkItHandler, const std::function<void()>& acpFlushHandler = nullptr);
spdlog::sink_ptr CreateCustomSinkMT(const std::function<void(const std::string&)>& acpSinkItHandler, const std::function<void()>& acpFlushHandler = nullptr);
std::shared_ptr<spdlog::logger> CreateLogger(const std::filesystem::path& acpPath, const std::string& acpID, const spdlog::sink_ptr& acpExtraSink = nullptr, const std::string& acpPattern = "[%Y-%m-%d %H:%M:%S UTC%z] [%t] %v");

// deep copies sol object (doesnt take into account potential duplicates)
sol::object DeepCopySolObject(const sol::object& acpObj, const sol::state_view& acpStateView);

// makes sol usertype or userdata immutable when accessed from lua
void MakeSolUsertypeImmutable(const sol::object& acpObj, const sol::state_view& acpStateView);

// Add unnamed function to the Lua registry
template<typename F>
sol::function MakeSolFunction(sol::state& aState, F aFunc)
{
    // This is slightly better than wrapping lambdas in sol::object:
    // 1. When the lambda is wrapped in an object sol registers additional usertype for the lambda type.
    // 2. Calling a lambda as an object has a tiny overhead of dealing with metatables.
    // 3. In Lua `type(f)` for a lambda as an object will return "userdata" instead of the expected "function".

    static constexpr auto s_cTempFuncName = "___func_temp_holder_";

    aState[s_cTempFuncName] = aFunc;
    sol::function luaFunc = aState[s_cTempFuncName];
    aState[s_cTempFuncName] = sol::nil;

    return luaFunc;
}

// Check if Lua object is of cdata type
bool IsLuaCData(const sol::object& acpObject);

float GetAlignedItemWidth(const int64_t acItemsCount);

float GetCenteredOffsetForText(const char* acpText);

enum class THWUCPResult
{
    CHANGED,
    APPLY,
    DISCARD,
    CANCEL
};
THWUCPResult UnsavedChangesPopup(bool& aFirstTime, const bool acMadeChanges, const TWidgetCB& acpSaveCB, const TWidgetCB& acpLoadCB, const TWidgetCB& acpCancelCB = nullptr);

[[nodiscard]] std::filesystem::path GetAbsolutePath(const std::string& acFilePath, const std::filesystem::path& acRootPath, const bool acAllowNonExisting, const bool acAllowSymlink = true);
[[nodiscard]] std::filesystem::path GetAbsolutePath(std::filesystem::path aFilePath, const std::filesystem::path& acRootPath, const bool acAllowNonExisting, const bool acAllowSymlink = true);

[[nodiscard]] std::filesystem::path GetLuaPath(const std::string& acFilePath, const std::filesystem::path& acRootPath, const bool acAllowNonExisting);
[[nodiscard]] std::filesystem::path GetLuaPath(std::filesystem::path aFilePath, const std::filesystem::path& acRootPath, const bool acAllowNonExisting);

[[nodiscard]] std::vector<uint8_t> ReadWholeBinaryFile(const std::filesystem::path& acpPath);
[[nodiscard]] std::string ReadWholeTextFile(const std::filesystem::path& acpPath);

[[nodiscard]] std::vector<uint8_t> EncodeToLzma(const std::filesystem::path& acpPath);
[[nodiscard]] std::vector<uint8_t> EncodeToLzma(const std::vector<uint8_t>& acpIn);

[[nodiscard]] std::vector<uint8_t> DecodeFromLzma(const std::filesystem::path& acpPath);
[[nodiscard]] std::vector<uint8_t> DecodeFromLzma(const std::vector<uint8_t>& acpIn);
