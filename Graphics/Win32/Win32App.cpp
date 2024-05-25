#include "Win32App.h"

namespace {

constexpr const wchar_t kWindowClassName[] = L"Win32 API";

} // namespace

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

Win32App::Win32App(LPCWSTR title, UINT width, UINT height)
    : m_hwnd(nullptr)
    , m_title(title)
    , m_width(width)
    , m_height(height)
{
}

int Win32App::Run()
{
	WNDCLASSEX wcex = {
		.cbSize        = sizeof(WNDCLASSEX),
		.style         = CS_HREDRAW | CS_VREDRAW,
		.lpfnWndProc   = WindowProc,
		.cbClsExtra    = 0,
		.cbWndExtra    = 0,
		.hInstance     = ::GetModuleHandle(nullptr),
		.hIcon         = 0,
		.hCursor       = LoadCursor(NULL, IDC_ARROW),
		.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1),
		.lpszMenuName  = nullptr,
		.lpszClassName = kWindowClassName,
		.hIconSm       = 0
	};
	ASSERT_RETURN(::RegisterClassEx(&wcex), 1);

	RECT  rect  = { 0, 0, static_cast<LONG>(m_width), static_cast<LONG>(m_height) };
	DWORD style = WS_OVERLAPPEDWINDOW;
	ASSERT_RETURN(::AdjustWindowRect(&rect, style, false), 1);

	m_hwnd = ::CreateWindowEx(
	    0,
	    kWindowClassName,
	    m_title,
	    style,
	    CW_USEDEFAULT,
	    CW_USEDEFAULT,
	    static_cast<int>(rect.right - rect.left),
	    static_cast<int>(rect.bottom - rect.top),
	    nullptr,
	    nullptr,
	    wcex.hInstance,
	    nullptr);
	ASSERT_RETURN(m_hwnd, 1);

	::SetWindowLongPtr(m_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

	ASSERT_RETURN(OnInitialize(), 1);

	::UpdateWindow(m_hwnd);
	::ShowWindow(m_hwnd, SW_NORMAL);
	::ShowCursor(true);

	MSG msg = {};
	while (msg.message != WM_QUIT)
	{
		while (::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}

		OnUpdate();
		OnRender();
	}

	OnFinalize();

	::UnregisterClass(m_title, wcex.hInstance);

	return static_cast<int>(msg.wParam);
}

bool Win32App::OnInitialize()
{
	return true;
}

void Win32App::OnFinalize()
{
}

void Win32App::OnUpdate()
{
}

void Win32App::OnRender()
{
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	auto app = reinterpret_cast<Win32App*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

	switch (uMsg)
	{
	case WM_DESTROY:
		::PostQuitMessage(0);
		break;
	default:
		return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	return 0;
}
