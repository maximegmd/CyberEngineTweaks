#include <stdafx.h>

#include "Utils.h"

#include <LzmaLib.h>
#include <spdlog/sinks/base_sink.h>
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

template<typename Mutex>
class CustomSink : public spdlog::sinks::base_sink<Mutex>
{
public:
    CustomSink(std::function<void(const std::string&)> aSinkItHandler, std::function<void()> aFlushHandler)
        : spdlog::sinks::base_sink<Mutex>()
        , m_sinkItHandler(aSinkItHandler)
        , m_flushHandler(aFlushHandler)
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

template<typename Mutex>
spdlog::sink_ptr CreateCustomSink(std::function<void(const std::string&)> aSinkItHandler,
                                  std::function<void()> aFlushHandler)
{
    return std::make_shared<CustomSink<Mutex>>(aSinkItHandler, aFlushHandler);
}

spdlog::sink_ptr CreateCustomSinkST(std::function<void(const std::string&)> aSinkItHandler,
                                    std::function<void()> aFlushHandler)
{
    return CreateCustomSink<spdlog::details::null_mutex>(aSinkItHandler, aFlushHandler);
}

spdlog::sink_ptr CreateCustomSinkMT(std::function<void(const std::string&)> aSinkItHandler,
                                    std::function<void()> aFlushHandler)
{
    return CreateCustomSink<std::mutex>(aSinkItHandler, aFlushHandler);
}

std::shared_ptr<spdlog::logger> CreateLogger(const std::filesystem::path& aPath, const std::string& aID,
                                             spdlog::sink_ptr aExtraSink, const std::string& aPattern)
{
    auto existingLogger = spdlog::get(aID);
    if (existingLogger)
        return existingLogger;

    const auto rotSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(aPath.native(), 1048576 * 5, 3);
    rotSink->set_pattern(aPattern);
    auto logger = std::make_shared<spdlog::logger>(aID, spdlog::sinks_init_list{rotSink});

    if (aExtraSink)
        logger->sinks().emplace_back(aExtraSink);

#ifdef CET_DEBUG
    logger->flush_on(spdlog::level::trace);
#else
    logger->flush_on(spdlog::level::err);
#endif

    register_logger(logger);
    return logger;
}

// deep copies sol object (doesnt take into account potential duplicates)
sol::object DeepCopySolObject(sol::object aObj, const sol::state_view& aStateView)
{
    if ((aObj == sol::nil) || (aObj.get_type() != sol::type::table))
        return aObj;
    sol::table src{aObj.as<sol::table>()};
    sol::table copy{aStateView, sol::create};
    for (auto kv : src)
        copy[DeepCopySolObject(kv.first, aStateView)] = DeepCopySolObject(kv.second, aStateView);
    copy[sol::metatable_key] = src[sol::metatable_key];
    return copy;
}

// makes sol usertype or userdata immutable when accessed from lua
void MakeSolUsertypeImmutable(sol::object aObj, const sol::state_view& aStateView)
{
    if (!aObj.is<sol::metatable>() && !aObj.is<sol::userdata>())
        return;

    sol::table target = aObj;
    sol::table metatable;
    sol::object metaref = target[sol::metatable_key];

    if (metaref.is<sol::table>())
    {
        metatable = metaref;
    }
    else
    {
        metatable = {aStateView, sol::create};
        target[sol::metatable_key] = metatable;
    }

    // prevent adding new properties
    metatable[sol::meta_function::new_index] = []() {};

    // prevent overriding metatable
    metatable[sol::meta_function::metatable] = []() { return sol::nil; };
}

// Check if Lua object is of cdata type
bool IsLuaCData(sol::object aObject)
{
    // Sol doesn't have enum for LuaJIT's cdata type since it's not a standard type.
    // But it's possible to check the type using numeric code (10).
    // LuaJIT packs int64/uint64 into cdata and some other types.
    // Since we're not using other types, this should be enough to check for int64/uint64 value.
    return static_cast<int>(aObject.get_type()) == 10;
}

float GetAlignedItemWidth(int64_t aItemsCount)
{
    return (ImGui::GetWindowContentRegionWidth() - static_cast<float>(aItemsCount - 1) * ImGui::GetStyle().ItemSpacing.x) / static_cast<float>(aItemsCount);
}

float GetCenteredOffsetForText(const char* acpText)
{
    return (ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(acpText).x) / 2.0f;
}

THWUCPResult UnsavedChangesPopup(bool& aFirstTime, bool aMadeChanges, TWidgetCB aSaveCB, TWidgetCB aLoadCB, TWidgetCB aCancelCB)
{
    if (aMadeChanges)
    {
        auto res = THWUCPResult::CHANGED;
        if (aFirstTime)
        {
            ImGui::OpenPopup("Unsaved changes");
            aFirstTime = false;
        }

        if (ImGui::BeginPopupModal("Unsaved changes", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            const auto shorterTextSz { ImGui::CalcTextSize("You have some unsaved changes.").x };
            const auto longerTextSz { ImGui::CalcTextSize("Do you wish to apply them or discard them?").x };
            const auto diffTextSz { longerTextSz - shorterTextSz };

            ImGui::SetCursorPosX(diffTextSz / 2);
            ImGui::TextUnformatted("You have some unsaved changes.");
            ImGui::TextUnformatted("Do you wish to apply them or discard them?");
            ImGui::Separator();

            const auto itemWidth = GetAlignedItemWidth(3);

            if (ImGui::Button("Apply", ImVec2(itemWidth, 0)))
            {
                if (aSaveCB)
                    aSaveCB();
                res = THWUCPResult::APPLY;
                aFirstTime = true;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Discard", ImVec2(itemWidth, 0)))
            {
                if (aLoadCB)
                    aLoadCB();
                res = THWUCPResult::DISCARD;
                aFirstTime = true;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(itemWidth, 0)))
            {
                if (aCancelCB)
                    aCancelCB();
                res = THWUCPResult::CANCEL;
                aFirstTime = true;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SetItemDefaultFocus();

            ImGui::EndPopup();
        }
        return res;
    }
    return THWUCPResult::APPLY; // no changes, same as if we were to Apply
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

    if (!exists(aFilePath))
    {
        if (!acAllowNonExisting)
            return {};
    }
    else if (is_symlink(aFilePath))
    {
        if (acAllowSymlink)
            return absolute(read_symlink(aFilePath));

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

std::vector<uint8_t> ReadWholeBinaryFile(const std::filesystem::path& acpPath)
{
    if (acpPath.empty() || !exists(acpPath))
        return {};

    std::ifstream file(acpPath, std::ios::binary);
    std::vector<uint8_t> bytes(std::istreambuf_iterator{file}, {});
    file.close();

    return bytes;
}

std::string ReadWholeTextFile(const std::filesystem::path& acpPath)
{
    if (acpPath.empty() || !exists(acpPath))
        return {};

    std::ifstream file(acpPath, std::ios::binary);
    std::string lines(std::istreambuf_iterator{file}, {});
    file.close();

    return lines;
}

std::vector<uint8_t> EncodeToLzma(const std::filesystem::path& acpPath)
{
    return EncodeToLzma(ReadWholeBinaryFile(acpPath));
}

std::vector<uint8_t> EncodeToLzma(const std::vector<uint8_t>& acpIn)
{
    size_t propsSize = LZMA_PROPS_SIZE;
    size_t destLen = acpIn.size() + acpIn.size() / 3 + 128;
    std::vector<uint8_t> outBuf;
    outBuf.resize(propsSize + destLen + sizeof(uint64_t));

    const auto res = LzmaCompress(
        &outBuf[LZMA_PROPS_SIZE + sizeof(uint64_t)], &destLen,
        &acpIn[0], acpIn.size(),
        &outBuf[sizeof(uint64_t)], &propsSize,
        -1, 0, -1, -1, -1, -1, -1);

    assert(propsSize == LZMA_PROPS_SIZE);
    assert(res == SZ_OK);

    const uint64_t finalSize = acpIn.size();
    std::memcpy(outBuf.data(), &finalSize, sizeof(uint64_t));

    outBuf.resize(propsSize + destLen + sizeof(uint64_t));

    return outBuf;
}

std::vector<uint8_t> DecodeFromLzma(const std::filesystem::path& acpPath)
{
    return DecodeFromLzma(ReadWholeBinaryFile(acpPath));
}

std::vector<uint8_t> DecodeFromLzma(const std::vector<uint8_t>& acpIn)
{
    uint64_t decodedSize = 0;
    std::memcpy(&decodedSize, acpIn.data(), sizeof(uint64_t));

    std::vector<uint8_t> outBuf;
    outBuf.resize(decodedSize);

    size_t srcLen = acpIn.size() - LZMA_PROPS_SIZE;
    SRes res = LzmaUncompress(
        &outBuf[0], &decodedSize,
        &acpIn[LZMA_PROPS_SIZE + sizeof(uint64_t)], &srcLen,
        &acpIn[sizeof(uint64_t)], LZMA_PROPS_SIZE);
        assert(res == SZ_OK);

    return outBuf;
}

