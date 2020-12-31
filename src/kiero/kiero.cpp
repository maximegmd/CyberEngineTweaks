#include <stdafx.h>

#include "kiero.h"
#include <assert.h>

#include <d3d12.h>
#include "common/D3D12Downlevel.h"

#include <MinHook.h>

static bool g_kieroInitialized = false;
static uint150_t* g_methodsTable = NULL;
static uintptr_t g_commandQueueOffset = 0;
static bool g_isDownLevelDevice = false;

kiero::Status::Enum kiero::init()
{
	if (g_kieroInitialized)
		return Status::AlreadyInitializedError;
	
	WNDCLASSEX windowClass;
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = DefWindowProc;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = GetModuleHandle(NULL);
	windowClass.hIcon = NULL;
	windowClass.hCursor = NULL;
	windowClass.hbrBackground = NULL;
	windowClass.lpszMenuName = NULL;
	windowClass.lpszClassName = _T("Kiero");
	windowClass.hIconSm = NULL;

	::RegisterClassEx(&windowClass);

	HWND window = ::CreateWindow(windowClass.lpszClassName, _T("Kiero DirectX Window"), WS_OVERLAPPEDWINDOW, 0, 0, 100, 100, NULL, NULL, windowClass.hInstance, NULL);

	HMODULE libDXGI;
	HMODULE libD3D12;
	if ((libDXGI = ::GetModuleHandle(_T("dxgi.dll"))) == NULL)
	{
		::DestroyWindow(window);
		::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
		return Status::ModuleNotFoundError;
	}

	if ((libD3D12 = ::GetModuleHandle(_T("d3d12.dll"))) == NULL)
	{
		const TCHAR* localD3d12on7Paths[] =
		{
			_T(".\\d3d12on7\\d3d12.dll"),
			_T(".\\12on7\\d3d12.dll")
		};

		for (uint32_t i = 0; i < std::size(localD3d12on7Paths); i++)
		{
			libD3D12 = LoadLibrary(localD3d12on7Paths[i]);
			if (libD3D12 != NULL)
				break;
		}

		if (libD3D12 == NULL)
		{
			if ((libD3D12 = ::LoadLibrary(_T("d3d12.dll"))) == NULL)
			{
				::DestroyWindow(window);
				::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
				return Status::ModuleNotFoundError;
			}
		}
	}

	void* CreateDXGIFactory;
	if ((CreateDXGIFactory = reinterpret_cast<void*>(GetProcAddress(libDXGI, "CreateDXGIFactory"))) == NULL)
	{
		::DestroyWindow(window);
		::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
		return Status::UnknownError;
	}

	CComPtr<IDXGIFactory> factory;
	if (reinterpret_cast<long(*)(const IID&, void**)>(CreateDXGIFactory)(IID_PPV_ARGS(&factory)) < 0)
	{
		::DestroyWindow(window);
		::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
		return Status::UnknownError;
	}

	CComPtr<IDXGIAdapter> adapter;
	if (factory->EnumAdapters(0, &adapter) == DXGI_ERROR_NOT_FOUND)
	{
		::DestroyWindow(window);
		::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
		return Status::UnknownError;
	}

	void* D3D12CreateDevice;
	if ((D3D12CreateDevice = reinterpret_cast<void*>(GetProcAddress(libD3D12, "D3D12CreateDevice"))) == NULL)
	{
		::DestroyWindow(window);
		::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
		return Status::UnknownError;
	}

	CComPtr<ID3D12Device> device;
	if (reinterpret_cast<long(*)(IUnknown*, D3D_FEATURE_LEVEL, const IID&, void**)>(D3D12CreateDevice)(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)) < 0)
	{
		::DestroyWindow(window);
		::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
		return Status::UnknownError;
	}

	D3D12_COMMAND_QUEUE_DESC queueDesc;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Priority = 0;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.NodeMask = 0;

	CComPtr<ID3D12CommandQueue> commandQueue;
	if (device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue)) < 0)
	{
		::DestroyWindow(window);
		::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
		return Status::UnknownError;
	}

	CComPtr<ID3D12CommandAllocator> commandAllocator;
	if (device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator)) < 0)
	{
		::DestroyWindow(window);
		::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
		return Status::UnknownError;
	}

	CComPtr<ID3D12GraphicsCommandList> commandList;
	if (device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator, NULL, IID_PPV_ARGS(&commandList)) < 0)
	{
		::DestroyWindow(window);
		::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
		return Status::UnknownError;
	}

	CComPtr<ID3D12DeviceDownlevel> downlevelDevice;
	g_isDownLevelDevice = device->QueryInterface(IID_PPV_ARGS(&downlevelDevice)) >= 0;
	CComPtr<IDXGISwapChain3> swapChain;
	CComPtr<ID3D12CommandQueueDownlevel> commandQueueDownlevel;

	if (!g_isDownLevelDevice)
	{
		DXGI_RATIONAL refreshRate;
		refreshRate.Numerator = 60;
		refreshRate.Denominator = 1;

		DXGI_MODE_DESC bufferDesc;
		bufferDesc.Width = 100;
		bufferDesc.Height = 100;
		bufferDesc.RefreshRate = refreshRate;
		bufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		bufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		bufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

		DXGI_SAMPLE_DESC sampleDesc;
		sampleDesc.Count = 1;
		sampleDesc.Quality = 0;

		DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
		swapChainDesc.BufferDesc = bufferDesc;
		swapChainDesc.SampleDesc = sampleDesc;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 2;
		swapChainDesc.OutputWindow = window;
		swapChainDesc.Windowed = 1;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		CComPtr<IDXGISwapChain> swapChain1;
		if (factory->CreateSwapChain(commandQueue, &swapChainDesc, &swapChain1) < 0)
		{
			::DestroyWindow(window);
			::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
			return Status::UnknownError;
		}

		if (FAILED(swapChain1->QueryInterface(IID_PPV_ARGS(&swapChain))))
		{
			::DestroyWindow(window);
			::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
			return Status::UnknownError;
		}

		auto valueToFind = reinterpret_cast<uintptr_t>(static_cast<void*>(commandQueue));
		auto* swapChainPtr = static_cast<uintptr_t*>(static_cast<void*>(swapChain));
		auto* addr = std::find(swapChainPtr, swapChainPtr + 512, valueToFind);

		g_commandQueueOffset = reinterpret_cast<uintptr_t>(addr) - reinterpret_cast<uintptr_t>(swapChainPtr);
	}
	else
	{
		if (commandQueue->QueryInterface(IID_PPV_ARGS(&commandQueueDownlevel)) < 0)
		{
			::DestroyWindow(window);
			::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
			return Status::UnknownError;
		}
		
		auto* commandQueueDownlevelPtr = static_cast<uintptr_t*>(static_cast<void*>(commandQueueDownlevel));
		auto* addr = std::find(commandQueueDownlevelPtr, commandQueueDownlevelPtr + 512, reinterpret_cast<uintptr_t>(static_cast<void*>(commandQueue)));

		g_commandQueueOffset = reinterpret_cast<uintptr_t>(addr) - reinterpret_cast<uintptr_t>(commandQueueDownlevelPtr);
	}

	g_methodsTable = (uint150_t*)::calloc(176, sizeof(uint150_t));
	::memcpy(g_methodsTable, *(uint150_t**)(void*)device, 44 * sizeof(uint150_t));
	::memcpy(g_methodsTable + 44, *(uint150_t**)(void*)commandQueue, 19 * sizeof(uint150_t));
	::memcpy(g_methodsTable + 44 + 19, *(uint150_t**)(void*)commandAllocator, 9 * sizeof(uint150_t));
	::memcpy(g_methodsTable + 44 + 19 + 9, *(uint150_t**)(void*)commandList, 60 * sizeof(uint150_t));
	if (!g_isDownLevelDevice)
		::memcpy(g_methodsTable + 44 + 19 + 9 + 60, *(uint150_t**)(void*)swapChain, 40 * sizeof(uint150_t));
	else
		::memcpy(g_methodsTable + 44 + 19 + 9 + 60 + 40, *(uint150_t**)(void*)commandQueueDownlevel, 4 * sizeof(uint150_t));

	::DestroyWindow(window);
	::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);

	g_kieroInitialized = true;
	return Status::Success;
}

