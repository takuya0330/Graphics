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
    , m_dsv_heap_size(0)
    , m_depth_buffer()
    , m_frame_fences()
    , m_frame_fence_values()
{
}

bool D3D12App::OnInitialize()
{
	HRESULT hr = S_OK;

	UINT dxgi_flags = 0;
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
		DXGI_SWAP_CHAIN_DESC1 desc = {
			.Width       = m_width,
			.Height      = m_height,
			.Format      = DXGI_FORMAT_R8G8B8A8_UNORM,
			.Stereo      = true,
			.SampleDesc  = {.Count = 1, .Quality = 0},
			.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
			.BufferCount = 2,
			.Scaling     = DXGI_SCALING_NONE,
			.SwapEffect  = DXGI_SWAP_EFFECT_FLIP_DISCARD,
			.AlphaMode   = DXGI_ALPHA_MODE_UNSPECIFIED,
			.Flags       = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
		};
		ComPtr<IDXGISwapChain1> dxgi_swap_chain1;
		hr = dxgi_factory6->CreateSwapChainForHwnd(m_gfx_cmd_queue.Get(), m_hwnd, &desc, nullptr, nullptr, dxgi_swap_chain1.GetAddressOf());
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

		D3D12_DESCRIPTOR_HEAP_DESC dsv_heap_desc = {
			.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
			.NumDescriptors = 1,
			.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
			.NodeMask       = 0
		};
		hr = m_device->CreateDescriptorHeap(&rtv_heap_desc, IID_PPV_ARGS(m_dsv_heap.GetAddressOf()));
		RETURN_FALSE_IF_FAILED(hr);
		m_dsv_heap_size = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}

	{
		auto handle = m_rtv_heap->GetCPUDescriptorHandleForHeapStart();
		for (UINT i = 0; i < kBackBufferCount; ++i)
		{
			hr = m_swap_chain4->GetBuffer(i, IID_PPV_ARGS(m_back_buffers.at(i).GetAddressOf()));
			RETURN_FALSE_IF_FAILED(hr);

			m_device->CreateRenderTargetView(m_back_buffers.at(i).Get(), nullptr, handle);
			handle.ptr += static_cast<SIZE_T>(i * m_rtv_heap_size);
		}
	}

	{
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
			.MipLevels        = 1,
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
		    nullptr,
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
		for (UINT i = 0; i < kBackBufferCount; ++i)
		{
			hr = m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_frame_fences.at(i).GetAddressOf()));
			RETURN_FALSE_IF_FAILED(hr);

			m_frame_fence_values.at(i) = 0;
		}
	}

	return true;
}

void D3D12App::OnFinalize()
{
}

void D3D12App::OnUpdate()
{
}

void D3D12App::OnRender()
{
}
