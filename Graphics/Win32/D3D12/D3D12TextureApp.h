#pragma once

#include "D3D12App.h"

#if APP_WIN32 && APP_D3D12 && APP_TEXTURE

class D3D12TextureApp : public D3D12App
{
public:
	D3D12TextureApp(LPCWSTR title, UINT width, UINT height);

	virtual ~D3D12TextureApp() override = default;

protected:
	virtual bool OnInitialize() override;

	virtual void OnFinalize() override;

	virtual void OnUpdate() override;

	virtual void OnRender() override;

private:
	ComPtr<ID3D12Resource>       m_vertex_buffer;
	ComPtr<ID3D12Resource>       m_index_buffer;
	ComPtr<ID3D12Resource>       m_texture;
	ComPtr<ID3D12DescriptorHeap> m_texture_heap;
	ComPtr<ID3D12DescriptorHeap> m_sampler_heap;
	ComPtr<ID3D12RootSignature>  m_root_signature;
	ComPtr<ID3D12PipelineState>  m_pipeline_state;
};

#endif