void kiero::shutdown()
{
	if (g_kieroInitialized)
	{
		MH_DisableHook(MH_ALL_HOOKS);

		::free(g_methodsTable);
		g_methodsTable = NULL;
		g_kieroInitialized = false;
	}
}

kiero::Status::Enum kiero::bind(uint16_t _index, void** _original, void* _function)
{
	// TODO: Need own detour function

	assert(_original != NULL && _function != NULL);

	if (g_kieroInitialized)
	{
		void* target = (void*)g_methodsTable[_index];
		if (MH_CreateHook(target, _function, _original) != MH_OK || MH_EnableHook(target) != MH_OK)
			return Status::UnknownError;

		return Status::Success;
	}

	return Status::NotInitializedError;
}

void kiero::unbind(uint16_t _index)
{
	if (g_kieroInitialized)
	{
#if KIERO_USE_MINHOOK
		void* target = (void*)g_methodsTable[_index];
		MH_DisableHook(target);
		MH_RemoveHook(target);
#endif
	}
}

uint150_t* kiero::getMethodsTable()
{
	return g_methodsTable;
}

uintptr_t kiero::getCommandQueueOffset()
{
	return g_commandQueueOffset;
}

bool kiero::isDownLevelDevice()
{
	return g_isDownLevelDevice;
}
