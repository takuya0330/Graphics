#include "D3D12ImGui.h"

// clang-format off
#include "External/ImGui/imgui.h"
#include "External/ImGui/backends/imgui_impl_win32.h"
#include "External/ImGui/backends/imgui_impl_dx12.h"
// clang-format on

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

D3D12ImGui::D3D12ImGui(LPCWSTR title, UINT width, UINT height)
    : D3D12App(title, width, height)
    , m_imgui_heap()
#if ENABLE_IMGUI_DEMO_WINDOW
    , m_enable_demo_window(true)
#endif
{
}

bool D3D12ImGui::OnInitialize()
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

void D3D12ImGui::OnFinalize()
{
	D3D12App::OnFinalize();

	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void D3D12ImGui::OnUpdate()
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

void D3D12ImGui::OnRender()
{
	HRESULT hr = S_OK;

	m_back_buffer_index = m_swap_chain4->GetCurrentBackBufferIndex();
	{
		hr = m_gfx_cmd_allocators.at(m_back_buffer_index)->Reset();
		ASSERT_IF_FAILED(hr);

		hr = m_gfx_cmd_list->Reset(m_gfx_cmd_allocators.at(m_back_buffer_index).Get(), nullptr);
		ASSERT_IF_FAILED(hr);
	}

	{
		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource   = m_back_buffers.at(m_back_buffer_index).Get();
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_RENDER_TARGET;
		m_gfx_cmd_list->ResourceBarrier(1, &barrier);
	}

	{
		D3D12_VIEWPORT viewport = {
			.TopLeftX = 0,
			.TopLeftY = 0,
			.Width    = static_cast<float>(m_width),
			.Height   = static_cast<float>(m_height),
			.MinDepth = 0.0f,
			.MaxDepth = 1.0f
		};
		m_gfx_cmd_list->RSSetViewports(1, &viewport);

		D3D12_RECT scissor = {
			.left   = 0,
			.top    = 0,
			.right  = static_cast<LONG>(m_width),
			.bottom = static_cast<LONG>(m_height)
		};
		m_gfx_cmd_list->RSSetScissorRects(1, &scissor);
	}

	{
		auto rtv = m_rtv_heap->GetCPUDescriptorHandleForHeapStart();
		rtv.ptr += static_cast<SIZE_T>(m_rtv_heap_size * m_back_buffer_index);

		constexpr float kClearColor[] = { 0, 0, 0, 1 };
		m_gfx_cmd_list->ClearRenderTargetView(rtv, kClearColor, 0, nullptr);

		auto dsv = m_dsv_heap->GetCPUDescriptorHandleForHeapStart();
		m_gfx_cmd_list->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

		m_gfx_cmd_list->OMSetRenderTargets(1, &rtv, false, nullptr);
	}

    {
		ImGui::Render();

        m_gfx_cmd_list->SetDescriptorHeaps(1, m_imgui_heap.GetAddressOf());
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_gfx_cmd_list.Get());
    }

	{
		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource   = m_back_buffers.at(m_back_buffer_index).Get();
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_PRESENT;
		m_gfx_cmd_list->ResourceBarrier(1, &barrier);
	}

	{
		hr = m_gfx_cmd_list->Close();
		ASSERT_IF_FAILED(hr);

		ID3D12CommandList* cmd_lists[] = { m_gfx_cmd_list.Get() };
		m_gfx_cmd_queue->ExecuteCommandLists(_countof(cmd_lists), cmd_lists);

		m_gfx_cmd_queue->Signal(m_fence.Get(), ++m_fence_value);
		if (m_fence->GetCompletedValue() < m_fence_value)
		{
			auto event = ::CreateEvent(nullptr, false, false, nullptr);
			m_fence->SetEventOnCompletion(m_fence_value, event);
			::WaitForSingleObject(event, INFINITE);
			::CloseHandle(event);
		}

		m_swap_chain4->Present(1, 0);
	}
}

LRESULT CALLBACK D3D12ImGui::OnWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
		return true;

	return Win32App::OnWindowProc(hWnd, uMsg, wParam, lParam);
}
