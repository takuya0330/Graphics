#include "D3D12App.h"

#include <filesystem>
#include <fstream>
#include <vector>

#define ENABLE_GPU_BASED_VALIDATION 0

D3D12App::D3D12App(LPCWSTR title, UINT width, UINT height)
    : Win32App(title, width, height)
    , m_num_back_buffers(2)
    , m_factory()
    , m_adapters()
    , m_adapter_index(-1)
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
    , m_fences()
    , m_fence_values()
{
}

bool D3D12App::OnInitialize()
{
	UINT dxgi_flags = 0;
	ASSERT_RETURN(enableDebugLayer(dxgi_flags), false);
	ASSERT_RETURN(createFactory(dxgi_flags), false);
	ASSERT_RETURN(searchAdapter(), false);
	ASSERT_RETURN(createDevice(), false);
	ASSERT_RETURN(createCommand(), false);
	ASSERT_RETURN(createSwapChain(), false);
	ASSERT_RETURN(createBackBuffer(), false);
	ASSERT_RETURN(createFence(), false);

	return true;
}

void D3D12App::OnFinalize()
{
	waitForGPU(m_gfx_cmd_queue.Get());
}

void D3D12App::OnUpdate()
{
}

void D3D12App::OnRender()
{
	reset();
	setViewport(static_cast<float>(m_width), static_cast<float>(m_height));
	setScissorRect(static_cast<LONG>(m_width), static_cast<LONG>(m_height));
	setBackBuffer();
	executeCommandList();
	present(1);
	waitPreviousFrame();
}

bool D3D12App::enableDebugLayer(UINT& dxgi_flags)
{
	HRESULT hr = S_OK;
#if defined(_DEBUG)
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
#endif

	return true;
}

bool D3D12App::createFactory(UINT dxgi_flags)
{
	HRESULT hr = S_OK;

	ComPtr<IDXGIFactory2> dxgi_factory2;
	hr = ::CreateDXGIFactory2(dxgi_flags, IID_PPV_ARGS(dxgi_factory2.GetAddressOf()));
	RETURN_FALSE_IF_FAILED(hr);

	hr = dxgi_factory2.As(&m_factory);
	RETURN_FALSE_IF_FAILED(hr);

	return true;
}

bool D3D12App::searchAdapter()
{
	HRESULT hr = S_OK;

	for (UINT i = 0;; ++i)
	{
		ComPtr<IDXGIAdapter1> dxgi_adapter1;
		hr = m_factory->EnumAdapters1(i, &dxgi_adapter1);
		if (hr == DXGI_ERROR_NOT_FOUND)
			break;

		DXGI_ADAPTER_DESC1 desc;
		dxgi_adapter1->GetDesc1(&desc);
		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			continue;

		m_adapters.emplace_back(dxgi_adapter1);
	}

	if (m_adapters.empty())
		return false;

	UINT index = 0;
	for (auto& it : m_adapters)
	{
		DXGI_ADAPTER_DESC1 desc;
		it->GetDesc1(&desc);

		Debug::Log(L"DXGI Adapter Info:\n");
		Debug::Log(L"- Description          : %s\n", desc.Description);
		Debug::Log(L"- VendorId             : %u\n", desc.VendorId);
		Debug::Log(L"- DeviceId             : %u\n", desc.DeviceId);
		Debug::Log(L"- SubSysId             : %u\n", desc.SubSysId);
		Debug::Log(L"- Revision             : %u\n", desc.Revision);
		Debug::Log(L"- DedicatedVideoMemory : %llu\n", desc.DedicatedVideoMemory);
		Debug::Log(L"- DedicatedSystemMemory: %llu\n", desc.DedicatedSystemMemory);
		Debug::Log(L"- SharedSystemMemory   : %llu\n", desc.SharedSystemMemory);

		if (m_adapter_index == -1)
		{
			hr = ::D3D12CreateDevice(it.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr);
			if (SUCCEEDED(hr))
			{
				m_adapter_index = index;
			}
		}

		++index;
	}

	RETURN_FALSE_IF_FAILED(hr);

	return true;
}

