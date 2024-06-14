// https://learn.microsoft.com/ja-jp/windows/win32/learnwin32/winmain--the-application-entry-point

#define APP_WIN32 1
#define APP_D3D11 1
#define APP_D3D12 1
#define APP_IMGUI 1

#if 0
#elif APP_WIN32 && APP_D3D12 && APP_IMGUI
#include "D3D12/D3D12ImGuiApp.h"
#elif APP_WIN32 && APP_D3D12
#include "D3D12/D3D12App.h"
#elif APP_WIN32 && APP_D3D11 && APP_IMGUI
#include "D3D11/D3D11ImGuiApp.h"
#elif APP_WIN32 && APP_D3D11
#include "D3D11/D3D11App.h"
#elif APP_WIN32
#include "Win32App.h"
#else
#include "CoreApp.h"
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
#elif APP_WIN32 && APP_D3D12 && APP_IMGUI
		app = std::make_unique<D3D12ImGuiApp>(L"D3D12 ImGui", kWidth, kHeight);
#elif APP_WIN32 && APP_D3D12
		app = std::make_unique<D3D12App>(L"D3D12 App", kWidth, kHeight);
#elif APP_WIN32 && APP_D3D11 && APP_IMGUI
		app = std::make_unique<D3D11ImGuiApp>(L"D3D11 ImGui", kWidth, kHeight);
#elif APP_WIN32 && APP_D3D11
		app = std::make_unique<D3D11App>(L"D3D11 App", kWidth, kHeight);
#elif APP_WIN32
		app = std::make_unique<Win32App>(L"Win32 App", kWidth, kHeight);
#else
		app = std::make_unique<CoreApp>();
#endif
		ret = app->Run();
	}

	return ret;
}
