#include "ImGuiRenderer.h"

#include "ImGuiMenu.h"
#include "Settings.h"

#include <d3d11.h>
#include <dxgi.h>

#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

#include <MinHook.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace ImGuiRenderer
{
	using PFN_Present = HRESULT(WINAPI*)(IDXGISwapChain*, UINT, UINT);
	using PFN_ResizeBuffers = HRESULT(WINAPI*)(IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT);

	static PFN_Present oPresent = nullptr;
	static PFN_ResizeBuffers oResizeBuffers = nullptr;
	static WNDPROC oWndProc = nullptr;

	static ID3D11Device* g_device = nullptr;
	static ID3D11DeviceContext* g_context = nullptr;
	static ID3D11RenderTargetView* g_rtv = nullptr;
	static HWND g_hwnd = nullptr;

	static bool g_initialized = false;
	static bool g_menuOpen = false;

	static void CreateRenderTarget(IDXGISwapChain* a_swapChain)
	{
		ID3D11Texture2D* backBuffer = nullptr;
		a_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
		if (backBuffer) {
			g_device->CreateRenderTargetView(backBuffer, nullptr, &g_rtv);
			backBuffer->Release();
		}
	}

	static void CleanupRenderTarget()
	{
		if (g_rtv) {
			g_rtv->Release();
			g_rtv = nullptr;
		}
	}

	static UINT KeyNameToVK(const std::string& a_key)
	{
		static const std::unordered_map<std::string, UINT> keyMap = {
			{"f1", VK_F1}, {"f2", VK_F2}, {"f3", VK_F3}, {"f4", VK_F4},
			{"f5", VK_F5}, {"f6", VK_F6}, {"f7", VK_F7}, {"f8", VK_F8},
			{"f9", VK_F9}, {"f10", VK_F10}, {"f11", VK_F11}, {"f12", VK_F12},
			{"insert", VK_INSERT}, {"ins", VK_INSERT},
			{"delete", VK_DELETE}, {"del", VK_DELETE},
			{"home", VK_HOME}, {"end", VK_END},
			{"pageup", VK_PRIOR}, {"pgup", VK_PRIOR},
			{"pagedown", VK_NEXT}, {"pgdn", VK_NEXT},
			{"pause", VK_PAUSE}, {"break", VK_PAUSE},
			{"scrolllock", VK_SCROLL},
			{"numlock", VK_NUMLOCK},
			{"backspace", VK_BACK},
			{"numpad0", VK_NUMPAD0}, {"numpad1", VK_NUMPAD1},
			{"numpad2", VK_NUMPAD2}, {"numpad3", VK_NUMPAD3},
			{"numpad4", VK_NUMPAD4}, {"numpad5", VK_NUMPAD5},
			{"numpad6", VK_NUMPAD6}, {"numpad7", VK_NUMPAD7},
			{"numpad8", VK_NUMPAD8}, {"numpad9", VK_NUMPAD9},
		};

		std::string lower = a_key;
		std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

		auto it = keyMap.find(lower);
		if (it != keyMap.end()) {
			return it->second;
		}
		return VK_F10;
	}

	static std::atomic<UINT> s_cachedToggleVK{ VK_DELETE };

	void RefreshToggleKey()
	{
		const auto settings = Settings::Main::GetSingleton();
		ReadLocker locker(settings->Lock);
		s_cachedToggleVK.store(KeyNameToVK(*settings->ToggleMenuKey), std::memory_order_relaxed);
	}

	static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		if (msg == WM_KEYDOWN && wParam == s_cachedToggleVK.load(std::memory_order_relaxed) && !(lParam & 0x40000000)) {
			g_menuOpen = !g_menuOpen;
			if (g_menuOpen) {
				ClipCursor(nullptr);  // Free cursor on menu
			}
			return 0;
		}

		if (g_menuOpen) {
			// Forward input to ImGui
			ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);

			// Block game input
			switch (msg) {
			case WM_MOUSEMOVE:
			case WM_LBUTTONDOWN:
			case WM_LBUTTONUP:
			case WM_LBUTTONDBLCLK:
			case WM_RBUTTONDOWN:
			case WM_RBUTTONUP:
			case WM_RBUTTONDBLCLK:
			case WM_MBUTTONDOWN:
			case WM_MBUTTONUP:
			case WM_MBUTTONDBLCLK:
			case WM_MOUSEWHEEL:
			case WM_MOUSEHWHEEL:
			case WM_KEYDOWN:
			case WM_KEYUP:
			case WM_SYSKEYDOWN:
			case WM_SYSKEYUP:
			case WM_CHAR:
			case WM_SYSCHAR:
			case WM_INPUT:
				return 0;
			}
		}

		return CallWindowProc(oWndProc, hWnd, msg, wParam, lParam);
	}

	static void InitImGui(IDXGISwapChain* a_swapChain)
	{
		DXGI_SWAP_CHAIN_DESC desc;
		a_swapChain->GetDesc(&desc);
		g_hwnd = desc.OutputWindow;

		a_swapChain->GetDevice(IID_PPV_ARGS(&g_device));
		g_device->GetImmediateContext(&g_context);

		CreateRenderTarget(a_swapChain);

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.IniFilename = nullptr;  // Don't save imgui.ini
		io.MouseDrawCursor = false;

		ImGuiMenu::ApplyBG3Theme();

		ImGui_ImplWin32_Init(g_hwnd);
		ImGui_ImplDX11_Init(g_device, g_context);

		oWndProc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(g_hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WndProc)));

		g_initialized = true;
		RefreshToggleKey();

		INFO("ImGui initialized successfully")
	}

	static HRESULT WINAPI HookPresent(IDXGISwapChain* a_swapChain, UINT a_syncInterval, UINT a_flags)
	{
		if (!g_initialized) {
			InitImGui(a_swapChain);
		}

		if (g_menuOpen) {
			ImGui_ImplDX11_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();

			ImGui::GetIO().MouseDrawCursor = true;

			ImGuiMenu::Draw();

			ImGui::Render();
			g_context->OMSetRenderTargets(1, &g_rtv, nullptr);
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		} else {
			ImGui::GetIO().MouseDrawCursor = false;
		}

		return oPresent(a_swapChain, a_syncInterval, a_flags);
	}

	static HRESULT WINAPI HookResizeBuffers(IDXGISwapChain* a_swapChain, UINT a_bufferCount, UINT a_width, UINT a_height, DXGI_FORMAT a_format, UINT a_flags)
	{
		CleanupRenderTarget();

		HRESULT hr = oResizeBuffers(a_swapChain, a_bufferCount, a_width, a_height, a_format, a_flags);

		CreateRenderTarget(a_swapChain);

		return hr;
	}

	void Init()
	{
		INFO("Initializing ImGui DX11 hook...")

		// Temporary window for dummy device
		WNDCLASSEX wc = {};
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.style = CS_CLASSDC;
		wc.lpfnWndProc = DefWindowProc;
		wc.hInstance = GetModuleHandle(nullptr);
		wc.lpszClassName = L"BG3CamTweaksImGuiTemp";
		RegisterClassEx(&wc);

		HWND tempHwnd = CreateWindow(wc.lpszClassName, L"", WS_OVERLAPPEDWINDOW, 0, 0, 100, 100, nullptr, nullptr, wc.hInstance, nullptr);
		if (!tempHwnd) {
			ERROR("ImGui Init: Failed to create temporary window")
			UnregisterClass(wc.lpszClassName, wc.hInstance);
			return;
		}

		// Temporary D3D11 device to get vtable
		DXGI_SWAP_CHAIN_DESC sd = {};
		sd.BufferCount = 1;
		sd.BufferDesc.Width = 2;
		sd.BufferDesc.Height = 2;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = tempHwnd;
		sd.SampleDesc.Count = 1;
		sd.Windowed = TRUE;
		sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

		IDXGISwapChain* tempSwapChain = nullptr;
		ID3D11Device* tempDevice = nullptr;
		ID3D11DeviceContext* tempContext = nullptr;

		D3D_FEATURE_LEVEL featureLevel;
		D3D_FEATURE_LEVEL requestedLevels[] = { D3D_FEATURE_LEVEL_11_0 };

		HRESULT hr = D3D11CreateDeviceAndSwapChain(
			nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
			requestedLevels, 1, D3D11_SDK_VERSION,
			&sd, &tempSwapChain, &tempDevice, &featureLevel, &tempContext);

		if (FAILED(hr)) {
			ERROR("ImGui Init: D3D11CreateDeviceAndSwapChain failed (HRESULT: 0x{:X})", static_cast<uint32_t>(hr))
			DestroyWindow(tempHwnd);
			UnregisterClass(wc.lpszClassName, wc.hInstance);
			return;
		}

		// Get vtable from swap chain
		void** swapChainVtable = *reinterpret_cast<void***>(tempSwapChain);
		void* presentAddr = swapChainVtable[8];       // IDXGISwapChain::Present
		void* resizeBuffersAddr = swapChainVtable[13]; // IDXGISwapChain::ResizeBuffers

		// Clean up
		tempContext->Release();
		tempSwapChain->Release();
		tempDevice->Release();
		DestroyWindow(tempHwnd);
		UnregisterClass(wc.lpszClassName, wc.hInstance);

		// Install hooks
		if (MH_Initialize() != MH_OK) {
			ERROR("ImGui Init: MH_Initialize failed")
			return;
		}

		if (MH_CreateHook(presentAddr, &HookPresent, reinterpret_cast<void**>(&oPresent)) != MH_OK) {
			ERROR("ImGui Init: Failed to hook Present")
			return;
		}

		if (MH_CreateHook(resizeBuffersAddr, &HookResizeBuffers, reinterpret_cast<void**>(&oResizeBuffers)) != MH_OK) {
			ERROR("ImGui Init: Failed to hook ResizeBuffers")
			return;
		}

		if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK) {
			ERROR("ImGui Init: Failed to enable hooks")
			return;
		}

		INFO("ImGui DX11 hooks installed successfully")
	}

	void Shutdown()
	{
		MH_DisableHook(MH_ALL_HOOKS);
		MH_Uninitialize();

		if (g_initialized) {
			if (oWndProc && g_hwnd) {
				SetWindowLongPtr(g_hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(oWndProc));
			}
			ImGui_ImplDX11_Shutdown();
			ImGui_ImplWin32_Shutdown();
			ImGui::DestroyContext();
			CleanupRenderTarget();
			if (g_context) {
				g_context->Release();
				g_context = nullptr;
			}
			if (g_device) {
				g_device->Release();
				g_device = nullptr;
			}
			g_initialized = false;
		}
	}

	bool IsMenuOpen()
	{
		return g_menuOpen;
	}
}
