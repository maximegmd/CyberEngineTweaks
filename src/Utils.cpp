#include "stdafx.h"

#include "Utils.h"

#include <spdlog/sinks/rotating_file_sink.h>

void ltrim(std::string& s)
{
    s.erase(s.begin(), std::ranges::find_if(s, [](unsigned char ch) { return !std::isspace(ch); }));
}

void rtrim(std::string& s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());
}

void trim(std::string& s)
{
    ltrim(s);
    rtrim(s);
}

std::string UTF16ToUTF8(std::wstring_view utf16)
{
    const auto utf16Length = static_cast<int>(utf16.length());
    const auto utf8Length = WideCharToMultiByte(CP_UTF8, 0, utf16.data(), utf16Length, nullptr, 0, nullptr, nullptr);
    if (!utf8Length)
        return {};

    std::string utf8;
    utf8.resize(utf8Length);
    WideCharToMultiByte(CP_UTF8, 0, utf16.data(), utf16Length, utf8.data(), utf8Length, nullptr, nullptr);

    return utf8;
}

std::wstring UTF8ToUTF16(std::string_view utf8)
{
    const auto utf8Length = static_cast<int>(utf8.length());
    const auto utf16Length = MultiByteToWideChar(CP_UTF8, 0, utf8.data(), utf8Length, nullptr, 0);
    if (!utf16Length)
        return {};

    std::wstring utf16;
    utf16.resize(utf16Length);
    MultiByteToWideChar(CP_UTF8, 0, utf8.data(), utf8Length, utf16.data(), utf16Length);

    return utf16;
}

template <typename Mutex> class CustomSink final : public spdlog::sinks::base_sink<Mutex>
{
public:
    CustomSink(const std::function<void(const std::string&)>& acpSinkItHandler, const std::function<void()>& acpFlushHandler)
        : spdlog::sinks::base_sink<Mutex>()
        , m_sinkItHandler(acpSinkItHandler)
        , m_flushHandler(acpFlushHandler)
    {
    }

protected:
    void sink_it_(const spdlog::details::log_msg& msg) override
    {
        if (!m_sinkItHandler)
            return;

        spdlog::memory_buf_t formatted;
        spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);
        m_sinkItHandler(fmt::to_string(formatted));
    }

    void flush_() override
    {
        if (m_flushHandler)
            m_flushHandler();
    }

private:
    std::function<void(const std::string&)> m_sinkItHandler{nullptr};
    std::function<void()> m_flushHandler{nullptr};
};

template <typename Mutex> spdlog::sink_ptr CreateCustomSink(const std::function<void(const std::string&)>& acpSinkItHandler, const std::function<void()>& acpFlushHandler)
{
    return std::make_shared<CustomSink<Mutex>>(acpSinkItHandler, acpFlushHandler);
}

spdlog::sink_ptr CreateCustomSinkST(const std::function<void(const std::string&)>& acpSinkItHandler, const std::function<void()>& acpFlushHandler)
{
    return CreateCustomSink<spdlog::details::null_mutex>(acpSinkItHandler, acpFlushHandler);
}

spdlog::sink_ptr CreateCustomSinkMT(const std::function<void(const std::string&)>& acpSinkItHandler, const std::function<void()>& acpFlushHandler)
{
    return CreateCustomSink<std::mutex>(acpSinkItHandler, acpFlushHandler);
}

std::shared_ptr<spdlog::logger> CreateLogger(
    const std::filesystem::path& acpPath, const std::string& acpID, const spdlog::sink_ptr& acpExtraSink, const std::string& acpPattern, const size_t acMaxFileSize,
    const size_t acMaxFileCount)
{
    if (auto existingLogger = spdlog::get(acpID))
        return existingLogger;

    const auto rotSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(acpPath.native(), acMaxFileSize, acMaxFileCount);
    rotSink->set_pattern(acpPattern);

    auto logger = std::make_shared<spdlog::logger>(acpID, spdlog::sinks_init_list{rotSink});
    logger->set_level(spdlog::level::level_enum::trace);

    if (acpExtraSink)
        logger->sinks().emplace_back(acpExtraSink);

#ifdef CET_DEBUG
    logger->flush_on(spdlog::level::trace);
#else
    logger->flush_on(spdlog::level::warn);
#endif

    register_logger(logger);
    return logger;
}