bool D3D12App::createDevice()
{
	auto hr = Utilities::CreateDevice(
        m_adapters.at(m_adapter_index).Get(),
        D3D_FEATURE_LEVEL_11_0,
        m_device.GetAddressOf());
	RETURN_FALSE_IF_FAILED(hr);

	_D3D12_SET_NAME(m_device, L"Device");

	return true;
}

bool D3D12App::createCommand()
{
	m_gfx_cmd_allocators.resize(m_num_back_buffers);

	if (!createCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT, m_gfx_cmd_queue.GetAddressOf()))
		return false;
	_D3D12_SET_NAME(m_gfx_cmd_queue, L"GraphicsCommandQueue");

	for (auto& allocator : m_gfx_cmd_allocators)
	{
		if (!createCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, allocator.GetAddressOf()))
			return false;

		static int index = 0;
		_D3D12_SET_NAME(allocator, L"GraphicsCommandAllocator%d", index++);
	}

	if (!createCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT, m_gfx_cmd_allocators.at(0).Get(), m_gfx_cmd_list.GetAddressOf()))
		return false;
	_D3D12_SET_NAME(m_gfx_cmd_list, L"GraphicsCommandList");

	return true;
}

bool D3D12App::createSwapChain()
{
	HRESULT hr = S_OK;

	m_back_buffers.resize(m_num_back_buffers);

	DXGI_SWAP_CHAIN_DESC1 swap_chain_desc1 = {
		.Width       = m_width,
		.Height      = m_height,
		.Format      = DXGI_FORMAT_R8G8B8A8_UNORM,
		.Stereo      = false,
		.SampleDesc  = { .Count = 1, .Quality = 0 },
		.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
		.BufferCount = m_num_back_buffers,
		.Scaling     = DXGI_SCALING_NONE,
		.SwapEffect  = DXGI_SWAP_EFFECT_FLIP_DISCARD,
		.AlphaMode   = DXGI_ALPHA_MODE_UNSPECIFIED,
		.Flags       = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
	};
	ComPtr<IDXGISwapChain1> dxgi_swap_chain1;
	hr = m_factory->CreateSwapChainForHwnd(m_gfx_cmd_queue.Get(), m_hwnd, &swap_chain_desc1, nullptr, nullptr, dxgi_swap_chain1.GetAddressOf());
	RETURN_FALSE_IF_FAILED(hr);

	hr = m_factory->MakeWindowAssociation(m_hwnd, DXGI_MWA_NO_ALT_ENTER);
	RETURN_FALSE_IF_FAILED(hr);

	hr = dxgi_swap_chain1.As(&m_swap_chain4);
	RETURN_FALSE_IF_FAILED(hr);

	m_back_buffer_index = m_swap_chain4->GetCurrentBackBufferIndex();

	return true;
}

bool D3D12App::createBackBuffer()
{
	HRESULT hr = S_OK;

	{
		m_back_buffers.resize(m_num_back_buffers);

		if (!createDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, m_num_back_buffers, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, m_rtv_heap.GetAddressOf()))
			return false;
		_D3D12_SET_NAME(m_rtv_heap, L"RTVHeap");

		m_rtv_heap_size = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		auto handle = m_rtv_heap->GetCPUDescriptorHandleForHeapStart();
		for (UINT i = 0; i < m_num_back_buffers; ++i)
		{
			auto& back_buffer = m_back_buffers.at(i);
			hr                = m_swap_chain4->GetBuffer(i, IID_PPV_ARGS(back_buffer.GetAddressOf()));
			RETURN_FALSE_IF_FAILED(hr);

			_D3D12_SET_NAME(back_buffer, L"BackBuffer%u", i);

			m_device->CreateRenderTargetView(back_buffer.Get(), nullptr, handle);
			handle.ptr += m_rtv_heap_size;
		}
	}

	{
		if (!createDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, m_dsv_heap.GetAddressOf()))
			return false;
		_D3D12_SET_NAME(m_dsv_heap, L"DSVHeap");

		D3D12_CLEAR_VALUE d3d12_clear_value = {
			.Format       = DXGI_FORMAT_D32_FLOAT,
			.DepthStencil = { .Depth = 1.0f, .Stencil = 0 }
		};

		if (!createTexture2D(
		        D3D12_HEAP_TYPE_DEFAULT,
		        m_width,
		        m_height,
		        DXGI_FORMAT_D32_FLOAT,
		        D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL,
		        D3D12_RESOURCE_STATE_DEPTH_WRITE,
		        &d3d12_clear_value,
		        m_depth_buffer.GetAddressOf()))
			return false;
		_D3D12_SET_NAME(m_depth_buffer, L"DepthBuffer");

		D3D12_DEPTH_STENCIL_VIEW_DESC dsv_desc = {
			.Format        = DXGI_FORMAT_D32_FLOAT,
			.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D,
			.Flags         = D3D12_DSV_FLAG_NONE,
			.Texture2D     = { .MipSlice = 0 }
		};
		auto handle = m_dsv_heap->GetCPUDescriptorHandleForHeapStart();
		m_device->CreateDepthStencilView(m_depth_buffer.Get(), &dsv_desc, handle);
	}

	return true;
}

