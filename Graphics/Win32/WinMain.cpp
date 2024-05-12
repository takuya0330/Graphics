// https://learn.microsoft.com/ja-jp/windows/win32/learnwin32/winmain--the-application-entry-point

#define APP_WIN32 1
#define APP_D3D11 1
#define APP_D3D12 1

#include <crtdbg.h>

#if 0
#elif APP_D3D12
#include "D3D12App.h"
#elif APP_D3D11
#include "D3D11App.h"
#elif APP_WIN32
#include "Win32App.h"
#endif

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PWSTR pCmdLine, _In_ int nCmdShow)
{
#if defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	constexpr UINT kWidth  = 1280;
	constexpr UINT kHeight = 720;

#if 0
#elif APP_D3D12
	D3D12App app(L"D3D12 App", kWidth, kHeight);
#elif APP_D3D11
	D3D11App app(L"D3D11 App", kWidth, kHeight);
#elif APP_WIN32
	Win32App app(L"Win32 App", kWidth, kHeight);
#endif
	return app.Run();
}
