#pragma once

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

#include <array>

#include "Win32App.h"

#define RETURN_HRESULT_IF_FAILED(hr)             \
	if (FAILED(hr))                              \
	{                                            \
		printf("ERROR: HRESULT = 0x%08X\n", hr); \
		return hr;                               \
	}

#define RETURN_FALSE_IF_FAILED(hr)               \
	if (FAILED(hr))                              \
	{                                            \
		printf("ERROR: HRESULT = 0x%08X\n", hr); \
		return false;                            \
	}

class D3D12App : public Win32App
{
public:
	static constexpr UINT kBackBufferCount = 2;

	template<class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	template<class T>
	using DoubleBufferArray = std::array<T, kBackBufferCount>;

	template<class T>
	using ComPtrs = DoubleBufferArray<ComPtr<T>>;

public:
	D3D12App(LPCWSTR title, UINT width, UINT height);

	virtual ~D3D12App() override = default;

protected:

private:
	bool OnInitialize() override;

	void OnFinalize() override;

	void OnUpdate() override;

	void OnRender() override;

protected:
	ComPtr<ID3D12Device>              m_device;
	ComPtr<ID3D12CommandQueue>        m_gfx_cmd_queue;
	ComPtrs<ID3D12CommandAllocator>   m_gfx_cmd_allocators;
	ComPtr<ID3D12GraphicsCommandList> m_gfx_cmd_list;
	ComPtr<IDXGISwapChain4>           m_swap_chain4;
	UINT                              m_back_buffer_index;
	ComPtr<ID3D12DescriptorHeap>      m_rtv_heap;
	UINT                              m_rtv_heap_size;
	ComPtrs<ID3D12Resource>           m_back_buffers;
	ComPtr<ID3D12DescriptorHeap>      m_dsv_heap;
	UINT                              m_dsv_heap_size;
	ComPtr<ID3D12Resource>            m_depth_buffer;
	ComPtrs<ID3D12Fence>              m_frame_fences;
	DoubleBufferArray<UINT64>         m_frame_fence_values;
};
