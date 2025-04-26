#pragma once

#include "D3D12App.h"

class D3D12RayTracingTriangleApp : public D3D12App
{
public:
	D3D12RayTracingTriangleApp(LPCWSTR title, UINT width, UINT height);

	virtual ~D3D12RayTracingTriangleApp() override = default;

protected:
	virtual bool OnInitialize() override;

	virtual void OnFinalize() override;

	virtual void OnUpdate() override;

	virtual void OnRender() override;

private:
	ComPtr<ID3D12Device5>              m_device5;
	ComPtr<ID3D12GraphicsCommandList4> m_gfx_cmd_list4;

	ComPtr<ID3D12Resource>       m_vertex_buffer;
	ComPtr<ID3D12Resource>       m_blas;
	ComPtr<ID3D12Resource>       m_tlas;
	ComPtr<ID3D12RootSignature>  m_global_root_signature;
	ComPtr<ID3D12StateObject>    m_state_object;
	ComPtr<ID3D12Resource>       m_dxr_output;
	ComPtr<ID3D12DescriptorHeap> m_resource_heap;
	uint32_t                     m_resource_alloc_size;
	ComPtr<ID3D12Resource>       m_shader_table;
	D3D12_DISPATCH_RAYS_DESC     m_dispatch_desc;
};
