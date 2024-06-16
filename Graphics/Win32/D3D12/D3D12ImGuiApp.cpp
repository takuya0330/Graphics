#include "D3D12ImGuiApp.h"

#if APP_WIN32 && APP_D3D12 && APP_IMGUI

// clang-format off
#include "External/ImGui/imgui.h"
#include "External/ImGui/backends/imgui_impl_win32.h"
#include "External/ImGui/backends/imgui_impl_dx12.h"
// clang-format on

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

D3D12ImGuiApp::D3D12ImGuiApp(LPCWSTR title, UINT width, UINT height)
    : D3D12App(title, width, height)
    , m_imgui_heap()
#if ENABLE_IMGUI_DEMO_WINDOW
    , m_enable_demo_window(true)
#endif
{
}

bool D3D12ImGuiApp::OnInitialize()
{
	ASSERT_RETURN(D3D12App::OnInitialize(), false);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ASSERT_RETURN(ImGui_ImplWin32_Init(m_hwnd), false);

	D3D12_DESCRIPTOR_HEAP_DESC imgui_heap_desc = {
		.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		.NumDescriptors = 1,
		.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
		.NodeMask       = 0
	};
	auto hr = m_device->CreateDescriptorHeap(&imgui_heap_desc, IID_PPV_ARGS(m_imgui_heap.GetAddressOf()));
	RETURN_FALSE_IF_FAILED(hr);

	auto imgui_cpu_handle = m_imgui_heap->GetCPUDescriptorHandleForHeapStart();
	auto imgui_gpu_handle = m_imgui_heap->GetGPUDescriptorHandleForHeapStart();
	ASSERT_RETURN(ImGui_ImplDX12_Init(m_device.Get(), kBackBufferCount, DXGI_FORMAT_R8G8B8A8_UNORM, m_imgui_heap.Get(), imgui_cpu_handle, imgui_gpu_handle), false);

	return true;
}

void D3D12ImGuiApp::OnFinalize()
{
	D3D12App::OnFinalize();

	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void D3D12ImGuiApp::OnUpdate()
{
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

#if ENABLE_IMGUI_DEMO_WINDOW
	if (m_enable_demo_window)
	{
		ImGui::ShowDemoWindow(&m_enable_demo_window);
	}
#endif
}

void D3D12ImGuiApp::OnRender()
{
	reset();
	setViewport(static_cast<float>(m_width), static_cast<float>(m_height));
	setScissorRect(static_cast<LONG>(m_width), static_cast<LONG>(m_height));
	setBackBuffer();
	renderImGui();
	executeCommandList();
	present(1);
	waitPreviousFrame();
}

LRESULT CALLBACK D3D12ImGuiApp::OnWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
		return true;

	return Win32App::OnWindowProc(hWnd, uMsg, wParam, lParam);
}

void D3D12ImGuiApp::renderImGui()
{
	ImGui::Render();

	m_gfx_cmd_list->SetDescriptorHeaps(1, m_imgui_heap.GetAddressOf());
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_gfx_cmd_list.Get());
}

#endif