bool D3D12App::createFence()
{
	HRESULT hr = S_OK;

	m_fences.resize(m_num_back_buffers);
	m_fence_values.resize(m_num_back_buffers);

	for (auto& fence : m_fences)
	{
		hr = Utilities::CreateFence(m_device.Get(), fence.GetAddressOf());
		RETURN_FALSE_IF_FAILED(hr);

		static int index = 0;
		_D3D12_SET_NAME(fence, L"Fence%d", index++);
	}
	std::fill(m_fence_values.begin(), m_fence_values.end(), 0);

	return true;
}

void D3D12App::reset()
{
	auto hr = Utilities::Reset(
        m_gfx_cmd_allocators.at(m_back_buffer_index).Get(),
        m_gfx_cmd_list.Get());
	ASSERT_IF_FAILED(hr);

    Utilities::ResourceBarrierTransition(
	    m_gfx_cmd_list.Get(),
	    m_back_buffers.at(m_back_buffer_index).Get(),
	    D3D12_RESOURCE_STATE_PRESENT,
	    D3D12_RESOURCE_STATE_RENDER_TARGET);
}

void D3D12App::setViewport(float width, float height)
{
	Utilities::SetViewport(m_gfx_cmd_list.Get(), width, height);
}

void D3D12App::setScissorRect(LONG width, LONG height)
{
	Utilities::SetScissorRect(m_gfx_cmd_list.Get(), width, height);
}

void D3D12App::setBackBuffer()
{
	auto rtv = m_rtv_heap->GetCPUDescriptorHandleForHeapStart();
	rtv.ptr += static_cast<SIZE_T>(m_rtv_heap_size * m_back_buffer_index);

	constexpr float kClearColor[] = { 0, 0, 0, 1 };
	m_gfx_cmd_list->ClearRenderTargetView(rtv, kClearColor, 0, nullptr);

	auto dsv = m_dsv_heap->GetCPUDescriptorHandleForHeapStart();
	m_gfx_cmd_list->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	m_gfx_cmd_list->OMSetRenderTargets(1, &rtv, false, nullptr);
}

void D3D12App::executeCommandList()
{
	Utilities::ResourceBarrierTransition(
	    m_gfx_cmd_list.Get(),
	    m_back_buffers.at(m_back_buffer_index).Get(),
	    D3D12_RESOURCE_STATE_RENDER_TARGET,
	    D3D12_RESOURCE_STATE_PRESENT);

	Utilities::ExecuteCommandList(m_gfx_cmd_queue.Get(), m_gfx_cmd_list.Get());
}

void D3D12App::present(UINT sync_interval)
{
	m_swap_chain4->Present(sync_interval, 0);
}

