#include "D3D12GPUTimerApp.h"

#include "Win32/Debug.h"

// clang-format off
#include "External/ImGui/imgui.h"
#include "External/ImGui/backends/imgui_impl_win32.h"
#include "External/ImGui/backends/imgui_impl_dx12.h"
// clang-format on

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

D3D12GPUTimerApp::D3D12GPUTimerApp(LPCWSTR title, UINT width, UINT height)
    : D3D12App(title, width, height)
    , m_query_heap()
    , m_readback_buffer()
    , m_gpu_time(0.0f)
    , m_imgui_heap()
{
}

bool D3D12GPUTimerApp::OnInitialize()
{
	ASSERT_RETURN(D3D12App::OnInitialize(), false);

	const uint32_t num_querys = 2 * m_num_back_buffers;

	D3D12_QUERY_HEAP_DESC query_heap_desc = {
		.Type     = D3D12_QUERY_HEAP_TYPE_TIMESTAMP,
		.Count    = num_querys,
		.NodeMask = 0
	};
	auto hr = m_device->CreateQueryHeap(
	    &query_heap_desc, IID_PPV_ARGS(m_query_heap.ReleaseAndGetAddressOf()));
	ASSERT_IF_FAILED(hr);

	if (!createBuffer(D3D12_HEAP_TYPE_READBACK, sizeof(uint64_t) * num_querys, D3D12_RESOURCE_STATE_COPY_DEST, m_readback_buffer.ReleaseAndGetAddressOf()))
		return false;

	{
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
		ASSERT_RETURN(ImGui_ImplDX12_Init(m_device.Get(), m_num_back_buffers, DXGI_FORMAT_R8G8B8A8_UNORM, m_imgui_heap.Get(), imgui_cpu_handle, imgui_gpu_handle), false);
	}

	return true;
}

void D3D12GPUTimerApp::OnFinalize()
{
	D3D12App::OnFinalize();

	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void D3D12GPUTimerApp::OnUpdate()
{
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	if (ImGui::Begin("GPU Time"))
	{
		ImGui::Text("Frame Time: %f", m_gpu_time);
	}
	ImGui::End();
}

void D3D12GPUTimerApp::OnRender()
{
	reset();

	m_gpu_time = getTime();

	beginCapture();

	setViewport(static_cast<float>(m_width), static_cast<float>(m_height));
	setScissorRect(static_cast<LONG>(m_width), static_cast<LONG>(m_height));
	setBackBuffer();

	{
		ImGui::Render();

		m_gfx_cmd_list->SetDescriptorHeaps(1, m_imgui_heap.GetAddressOf());
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_gfx_cmd_list.Get());
	}

	endCapture();

	executeCommandList();
	present(1);
	waitPreviousFrame();
}

LRESULT CALLBACK D3D12GPUTimerApp::OnWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
		return true;

	return D3D12App::OnWindowProc(hWnd, uMsg, wParam, lParam);
}

void D3D12GPUTimerApp::beginCapture()
{
	m_gfx_cmd_list->EndQuery(m_query_heap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, getQueryIndex(true));
}

void D3D12GPUTimerApp::endCapture()
{
	m_gfx_cmd_list->EndQuery(m_query_heap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, getQueryIndex(false));
}

float D3D12GPUTimerApp::getTime()
{
	static int block_count = 0;
	if (block_count++ < m_num_back_buffers)
		return 0;

	uint32_t query_index  = getQueryIndex(true);
	uint32_t query_offset = query_index * sizeof(uint64_t);

	m_gfx_cmd_list->ResolveQueryData(m_query_heap.Get(),
	    D3D12_QUERY_TYPE_TIMESTAMP,
	    query_index,
	    2,
	    m_readback_buffer.Get(),
	    query_index * sizeof(uint64_t));

	uint64_t*   ptr   = nullptr;
	D3D12_RANGE range = {
		.Begin = query_offset,
		.End   = query_offset + sizeof(uint64_t) * 2
	};
	auto hr = m_readback_buffer->Map(0, &range, reinterpret_cast<void**>(&ptr));
	ASSERT_RETURN(SUCCEEDED(hr), 0.0f);

	uint64_t beg = ptr[query_index + 0];
	uint64_t end = ptr[query_index + 1];

	m_readback_buffer->Unmap(0, nullptr);

	uint64_t frequency = 0;
	hr                 = m_gfx_cmd_queue->GetTimestampFrequency(&frequency);
	ASSERT_IF_FAILED(hr);

	return (end > beg) ? (end - beg) * 1000.0 / static_cast<double>(frequency) : 0.0f;
}

uint32_t D3D12GPUTimerApp::getQueryIndex(bool is_begin) const
{
	return (m_back_buffer_index * 2) + (is_begin ? 0 : 1);
}
