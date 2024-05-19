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

#define ASSERT_IF_FAILED(hr)                     \
	if (FAILED(hr))                              \
	{                                            \
		printf("ERROR: HRESULT = 0x%08X\n", hr); \
		assert(SUCCEEDED(hr));                   \
	}

class D3D12App : public Win32App
{
public:
	static constexpr UINT kBackBufferCount = 2;

	template<class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	template<class T>
	using Array2 = std::array<T, kBackBufferCount>;

	template<class T>
	using ComPtr2 = Array2<ComPtr<T>>;

public:
	D3D12App(LPCWSTR title, UINT width, UINT height);

	virtual ~D3D12App() override = default;

protected:
	virtual void PushCommandList(ID3D12GraphicsCommandList* gfx_cmd_list);

private:
	bool OnInitialize() override;

	void OnFinalize() override;

	void OnUpdate() override;

	void OnRender() override;

protected:
	ComPtr<ID3D12Device>              m_device;
	ComPtr<ID3D12CommandQueue>        m_gfx_cmd_queue;
	ComPtr2<ID3D12CommandAllocator>   m_gfx_cmd_allocators;
	ComPtr<ID3D12GraphicsCommandList> m_gfx_cmd_list;
	ComPtr<IDXGISwapChain4>           m_swap_chain4;
	ComPtr<ID3D12DescriptorHeap>      m_rtv_heap;
	UINT                              m_rtv_heap_size;
	ComPtr2<ID3D12Resource>           m_back_buffers;
	ComPtr<ID3D12DescriptorHeap>      m_dsv_heap;
	ComPtr<ID3D12Resource>            m_depth_buffer;
	ComPtr<ID3D12Fence>               m_fence;
	UINT64                            m_fence_value;
};
