#pragma once

#include <windows.h>
#include <filesystem>
#include "Image.h"

struct Options
{
	static LPCSTR TargetAppWindowTitle;

	Image GameImage;
	static std::filesystem::path Path;
	static std::string ExeName;

	static bool IsCyberpunk2077(); /*const noexcept;*/
	static void Initialize(HMODULE aModule);
	static Options& Get();


	bool PatchSMT{ false };
	bool PatchAVX{ false };
	bool PatchVirtualInput{ false };
	bool PatchMemoryPool{ false };
	bool PatchEnableDebug{ false };
	bool PatchRemovePedestrians{ false };
	bool PatchAsyncCompute{ false };
	bool PatchAntialiasing{ false };
	bool PatchSkipStartMenu{ false };
	bool PatchDisableIntroMovies{ false };
	bool PatchDisableVignette{ false };
	bool PatchDisableBoundaryTeleport{ false };

	float CPUMemoryPoolFraction{ 0.5f };
	float GPUMemoryPoolFraction{ 1.f };
	bool DumpGameOptions{ false };
		
	bool Console{ true };
	int ConsoleKey{ VK_OEM_3 };
	bool Uninject{ false };
	int ConsoleUninjectKey{ VK_SUBTRACT };	
	int ConsoleChar{ 0 };

private:

	Options(HMODULE aModule);
};