void D3D12App::waitPreviousFrame()
{
	m_gfx_cmd_queue->Signal(m_fences.at(m_back_buffer_index).Get(), ++m_fence_values.at(m_back_buffer_index));

	m_back_buffer_index = m_swap_chain4->GetCurrentBackBufferIndex();
	if (m_fences.at(m_back_buffer_index)->GetCompletedValue() < m_fence_values.at(m_back_buffer_index))
	{
		auto event = ::CreateEvent(nullptr, false, false, nullptr);
		m_fences.at(m_back_buffer_index)->SetEventOnCompletion(m_fence_values.at(m_back_buffer_index), event);
#pragma warning(push)
#pragma warning(disable : 6387)
		::WaitForSingleObject(event, INFINITE);
		::CloseHandle(event);
#pragma warning(pop)
	}
}

void D3D12App::waitForGPU(ID3D12CommandQueue* cmd_queue)
{
	ComPtr<ID3D12Fence> fence;
	constexpr UINT64    expect_value = 1;

	auto hr = m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(fence.GetAddressOf()));
	ASSERT_IF_FAILED(hr);

	cmd_queue->Signal(fence.Get(), expect_value);
	if (fence->GetCompletedValue() != expect_value)
	{
		auto event = ::CreateEvent(nullptr, false, false, nullptr);
		fence->SetEventOnCompletion(expect_value, event);
#pragma warning(push)
#pragma warning(disable : 6387)
		::WaitForSingleObject(event, INFINITE);
		::CloseHandle(event);
#pragma warning(pop)
	}
}

bool D3D12App::createCommandQueue(
    const D3D12_COMMAND_LIST_TYPE type,
    ID3D12CommandQueue**          cmd_queue)
{
	D3D12_COMMAND_QUEUE_DESC command_queue_desc = {
		.Type     = type,
		.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
		.Flags    = D3D12_COMMAND_QUEUE_FLAG_NONE,
		.NodeMask = 0
	};
	auto hr = m_device->CreateCommandQueue(&command_queue_desc, IID_PPV_ARGS(cmd_queue));
	RETURN_FALSE_IF_FAILED(hr);

	return true;
}

bool D3D12App::createCommandAllocator(
    const D3D12_COMMAND_LIST_TYPE type,
    ID3D12CommandAllocator**      cmd_allocator)
{
	auto hr = m_device->CreateCommandAllocator(type, IID_PPV_ARGS(cmd_allocator));
	RETURN_FALSE_IF_FAILED(hr);

	return true;
}

bool D3D12App::createCommandList(
    const D3D12_COMMAND_LIST_TYPE type,
    ID3D12CommandAllocator*       cmd_allocator,
    ID3D12GraphicsCommandList**   cmd_list)
{
	auto hr = Utilities::CreateCommandList(
	    m_device.Get(),
	    D3D12_COMMAND_LIST_TYPE_DIRECT,
	    m_gfx_cmd_allocators.at(m_back_buffer_index).Get(),
	    m_gfx_cmd_list.GetAddressOf());
	RETURN_FALSE_IF_FAILED(hr);

	return true;
}

bool D3D12App::createDescriptorHeap(
    const D3D12_DESCRIPTOR_HEAP_TYPE  heap_type,
    const UINT                        num_descriptors,
    const D3D12_DESCRIPTOR_HEAP_FLAGS flags,
    ID3D12DescriptorHeap**            descriptor_heap)
{
	D3D12_DESCRIPTOR_HEAP_DESC dsv_heap_desc = {
		.Type           = heap_type,
		.NumDescriptors = num_descriptors,
		.Flags          = flags,
		.NodeMask       = 0
	};
	auto hr = m_device->CreateDescriptorHeap(&dsv_heap_desc, IID_PPV_ARGS(descriptor_heap));
	RETURN_FALSE_IF_FAILED(hr);

	return true;
}

bool D3D12App::createBuffer(
    const D3D12_HEAP_TYPE       heap_type,
    const UINT64                size,
    const D3D12_RESOURCE_FLAGS  flags,
    const D3D12_RESOURCE_STATES state,
    ID3D12Resource**            resource)
{
	auto hr = Utilities::CreateBuffer(
	    m_device.Get(),
	    heap_type,
	    size,
	    flags,
	    state,
	    resource);
	RETURN_FALSE_IF_FAILED(hr);

	return true;
}