// deep copies sol object (doesnt take into account potential duplicates)
sol::object DeepCopySolObject(const sol::object& acpObj, const sol::state_view& acpStateView)
{
    if (acpObj == sol::nil || acpObj.get_type() != sol::type::table)
        return acpObj;
    sol::table src = acpObj;
    sol::table copy{acpStateView, sol::create};
    for (const auto& kv : src)
        copy[DeepCopySolObject(kv.first, acpStateView)] = DeepCopySolObject(kv.second, acpStateView);
    copy[sol::metatable_key] = src[sol::metatable_key];
    return copy;
}

// makes sol usertype or userdata immutable when accessed from lua
void MakeSolUsertypeImmutable(const sol::object& acpObj, const sol::state_view& acpStateView)
{
    if (!acpObj.is<sol::metatable>() && !acpObj.is<sol::userdata>())
        return;

    sol::table target = acpObj;
    sol::table metatable;
    sol::object metaref = target[sol::metatable_key];

    if (metaref.is<sol::table>())
    {
        metatable = metaref;
    }
    else
    {
        metatable = {acpStateView, sol::create};
        target[sol::metatable_key] = metatable;
    }

    // prevent adding new properties
    metatable[sol::meta_function::new_index] = [] {
    };

    // prevent overriding metatable
    metatable[sol::meta_function::metatable] = []
    {
        return sol::nil;
    };
}

// Check if Lua object is of cdata type
bool IsLuaCData(const sol::object& acpObject)
{
    // Sol doesn't have enum for LuaJIT's cdata type since it's not a standard type.
    // But it's possible to check the type using numeric code (10).
    // LuaJIT packs int64/uint64 into cdata and some other types.
    // Since we're not using other types, this should be enough to check for int64/uint64 value.
    return static_cast<int>(acpObject.get_type()) == 10;
}

float GetAlignedItemWidth(const int64_t acItemsCount)
{
    return (ImGui::GetWindowContentRegionWidth() - static_cast<float>(acItemsCount - 1) * ImGui::GetStyle().ItemSpacing.x) / static_cast<float>(acItemsCount);
}

float GetCenteredOffsetForText(const char* acpText)
{
    return (ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(acpText).x) / 2.0f;
}

