#pragma once

#include <array>

#include "Win32/Win32App.h"

class D3D12WaitableSwapChainApp : public Win32App
{
public:
	D3D12WaitableSwapChainApp(LPCWSTR title, UINT width, UINT height);

	virtual ~D3D12WaitableSwapChainApp() override = default;

protected:
	virtual bool OnInitialize() override;

	virtual void OnFinalize() override;

	virtual void OnUpdate() override;

	virtual void OnRender() override;

protected:
	void reset();

	void setViewport(float width, float height);

	void setScissorRect(LONG width, LONG height);

	void setBackBuffer();

	void executeCommandList();

	void present(UINT sync_interval);

    void waitSwapChain();

	void waitPreviousFrame();

	void waitForGPU(ID3D12CommandQueue* cmd_queue);

protected:
	bool createCommandQueue(
	    const D3D12_COMMAND_LIST_TYPE type,
	    ID3D12CommandQueue**          cmd_queue);

	bool createCommandAllocator(
	    const D3D12_COMMAND_LIST_TYPE type,
	    ID3D12CommandAllocator**      cmd_allocator);

	bool createCommandList(
	    const D3D12_COMMAND_LIST_TYPE type,
	    ID3D12CommandAllocator*       cmd_allocator,
	    ID3D12GraphicsCommandList**   cmd_list);

	bool createDescriptorHeap(
	    const D3D12_DESCRIPTOR_HEAP_TYPE  heap_type,
	    const UINT                        size,
	    const D3D12_DESCRIPTOR_HEAP_FLAGS flags,
	    ID3D12DescriptorHeap**            descriptor_heap);

	bool createTexture2D(
	    const D3D12_HEAP_TYPE       heap_type,
	    const UINT64                width,
	    const UINT                  height,
	    const DXGI_FORMAT           format,
	    const D3D12_RESOURCE_FLAGS  flags,
	    const D3D12_RESOURCE_STATES state,
	    const D3D12_CLEAR_VALUE*    clear_value,
	    ID3D12Resource**            resource);

protected:
	static constexpr UINT kBackBufferCount = 3;

    template<class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

    template<class T>
	using Array3 = std::array<T, kBackBufferCount>;

	template<class T>
	using ComPtr3 = Array3<ComPtr<T>>;

private:
	ComPtr<ID3D12Device>              m_device;
	ComPtr<ID3D12CommandQueue>        m_gfx_cmd_queue;
	ComPtr3<ID3D12CommandAllocator>   m_gfx_cmd_allocators;
	ComPtr<ID3D12GraphicsCommandList> m_gfx_cmd_list;
	ComPtr<IDXGISwapChain4>           m_swap_chain4;
	UINT                              m_back_buffer_index;
	HANDLE                            m_frame_latency_waitable_object;
	ComPtr<ID3D12DescriptorHeap>      m_rtv_heap;
	UINT                              m_rtv_heap_size;
	ComPtr3<ID3D12Resource>           m_back_buffers;
	ComPtr<ID3D12DescriptorHeap>      m_dsv_heap;
	ComPtr<ID3D12Resource>            m_depth_buffer;
	ComPtr3<ID3D12Fence>              m_fences;
	Array3<UINT64>                    m_fence_values;
};
