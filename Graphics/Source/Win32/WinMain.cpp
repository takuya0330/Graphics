﻿#include "CoreApp.h"

#if 0
#elif _WIN32 && _RHI
#include "RHI/RHIApp.h"
#elif _WIN32 && _D3D12 && _TEXTURE
#include "D3D12/D3D12TextureApp.h"
#elif _WIN32 && _D3D12 && _TRIANGLE
#include "D3D12/D3D12TriangleApp.h"
#elif _WIN32 && _D3D12 && _IMGUI
#include "D3D12/D3D12ImGuiApp.h"
#elif _WIN32 && _D3D12
#include "D3D12/D3D12App.h"
#elif _WIN32 && _D3D11 && _TEXTURE
#include "D3D11/D3D11TextureApp.h"
#elif _WIN32 && _D3D11 && _TRIANGLE
#include "D3D11/D3D11TriangleApp.h"
#elif _WIN32 && _D3D11 && _IMGUI
#include "D3D11/D3D11ImGuiApp.h"
#elif _WIN32 && _D3D11
#include "D3D11/D3D11App.h"
#elif _WIN32
#include "Win32App.h"
#endif

#include <crtdbg.h>

#include <memory>

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PWSTR pCmdLine, _In_ int nCmdShow)
{
#if defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	constexpr UINT kWidth  = 1280;
	constexpr UINT kHeight = 720;

	int ret = 0;
	{
		std::unique_ptr<CoreApp> app;
#if 0
#elif _WIN32 && _RHI
		app = std::make_unique<RHIApp>(L"RHI App", kWidth, kHeight);
#elif _WIN32 && _D3D12 && _TEXTURE
		app = std::make_unique<D3D12TextureApp>(L"D3D12 Texture", kWidth, kHeight);
#elif _WIN32 && _D3D12 && _TRIANGLE
		app = std::make_unique<D3D12TriangleApp>(L"D3D12 Triangle", kWidth, kHeight);
#elif _WIN32 && _D3D12 && _IMGUI
		app = std::make_unique<D3D12ImGuiApp>(L"D3D12 ImGui", kWidth, kHeight);
#elif _WIN32 && _D3D12
		app = std::make_unique<D3D12App>(L"D3D12 App", kWidth, kHeight);
#elif _WIN32 && _D3D11 && _TEXTURE
		app = std::make_unique<D3D11TextureApp>(L"D3D11 Texture", kWidth, kHeight);
#elif _WIN32 && _D3D11 && _TRIANGLE
		app = std::make_unique<D3D11TriangleApp>(L"D3D11 Triangle", kWidth, kHeight);
#elif _WIN32 && _D3D11 && _IMGUI
		app = std::make_unique<D3D11ImGuiApp>(L"D3D11 ImGui", kWidth, kHeight);
#elif _WIN32 && _D3D11
		app = std::make_unique<D3D11App>(L"D3D11 App", kWidth, kHeight);
#elif _WIN32
		app = std::make_unique<Win32App>(L"Win32 App", kWidth, kHeight);
#else
		app = std::make_unique<CoreApp>();
#endif
		ret = app->Run();
	}
	return ret;
}