bool D3D12App::createBuffer(
    const D3D12_HEAP_TYPE       heap_type,
    const UINT64                size,
    const D3D12_RESOURCE_STATES state,
    ID3D12Resource**            resource)
{
	return createBuffer(heap_type, size, D3D12_RESOURCE_FLAG_NONE, state, resource);
}

bool D3D12App::createTexture2D(
    const D3D12_HEAP_TYPE       heap_type,
    const UINT64                width,
    const UINT                  height,
    const UINT16                array_size,
    const UINT16                mip_levels,
    const DXGI_FORMAT           format,
    const D3D12_RESOURCE_FLAGS  flags,
    const D3D12_RESOURCE_STATES state,
    const D3D12_CLEAR_VALUE*    clear_value,
    ID3D12Resource**            resource)
{
	auto hr = Utilities::CreateTexture2D(
	    m_device.Get(),
	    heap_type,
	    width,
	    height,
	    array_size,
	    mip_levels,
	    format,
	    flags,
	    state,
	    clear_value,
	    resource);
	RETURN_FALSE_IF_FAILED(hr);

	return true;
}


bool D3D12App::createTexture2D(
    const D3D12_HEAP_TYPE       heap_type,
    const UINT64                width,
    const UINT                  height,
    const DXGI_FORMAT           format,
    const D3D12_RESOURCE_FLAGS  flags,
    const D3D12_RESOURCE_STATES state,
    const D3D12_CLEAR_VALUE*    clear_value,
    ID3D12Resource**            resource)
{
	return createTexture2D(
        heap_type,
        width,
        height,
        1,
        0,
        format,
        flags,
        state,
        clear_value,
        resource);
}

bool D3D12App::loadShader(
    const wchar_t* filename,
    const wchar_t* entry_point,
    const wchar_t* shader_model,
    IDxcBlob**     blob)
{
	std::wstring hlsl = m_hlsl_dir + filename;

	HRESULT hr = S_OK;

	ComPtr<IDxcUtils> utils;
	hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(utils.GetAddressOf()));
	RETURN_FALSE_IF_FAILED(hr);

	ComPtr<IDxcCompiler3> compiler;
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(compiler.GetAddressOf()));
	RETURN_FALSE_IF_FAILED(hr);

	ComPtr<IDxcIncludeHandler> handler;
	hr = utils->CreateDefaultIncludeHandler(handler.GetAddressOf());
	RETURN_FALSE_IF_FAILED(hr);

	ComPtr<IDxcBlobEncoding> encoding;
	hr = utils->LoadFile(hlsl.c_str(), nullptr, encoding.GetAddressOf());
	RETURN_FALSE_IF_FAILED(hr);

	std::vector<const wchar_t*> args;
	{
		args.emplace_back(hlsl.c_str());

		args.emplace_back(L"-E");
		args.emplace_back(entry_point);

		args.emplace_back(L"-T");
		args.emplace_back(shader_model);

		args.emplace_back(L"-Qstrip_debug");
		args.emplace_back(L"-Qstrip_reflect");

		args.emplace_back(DXC_ARG_WARNINGS_ARE_ERRORS);
		args.emplace_back(DXC_ARG_DEBUG);
	}

	DxcBuffer buffer = {
		.Ptr      = encoding->GetBufferPointer(),
		.Size     = encoding->GetBufferSize(),
		.Encoding = DXC_CP_ACP
	};

	ComPtr<IDxcResult> result;
	hr = compiler->Compile(&buffer, args.data(), static_cast<UINT32>(args.size()), handler.Get(), IID_PPV_ARGS(result.GetAddressOf()));
	RETURN_FALSE_IF_FAILED(hr);

	ComPtr<IDxcBlobUtf8> error;
	hr = result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(error.GetAddressOf()), nullptr);
	RETURN_FALSE_IF_FAILED(hr);

	if (error && error->GetStringLength() > 0)
	{
		Debug::Log("%s\n", error->GetStringPointer());
		return false;
	}

	hr = result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(blob), nullptr);
	RETURN_FALSE_IF_FAILED(hr);

	return true;
}
