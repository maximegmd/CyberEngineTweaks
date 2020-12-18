#pragma once

#include <windows.h>
#include <filesystem>
#include "Image.h"

struct Options
{
	static void Initialize(HMODULE aModule);
	static Options& Get();
	
	bool IsCyberpunk2077() const noexcept;

	bool PatchSpectre { true };
	bool PatchSMT{ true };
	bool PatchAVX{ true };
	bool PatchVirtualInput{ true };
	bool PatchMemoryPool{ true };
	bool PatchUnlockMenu{ false };
	bool PatchRemovePedestrians{ false };
	bool PatchAsyncCompute{ false };
	bool PatchAntialiasing{ false };
	bool PatchSkipStartMenu{ true };
	bool DumpGameOptions{ false };
	bool Console{ true };
	float CPUMemoryPoolFraction{ 0.5f };
	float GPUMemoryPoolFraction{ 1.f };
	std::filesystem::path Path;
	std::string ExeName;
	Image GameImage;

private:

	Options(HMODULE aModule);
};