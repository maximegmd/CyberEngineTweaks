#pragma once

#include <windows.h>
#include <filesystem>

struct Image;
struct Options
{
	Options(HMODULE aModule);

	bool PatchSpectre { true };
	bool PatchSMT{ true };
	bool PatchAVX{ false };
	bool PatchVirtualInput{ true };
	bool PatchMemoryPool{ true };
	bool PatchUnlockMenu{ true };
	std::filesystem::path Path;
};