#pragma once

#include <windows.h>
#include <filesystem>

struct Image;
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
	bool PatchSkipStartMenu{ true };
	float CPUMemoryPoolFraction{ 0.5f };
	float GPUMemoryPoolFraction{ 1.f };
	std::filesystem::path Path;
	std::string ExeName;

private:

	Options(HMODULE aModule);
};