#pragma once

#include <windows.h>
#include <filesystem>

struct Image;
struct Options
{
	Options(HMODULE aModule);

	bool IsCyberpunk2077() const noexcept;

	bool PatchSpectre { true };
	bool PatchSMT{ true };
	bool PatchAVX{ false };
	bool PatchVirtualInput{ true };
	bool PatchMemoryPool{ true };
	bool PatchUnlockMenu{ false };
	std::filesystem::path Path;
	std::string ExeName;
};