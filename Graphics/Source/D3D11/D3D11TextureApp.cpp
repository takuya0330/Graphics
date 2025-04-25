#include "D3D11TextureApp.h"

#include "Win32/Image.h"
#include "Win32/String.h"

namespace {

struct Vertex
{
	float position[3];
	float texcoord[2];
};

} // namespace

D3D11TextureApp::D3D11TextureApp(LPCWSTR title, UINT width, UINT height)
    : D3D11App(title, width, height)
    , m_vertex_buffer()
    , m_index_buffer()
    , m_texture_view()
    , m_sampler()
    , m_vertex_shader()
    , m_input_layout()
    , m_pixel_shader()
{
}

bool D3D11TextureApp::OnInitialize()
{
	ASSERT_RETURN(D3D11App::OnInitialize(), false);

	HRESULT hr = S_OK;

	{
		const Vertex vertices[] = {
			{ { -0.5, +0.5 * m_aspect_ratio, 0 }, { 0, 0 } },
			{ { +0.5, +0.5 * m_aspect_ratio, 0 }, { 1, 0 } },
			{ { -0.5, -0.5 * m_aspect_ratio, 0 }, { 0, 1 } },
			{ { +0.5, -0.5 * m_aspect_ratio, 0 }, { 1, 1 } },
		};

		D3D11_BUFFER_DESC buffer_desc = {};
		{
			buffer_desc.ByteWidth           = sizeof(vertices);
			buffer_desc.Usage               = D3D11_USAGE_IMMUTABLE;
			buffer_desc.BindFlags           = D3D11_BIND_VERTEX_BUFFER;
			buffer_desc.CPUAccessFlags      = 0;
			buffer_desc.MiscFlags           = 0;
			buffer_desc.StructureByteStride = 0;
		}
		D3D11_SUBRESOURCE_DATA subresource = {};
		{
			subresource.pSysMem          = vertices;
			subresource.SysMemPitch      = 0;
			subresource.SysMemSlicePitch = 0;
		}
		hr = m_d3d11_device->CreateBuffer(&buffer_desc, &subresource, m_vertex_buffer.GetAddressOf());
		RETURN_FALSE_IF_FAILED(hr);
	}

	{
		const uint32_t indices[] = { 0, 1, 2, 2, 1, 3 };

		D3D11_BUFFER_DESC buffer_desc = {};
		{
			buffer_desc.ByteWidth           = sizeof(indices);
			buffer_desc.Usage               = D3D11_USAGE_IMMUTABLE;
			buffer_desc.BindFlags           = D3D11_BIND_INDEX_BUFFER;
			buffer_desc.CPUAccessFlags      = 0;
			buffer_desc.MiscFlags           = 0;
			buffer_desc.StructureByteStride = 0;
		}
		D3D11_SUBRESOURCE_DATA subresource = {};
		{
			subresource.pSysMem          = indices;
			subresource.SysMemPitch      = 0;
			subresource.SysMemSlicePitch = 0;
		}
		hr = m_d3d11_device->CreateBuffer(&buffer_desc, &subresource, m_index_buffer.GetAddressOf());
		RETURN_FALSE_IF_FAILED(hr);
	}

	{
		Image image(UTF16To8(m_asset_dir + L"uv.png").c_str());

		ComPtr<ID3D11Texture2D> texture;

		D3D11_TEXTURE2D_DESC tex_desc = {};
		{
			tex_desc.Width              = image.GetWidth();
			tex_desc.Height             = image.GetHeight();
			tex_desc.MipLevels          = 1;
			tex_desc.ArraySize          = 1;
			tex_desc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM;
			tex_desc.SampleDesc.Count   = 1;
			tex_desc.SampleDesc.Quality = 0;
			tex_desc.Usage              = D3D11_USAGE_IMMUTABLE;
			tex_desc.BindFlags          = D3D11_BIND_SHADER_RESOURCE;
			tex_desc.CPUAccessFlags     = 0;
			tex_desc.MiscFlags          = 0;
		}
		D3D11_SUBRESOURCE_DATA subresource = {};
		{
			subresource.pSysMem          = image.GetBuffer();
			subresource.SysMemPitch      = image.GetStride();
			subresource.SysMemSlicePitch = 0;
		}
		hr = m_d3d11_device->CreateTexture2D(&tex_desc, &subresource, texture.GetAddressOf());
		RETURN_FALSE_IF_FAILED(hr);

		D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
		{
			srv_desc.Format              = DXGI_FORMAT_R8G8B8A8_UNORM;
			srv_desc.ViewDimension       = D3D11_SRV_DIMENSION_TEXTURE2D;
			srv_desc.Texture2D.MipLevels = 1;
		}
		hr = m_d3d11_device->CreateShaderResourceView(texture.Get(), &srv_desc, m_texture_view.GetAddressOf());
		RETURN_FALSE_IF_FAILED(hr);
	}

	{
		D3D11_SAMPLER_DESC sampler_desc = {};
		{
			sampler_desc.Filter         = D3D11_ENCODE_BASIC_FILTER(D3D11_FILTER_TYPE_LINEAR, D3D11_FILTER_TYPE_LINEAR, D3D11_FILTER_TYPE_LINEAR, D3D11_FILTER_REDUCTION_TYPE_STANDARD);
			sampler_desc.AddressU       = D3D11_TEXTURE_ADDRESS_WRAP;
			sampler_desc.AddressV       = D3D11_TEXTURE_ADDRESS_WRAP;
			sampler_desc.AddressW       = D3D11_TEXTURE_ADDRESS_WRAP;
			sampler_desc.MinLOD         = -FLT_MAX;
			sampler_desc.MaxLOD         = FLT_MAX;
			sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		}
		hr = m_d3d11_device->CreateSamplerState(&sampler_desc, m_sampler.GetAddressOf());
	}

	ComPtr<ID3DBlob> vs;
	if (!loadShader(L"PositionTexcoord_vs.hlsl", "main", "vs_5_0", vs.GetAddressOf()))
		return false;

	ComPtr<ID3DBlob> ps;
	if (!loadShader(L"PositionTexcoord_ps.hlsl", "main", "ps_5_0", ps.GetAddressOf()))
		return false;

	hr = m_d3d11_device->CreateVertexShader(vs->GetBufferPointer(), vs->GetBufferSize(), nullptr, m_vertex_shader.GetAddressOf());
	RETURN_FALSE_IF_FAILED(hr);

#if 0
	D3D11_INPUT_ELEMENT_DESC input_layout[] = {
		{"POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};
	hr = m_d3d11_device->CreateInputLayout(input_layout, _countof(input_layout), vs->GetBufferPointer(), vs->GetBufferSize(), m_input_layout.GetAddressOf());
	RETURN_FALSE_IF_FAILED(hr);

	hr = m_d3d11_device->CreatePixelShader(ps->GetBufferPointer(), ps->GetBufferSize(), nullptr, m_pixel_shader.GetAddressOf());
	RETURN_FALSE_IF_FAILED(hr);
#else
	if (!createVertexShader(vs.Get(), m_vertex_shader.GetAddressOf(), m_input_layout.GetAddressOf()))
		return false;

	if (!createPixelShader(ps.Get(), m_pixel_shader.GetAddressOf()))
		return false;
#endif

	return true;
}

void D3D11TextureApp::OnFinalize()
{
}

void D3D11TextureApp::OnUpdate()
{
}

void D3D11TextureApp::OnRender()
{
	setViewport(static_cast<float>(m_width), static_cast<float>(m_height));
	setBackBuffer();

	{
		m_d3d11_immediate_context->VSSetShader(m_vertex_shader.Get(), nullptr, 0);
		m_d3d11_immediate_context->PSSetShader(m_pixel_shader.Get(), nullptr, 0);

		m_d3d11_immediate_context->PSSetShaderResources(0, 1, m_texture_view.GetAddressOf());
		m_d3d11_immediate_context->PSSetSamplers(0, 1, m_sampler.GetAddressOf());

		UINT offset = 0;
		UINT stride = sizeof(Vertex);
		m_d3d11_immediate_context->IASetInputLayout(m_input_layout.Get());
		m_d3d11_immediate_context->IASetVertexBuffers(0, 1, m_vertex_buffer.GetAddressOf(), &stride, &offset);
		m_d3d11_immediate_context->IASetIndexBuffer(m_index_buffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		m_d3d11_immediate_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_d3d11_immediate_context->DrawIndexed(6, 0, 0);
	}

	present(1);
}
