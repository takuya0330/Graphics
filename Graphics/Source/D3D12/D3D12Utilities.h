#pragma once

#include "Platform.h"
#include "Win32/Debug.h"

namespace Utilities {

template<class DeviceX>
concept DeviceType = std::is_base_of_v<ID3D12Device, DeviceX>;

template<class GraphicsCommandListX>
concept GraphicsCommandListType = std::is_base_of_v<ID3D12GraphicsCommandList, GraphicsCommandListX>;

template<class FenceX>
concept FenceType = std::is_base_of_v<ID3D12Fence, FenceX>;

template<class ResourceX>
concept ResourceType = std::is_base_of_v<ID3D12Resource, ResourceX>;

inline D3D12_RESOURCE_BARRIER CreateResourceBarrierTrantision(
    ID3D12Resource*       d3d12_resource,
    D3D12_RESOURCE_STATES before_state,
    D3D12_RESOURCE_STATES after_state)
{
	D3D12_RESOURCE_BARRIER barrier = {};
	{
		barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource   = d3d12_resource;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrier.Transition.StateBefore = before_state;
		barrier.Transition.StateAfter  = after_state;
	}
	return barrier;
}

inline HRESULT CreateDevice(
    IDXGIAdapter*     dxgi_adapter,
    D3D_FEATURE_LEVEL d3d_feature_level,
    DeviceType auto** d3d12_device)
{
	auto hr = ::D3D12CreateDevice(dxgi_adapter, d3d_feature_level, IID_PPV_ARGS(d3d12_device));
	RETURN_HRESULT_IF_FAILED(hr);

	return hr;
}

inline HRESULT CreateCommandList(
    ID3D12Device*                  d3d12_device,
    D3D12_COMMAND_LIST_TYPE        d3d12_command_list_type,
    ID3D12CommandAllocator*        d3d12_command_allocator,
    GraphicsCommandListType auto** d3d12_command_list,
    bool                           is_close = true)
{
	auto hr = d3d12_device->CreateCommandList(
	    0,
	    d3d12_command_list_type,
	    d3d12_command_allocator,
	    nullptr,
	    IID_PPV_ARGS(d3d12_command_list));
	RETURN_HRESULT_IF_FAILED(hr);

	if (is_close)
	{
		hr = (*d3d12_command_list)->Close();
		RETURN_HRESULT_IF_FAILED(hr);
	}

	return hr;
}

inline HRESULT CreateFence(
    ID3D12Device*    d3d12_device,
    FenceType auto** d3d12_fence)
{
	auto hr = d3d12_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(d3d12_fence));
	RETURN_HRESULT_IF_FAILED(hr);

	return hr;
}

inline HRESULT CreateResource(
    ID3D12Device*              d3d12_device,
    D3D12_HEAP_TYPE            d3d12_heap_type,
    const D3D12_RESOURCE_DESC* d3d12_resource_desc,
    D3D12_RESOURCE_STATES      d3d12_resource_state,
    const D3D12_CLEAR_VALUE*   d3d12_clear_value,
    ResourceType auto**        d3d12_resource)
{
	D3D12_HEAP_PROPERTIES heap_properties = {
		.Type                 = d3d12_heap_type,
		.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
		.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
		.CreationNodeMask     = 0,
		.VisibleNodeMask      = 0
	};

	auto hr = d3d12_device->CreateCommittedResource(
	    &heap_properties,
	    D3D12_HEAP_FLAG_NONE,
	    d3d12_resource_desc,
	    d3d12_resource_state,
	    d3d12_clear_value,
	    IID_PPV_ARGS(d3d12_resource));
	RETURN_HRESULT_IF_FAILED(hr);

	return hr;
}

inline HRESULT CreateBuffer(
    ID3D12Device*         d3d12_device,
    D3D12_HEAP_TYPE       d3d12_heap_type,
    UINT64                size,
    D3D12_RESOURCE_FLAGS  d3d12_resource_flags,
    D3D12_RESOURCE_STATES d3d12_resource_state,
    ResourceType auto**   d3d12_resource)
{
	D3D12_RESOURCE_DESC resource_desc = {
		.Dimension        = D3D12_RESOURCE_DIMENSION_BUFFER,
		.Alignment        = 0,
		.Width            = size,
		.Height           = 1,
		.DepthOrArraySize = 1,
		.MipLevels        = 1,
		.Format           = DXGI_FORMAT_UNKNOWN,
		.SampleDesc       = { .Count = 1, .Quality = 0 },
		.Layout           = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
		.Flags            = d3d12_resource_flags
	};
	return CreateResource(
	    d3d12_device,
	    d3d12_heap_type,
	    &resource_desc,
	    d3d12_resource_state,
	    nullptr,
	    d3d12_resource);
}