TChangedCBResult UnsavedChangesPopup(
    const std::string& acpOwnerName, bool& aFirstTime, const bool acMadeChanges, const TWidgetCB& acpSaveCB, const TWidgetCB& acpLoadCB, const TWidgetCB& acpCancelCB)
{
    auto popupTitle = acpOwnerName.empty() ? "Unsaved changes" : fmt::format("{} - Unsaved changes", acpOwnerName);

    if (acMadeChanges)
    {
        auto res = TChangedCBResult::CHANGED;
        if (aFirstTime)
        {

            ImGui::OpenPopup(popupTitle.c_str());
            aFirstTime = false;
        }

        if (ImGui::BeginPopupModal(popupTitle.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            const auto shorterTextSz{ImGui::CalcTextSize("You have some unsaved changes.").x};
            const auto longerTextSz{ImGui::CalcTextSize("Do you wish to apply them or discard them?").x};
            const auto diffTextSz{longerTextSz - shorterTextSz};

            ImGui::SetCursorPosX(diffTextSz / 2);
            ImGui::TextUnformatted("You have some unsaved changes.");
            ImGui::TextUnformatted("Do you wish to apply them or discard them?");
            ImGui::Separator();

            const auto itemWidth = GetAlignedItemWidth(3);

            if (ImGui::Button("Apply", ImVec2(itemWidth, 0)))
            {
                if (acpSaveCB)
                    acpSaveCB();
                res = TChangedCBResult::APPLY;
                aFirstTime = true;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Discard", ImVec2(itemWidth, 0)))
            {
                if (acpLoadCB)
                    acpLoadCB();
                res = TChangedCBResult::DISCARD;
                aFirstTime = true;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(itemWidth, 0)))
            {
                if (acpCancelCB)
                    acpCancelCB();
                res = TChangedCBResult::CANCEL;
                aFirstTime = true;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SetItemDefaultFocus();

            ImGui::EndPopup();
        }
        return res;
    }
    return TChangedCBResult::APPLY; // no changes, same as if we were to Apply
}

std::filesystem::path GetAbsolutePath(const std::string& acFilePath, const std::filesystem::path& acRootPath, const bool acAllowNonExisting, const bool acAllowSymlink)
{
    return GetAbsolutePath(UTF8ToUTF16(acFilePath), acRootPath, acAllowNonExisting, acAllowSymlink);
}

std::filesystem::path GetAbsolutePath(std::filesystem::path aFilePath, const std::filesystem::path& acRootPath, const bool acAllowNonExisting, const bool acAllowSymlink)
{
    assert(!aFilePath.empty());
    if (aFilePath.empty())
        return {};

    aFilePath.make_preferred();

    if (aFilePath.is_relative())
    {
        if (!acRootPath.empty())
            aFilePath = acRootPath / aFilePath;

        aFilePath = absolute(aFilePath);
    }

    if (is_symlink(aFilePath))
    {
        if (acAllowSymlink)
            return absolute(read_symlink(aFilePath));

        return {};
    }

    if (!exists(aFilePath))
    {
        if (!acAllowNonExisting)
            return {};
    }

    return aFilePath;
}

std::filesystem::path GetLuaPath(const std::string& acFilePath, const std::filesystem::path& acRootPath, const bool acAllowNonExisting)
{
    return GetLuaPath(UTF8ToUTF16(acFilePath), acRootPath, acAllowNonExisting);
}

std::filesystem::path GetLuaPath(std::filesystem::path aFilePath, const std::filesystem::path& acRootPath, const bool acAllowNonExisting)
{
    assert(!aFilePath.empty());
    assert(!aFilePath.is_absolute());

    if (aFilePath.empty() || aFilePath.is_absolute())
        return {};

    aFilePath.make_preferred();

    if (aFilePath.native().starts_with(L"..\\"))
        return {};

    aFilePath = GetAbsolutePath(aFilePath, acRootPath, acAllowNonExisting, false);
    if (aFilePath.empty())
        return {};

    const auto relativeFilePathToRoot = relative(aFilePath, acRootPath);
    if (relativeFilePathToRoot.native().starts_with(L"..\\") || relativeFilePathToRoot.native().find(L"\\..\\") != std::wstring::npos)
        return {};

    return relative(aFilePath, std::filesystem::current_path());
}

std::vector<char> ReadWholeBinaryFile(const std::filesystem::path& acpPath)
{
    if (acpPath.empty() || !exists(acpPath))
        return {};

    std::ifstream file(acpPath, std::ios::binary);
    file.exceptions(std::ios::badbit | std::ios::failbit | std::ios::eofbit);
    std::vector<char> bytes(std::istreambuf_iterator{file}, {});
    file.close();

    return bytes;
}

std::string ReadWholeTextFile(const std::filesystem::path& acpPath)
{
    if (acpPath.empty() || !exists(acpPath))
        return {};

    std::ifstream file(acpPath, std::ios::binary);
    file.exceptions(std::ios::badbit | std::ios::failbit | std::ios::eofbit);
    std::string lines(std::istreambuf_iterator{file}, {});
    file.close();

    return lines;
}
