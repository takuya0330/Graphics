#pragma once

#include "D3D12App.h"

class D3D12ConstantBufferApp : public D3D12App
{
public:
	D3D12ConstantBufferApp(LPCWSTR title, UINT width, UINT height);

	virtual ~D3D12ConstantBufferApp() override = default;

protected:
	virtual bool OnInitialize() override;

	virtual void OnRender() override;

private:
	ComPtr<ID3D12DescriptorHeap> m_cbv_heap;
	ComPtr<ID3D12Resource>       m_constant_buffer;
	ComPtr<ID3D12Resource>       m_vertex_buffer;
	ComPtr<ID3D12RootSignature>  m_root_signature;
	ComPtr<ID3D12PipelineState>  m_pipeline_state;
};
