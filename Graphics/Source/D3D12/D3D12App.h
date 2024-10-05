#pragma once

#include <vector>

#include "Win32/Win32App.h"

class D3D12App : public Win32App
{
public:
	D3D12App(LPCWSTR title, UINT width, UINT height);

	virtual ~D3D12App() override = default;

protected:
	virtual bool OnInitialize() override;

	virtual void OnFinalize() override;

	virtual void OnUpdate() override;

	virtual void OnRender() override;

protected:
	bool initD3D12(UINT num_back_buffers);

	void reset();

	void setViewport(float width, float height);

	void setScissorRect(LONG width, LONG height);

	void setBackBuffer();

	void executeCommandList();

	void present(UINT sync_interval);

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

	bool createBuffer(
	    const D3D12_HEAP_TYPE       heap_type,
	    const UINT64                size,
	    const D3D12_RESOURCE_STATES state,
	    ID3D12Resource**            resource);

	bool createTexture2D(
	    const D3D12_HEAP_TYPE       heap_type,
	    const UINT64                width,
	    const UINT                  height,
	    const DXGI_FORMAT           format,
	    const D3D12_RESOURCE_FLAGS  flags,
	    const D3D12_RESOURCE_STATES state,
	    const D3D12_CLEAR_VALUE*    clear_value,
	    ID3D12Resource**            resource);

	bool loadShader(
	    const wchar_t* filename,
	    const wchar_t* entry_point,
	    const wchar_t* shader_model,
	    IDxcBlob**     blob);

protected:
	template<class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

    template<class T>
	using ComPtrs = std::vector<ComPtr<T>>;

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
	ComPtr<ID3D12Resource>            m_depth_buffer;
	ComPtrs<ID3D12Fence>              m_fences;
	std::vector<UINT64>               m_fence_values;
};