inline HRESULT CreateTexture2D(
    ID3D12Device*            d3d12_device,
    D3D12_HEAP_TYPE          d3d12_heap_type,
    UINT64                   width,
    UINT                     height,
    UINT16                   array_size,
    UINT16                   mip_levels,
    DXGI_FORMAT              dxgi_format,
    D3D12_RESOURCE_FLAGS     d3d12_resource_flags,
    D3D12_RESOURCE_STATES    d3d12_resource_state,
    const D3D12_CLEAR_VALUE* d3d12_clear_value,
    ResourceType auto**      d3d12_resource)
{
	D3D12_RESOURCE_DESC resource_desc = {
		.Dimension        = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
		.Alignment        = 0,
		.Width            = width,
		.Height           = height,
		.DepthOrArraySize = array_size,
		.MipLevels        = mip_levels,
		.Format           = dxgi_format,
		.SampleDesc       = { .Count = 1, .Quality = 0 },
		.Layout           = D3D12_TEXTURE_LAYOUT_UNKNOWN,
		.Flags            = d3d12_resource_flags
	};
	return CreateResource(
	    d3d12_device,
	    d3d12_heap_type,
	    &resource_desc,
	    d3d12_resource_state,
	    d3d12_clear_value,
	    d3d12_resource);
}

inline HRESULT Reset(
    ID3D12CommandAllocator*    d3d12_command_allocator,
    ID3D12GraphicsCommandList* d3d12_command_list)
{
	auto hr = d3d12_command_allocator->Reset();
	RETURN_HRESULT_IF_FAILED(hr);

	hr = d3d12_command_list->Reset(d3d12_command_allocator, nullptr);
	RETURN_HRESULT_IF_FAILED(hr);

	return hr;
}

inline void SetViewport(
    ID3D12GraphicsCommandList* d3d12_command_list,
    float                      width,
    float                      height)
{
	D3D12_VIEWPORT viewport = {
		.TopLeftX = 0,
		.TopLeftY = 0,
		.Width    = width,
		.Height   = height,
		.MinDepth = 0.0f,
		.MaxDepth = 1.0f
	};
	d3d12_command_list->RSSetViewports(1, &viewport);
}

inline void SetScissorRect(
    ID3D12GraphicsCommandList* d3d12_command_list,
    LONG                       width,
    LONG                       height)
{
	D3D12_RECT scissor = {
		.left   = 0,
		.top    = 0,
		.right  = width,
		.bottom = height
	};
	d3d12_command_list->RSSetScissorRects(1, &scissor);
}

inline void ResourceBarrierTransition(
    ID3D12GraphicsCommandList* d3d12_command_list,
    ID3D12Resource*            d3d12_resource,
    D3D12_RESOURCE_STATES      before_state,
    D3D12_RESOURCE_STATES      after_state)
{
	auto barrier = CreateResourceBarrierTrantision(d3d12_resource, before_state, after_state);
	d3d12_command_list->ResourceBarrier(1, &barrier);
}

inline void ResourceBarrierUAV(
    ID3D12GraphicsCommandList* d3d12_command_list,
    ID3D12Resource*            d3d12_resource)
{
	D3D12_RESOURCE_BARRIER barrier = {};
	{
		barrier.Type          = D3D12_RESOURCE_BARRIER_TYPE_UAV;
		barrier.Flags         = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.UAV.pResource = d3d12_resource;
	}
	d3d12_command_list->ResourceBarrier(1, &barrier);
}

inline void ExecuteCommandList(
    ID3D12CommandQueue*        d3d12_command_queue,
    ID3D12GraphicsCommandList* d3d12_command_list,
    bool                       is_close = true)
{
	if (is_close)
	{
		auto hr = d3d12_command_list->Close();
		ASSERT_IF_FAILED(hr);
	}

	ID3D12CommandList* command_lists[] = { d3d12_command_list };
	d3d12_command_queue->ExecuteCommandLists(_countof(command_lists), command_lists);
}

} // namespace Utilities
