#pragma once

#include <windows.h>
#include <filesystem>
#include "Image.h"

struct Options
{
	static void Initialize(HMODULE aModule);
	static Options& Get();
	
	bool IsCyberpunk2077() const noexcept;

	bool PatchSMT{ false };
	bool PatchAVX{ true };
	bool PatchVirtualInput{ true };
	bool PatchMemoryPool{ true };
	bool PatchEnableDebug{ false };
	bool PatchRemovePedestrians{ false };
	bool PatchAsyncCompute{ false };
	bool PatchAntialiasing{ false };
	bool PatchSkipStartMenu{ true };
	bool PatchDisableIntroMovies{ false };
	bool PatchDisableVignette{ false };
	bool PatchDisableBoundaryTeleport{ false };

	bool DumpGameOptions{ false };
	bool Console{ true };
	int ConsoleKey{ VK_OEM_3 };
	int ConsoleChar{ 0 };
	float CPUMemoryPoolFraction{ 0.5f };
	float GPUMemoryPoolFraction{ 1.f };
	std::filesystem::path Path;
	std::string ExeName;
	Image GameImage;

private:

	Options(HMODULE aModule);
};