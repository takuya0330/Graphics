#include "D3D11ImGui.h"

// clang-format off
#include "External/ImGui/imgui.h"
#include "External/ImGui/backends/imgui_impl_win32.h"
#include "External/ImGui/backends/imgui_impl_dx11.h"
// clang-format on

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

D3D11ImGui::D3D11ImGui(LPCWSTR title, UINT width, UINT height)
    : D3D11App(title, width, height)
#if ENABLE_IMGUI_DEMO_WINDOW
    , m_enable_demo_window(true)
#endif
{
}

bool D3D11ImGui::OnInitialize()
{
	ASSERT_RETURN(D3D11App::OnInitialize(), false);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ASSERT_RETURN(ImGui_ImplWin32_Init(m_hwnd), false);
    ASSERT_RETURN(ImGui_ImplDX11_Init(m_d3d11_device.Get(), m_d3d11_immediate_context.Get()), false);

    return true;
}

void D3D11ImGui::OnFinalize()
{
	D3D11App::OnFinalize();

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void D3D11ImGui::OnUpdate()
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

void D3D11ImGui::OnRender()
{
	{
		D3D11_VIEWPORT viewport = {
			.TopLeftX = 0,
			.TopLeftY = 0,
			.Width    = static_cast<float>(m_width),
			.Height   = static_cast<float>(m_height),
			.MinDepth = 0.0f,
			.MaxDepth = 1.0f
		};
		m_d3d11_immediate_context->RSSetViewports(1, &viewport);
	}

	{
		constexpr float kClearColor[] = { 0, 0, 0, 1 };
		m_d3d11_immediate_context->ClearRenderTargetView(m_d3d11_render_target_view.Get(), kClearColor);
		m_d3d11_immediate_context->ClearDepthStencilView(m_d3d11_depth_stencil_view.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		m_d3d11_immediate_context->OMSetRenderTargets(1, m_d3d11_render_target_view.GetAddressOf(), m_d3d11_depth_stencil_view.Get());
	}

    {
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    }

	m_dxgi_swap_chain->Present(1, 0);
}

LRESULT CALLBACK D3D11ImGui::OnWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
		return true;

	return Win32App::OnWindowProc(hWnd, uMsg, wParam, lParam);
}
