#include "D3D12App.h"

#define ENABLE_GPU_BASED_VALIDATION 0

D3D12App::D3D12App(LPCWSTR title, UINT width, UINT height)
    : Win32App(title, width, height)
    , m_device()
    , m_gfx_cmd_queue()
    , m_gfx_cmd_allocators()
    , m_gfx_cmd_list()
    , m_swap_chain4()
    , m_back_buffer_index(0)
    , m_rtv_heap()
    , m_rtv_heap_size(0)
    , m_back_buffers()
    , m_dsv_heap()
    , m_depth_buffer()
    , m_fence()
    , m_fence_value(0)
{
}

bool D3D12App::OnInitialize()
{
	HRESULT hr = S_OK;

	UINT dxgi_flags = 0;
#if defined(_DEBUG)
	{
		ComPtr<ID3D12Debug> d3d12_debug;
		hr = ::D3D12GetDebugInterface(IID_PPV_ARGS(d3d12_debug.GetAddressOf()));
		RETURN_FALSE_IF_FAILED(hr);

		d3d12_debug->EnableDebugLayer();
		dxgi_flags |= DXGI_CREATE_FACTORY_DEBUG;

#if ENABLE_GPU_BASED_VALIDATION
		ComPtr<ID3D12Debug3> d3d12_debug3;
		hr = d3d12_debug.As(&d3d12_debug3);
		RETURN_FALSE_IF_FAILED(hr);

		d3d12_debug3->SetEnableGPUBasedValidation(true);
#endif
	}
#endif

	ComPtr<IDXGIFactory6> dxgi_factory6;
	{
		ComPtr<IDXGIFactory2> dxgi_factory2;
		hr = ::CreateDXGIFactory2(dxgi_flags, IID_PPV_ARGS(dxgi_factory2.GetAddressOf()));
		RETURN_FALSE_IF_FAILED(hr);

		hr = dxgi_factory2.As(&dxgi_factory6);
		RETURN_FALSE_IF_FAILED(hr);
	}

	ComPtr<IDXGIAdapter1> dxgi_adapter1;
	{
		for (UINT i = 0; SUCCEEDED(dxgi_factory6->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(dxgi_adapter1.GetAddressOf()))); ++i)
		{
			DXGI_ADAPTER_DESC1 desc;
			dxgi_adapter1->GetDesc1(&desc);
			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
				continue;

			if (SUCCEEDED(::D3D12CreateDevice(dxgi_adapter1.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)))
				break;
		}

		if (!dxgi_adapter1)
		{
			for (UINT i = 0; SUCCEEDED(dxgi_factory6->EnumAdapters1(i, &dxgi_adapter1)); ++i)
			{
				DXGI_ADAPTER_DESC1 desc;
				dxgi_adapter1->GetDesc1(&desc);
				if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
					continue;

				if (SUCCEEDED(::D3D12CreateDevice(dxgi_adapter1.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)))
					break;
			}
		}
	}

	{
		hr = ::D3D12CreateDevice(dxgi_adapter1.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(m_device.GetAddressOf()));
		RETURN_FALSE_IF_FAILED(hr);
	}

	{
		D3D12_COMMAND_QUEUE_DESC d3d12_command_queue_desc = {
			.Type     = D3D12_COMMAND_LIST_TYPE_DIRECT,
			.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
			.Flags    = D3D12_COMMAND_QUEUE_FLAG_NONE,
			.NodeMask = 0
		};
		hr = m_device->CreateCommandQueue(&d3d12_command_queue_desc, IID_PPV_ARGS(m_gfx_cmd_queue.GetAddressOf()));
		RETURN_FALSE_IF_FAILED(hr);
	}

	{
		for (auto& allocator : m_gfx_cmd_allocators)
		{
			hr = m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(allocator.GetAddressOf()));
			RETURN_FALSE_IF_FAILED(hr);
		}
	}

	{
		hr = m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_gfx_cmd_allocators.at(0).Get(), nullptr, IID_PPV_ARGS(m_gfx_cmd_list.GetAddressOf()));
		RETURN_FALSE_IF_FAILED(hr);

		hr = m_gfx_cmd_list->Close();
		RETURN_FALSE_IF_FAILED(hr);
	}

	{
		DXGI_SWAP_CHAIN_DESC1 swap_chain_desc1 = {
			.Width       = m_width,
			.Height      = m_height,
			.Format      = DXGI_FORMAT_R8G8B8A8_UNORM,
			.Stereo      = true,
			.SampleDesc  = {.Count = 1, .Quality = 0},
			.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
			.BufferCount = kBackBufferCount,
			.Scaling     = DXGI_SCALING_NONE,
			.SwapEffect  = DXGI_SWAP_EFFECT_FLIP_DISCARD,
			.AlphaMode   = DXGI_ALPHA_MODE_UNSPECIFIED,
			.Flags       = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
		};
		ComPtr<IDXGISwapChain1> dxgi_swap_chain1;
		hr = dxgi_factory6->CreateSwapChainForHwnd(m_gfx_cmd_queue.Get(), m_hwnd, &swap_chain_desc1, nullptr, nullptr, dxgi_swap_chain1.GetAddressOf());
		RETURN_FALSE_IF_FAILED(hr);

		hr = dxgi_factory6->MakeWindowAssociation(m_hwnd, DXGI_MWA_NO_ALT_ENTER);
		RETURN_FALSE_IF_FAILED(hr);

		hr = dxgi_swap_chain1.As(&m_swap_chain4);
		RETURN_FALSE_IF_FAILED(hr);
	}

	{
		D3D12_DESCRIPTOR_HEAP_DESC rtv_heap_desc = {
			.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
			.NumDescriptors = kBackBufferCount,
			.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
			.NodeMask       = 0
		};
		hr = m_device->CreateDescriptorHeap(&rtv_heap_desc, IID_PPV_ARGS(m_rtv_heap.GetAddressOf()));
		RETURN_FALSE_IF_FAILED(hr);
		m_rtv_heap_size = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		auto handle = m_rtv_heap->GetCPUDescriptorHandleForHeapStart();
		for (UINT i = 0; i < kBackBufferCount; ++i)
		{
			auto& back_buffer = m_back_buffers.at(i);
			hr                = m_swap_chain4->GetBuffer(i, IID_PPV_ARGS(back_buffer.GetAddressOf()));
			RETURN_FALSE_IF_FAILED(hr);

			m_device->CreateRenderTargetView(back_buffer.Get(), nullptr, handle);
			handle.ptr += m_rtv_heap_size;
		}
	}

	{
		D3D12_DESCRIPTOR_HEAP_DESC dsv_heap_desc = {
			.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
			.NumDescriptors = 1,
			.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
			.NodeMask       = 0
		};
		hr = m_device->CreateDescriptorHeap(&dsv_heap_desc, IID_PPV_ARGS(m_dsv_heap.GetAddressOf()));
		RETURN_FALSE_IF_FAILED(hr);

		D3D12_HEAP_PROPERTIES d3d12_heap_properties = {
			.Type                 = D3D12_HEAP_TYPE_DEFAULT,
			.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
			.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
			.CreationNodeMask     = 0,
			.VisibleNodeMask      = 0
		};

		D3D12_RESOURCE_DESC d3d12_resource_desc = {
			.Dimension        = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
			.Alignment        = 0,
			.Width            = m_width,
			.Height           = m_height,
			.DepthOrArraySize = 1,
			.MipLevels        = 0,
			.Format           = DXGI_FORMAT_D32_FLOAT,
			.SampleDesc       = {.Count = 1, .Quality = 0},
			.Layout           = D3D12_TEXTURE_LAYOUT_UNKNOWN,
			.Flags            = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
		};

		D3D12_CLEAR_VALUE d3d12_clear_value = {
			.Format       = d3d12_resource_desc.Format,
			.DepthStencil = {.Depth = 1.0f, .Stencil = 0}
		};

		hr = m_device->CreateCommittedResource(
		    &d3d12_heap_properties,
		    D3D12_HEAP_FLAG_NONE,
		    &d3d12_resource_desc,
		    D3D12_RESOURCE_STATE_DEPTH_WRITE,
		    &d3d12_clear_value,
		    IID_PPV_ARGS(m_depth_buffer.GetAddressOf()));
		RETURN_FALSE_IF_FAILED(hr);

		D3D12_DEPTH_STENCIL_VIEW_DESC dsv_desc = {
			.Format        = d3d12_resource_desc.Format,
			.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D,
			.Flags         = D3D12_DSV_FLAG_NONE,
			.Texture2D     = { .MipSlice = 0 }
		};
		auto handle = m_dsv_heap->GetCPUDescriptorHandleForHeapStart();
		m_device->CreateDepthStencilView(m_depth_buffer.Get(), &dsv_desc, handle);
	}

	{
		hr = m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_fence.GetAddressOf()));
		RETURN_FALSE_IF_FAILED(hr);

		m_fence_value = 0;
	}

	return true;
}

void D3D12App::OnFinalize()
{
	m_gfx_cmd_queue->Signal(m_fence.Get(), ++m_fence_value);
	if (m_fence->GetCompletedValue() < m_fence_value)
	{
		auto event = ::CreateEvent(nullptr, false, false, nullptr);
		m_fence->SetEventOnCompletion(m_fence_value, event);
		::WaitForSingleObject(event, INFINITE);
		::CloseHandle(event);
	}
}

void D3D12App::OnUpdate()
{
}

void D3D12App::OnRender()
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
