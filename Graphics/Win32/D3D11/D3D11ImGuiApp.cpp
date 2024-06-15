#include "D3D11ImGuiApp.h"

// clang-format off
#include "External/ImGui/imgui.h"
#include "External/ImGui/backends/imgui_impl_win32.h"
#include "External/ImGui/backends/imgui_impl_dx11.h"
// clang-format on

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

D3D11ImGuiApp::D3D11ImGuiApp(LPCWSTR title, UINT width, UINT height)
    : D3D11App(title, width, height)
#if ENABLE_IMGUI_DEMO_WINDOW
    , m_enable_demo_window(true)
#endif
{
}

bool D3D11ImGuiApp::OnInitialize()
{
	ASSERT_RETURN(D3D11App::OnInitialize(), false);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ASSERT_RETURN(ImGui_ImplWin32_Init(m_hwnd), false);
    ASSERT_RETURN(ImGui_ImplDX11_Init(m_d3d11_device.Get(), m_d3d11_immediate_context.Get()), false);

    return true;
}

void D3D11ImGuiApp::OnFinalize()
{
	D3D11App::OnFinalize();

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void D3D11ImGuiApp::OnUpdate()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

#if ENABLE_IMGUI_DEMO_WINDOW
	if (m_enable_demo_window)
	{
		ImGui::ShowDemoWindow(&m_enable_demo_window);
	}
#endif
}

void D3D11ImGuiApp::OnRender()
{
	setViewport(static_cast<float>(m_width), static_cast<float>(m_height));
	setBackBuffer();
	present(1);
}

LRESULT CALLBACK D3D11ImGuiApp::OnWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
		return true;

	return Win32App::OnWindowProc(hWnd, uMsg, wParam, lParam);
}
