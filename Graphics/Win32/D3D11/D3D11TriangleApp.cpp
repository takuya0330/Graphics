#include "D3D11TriangleApp.h"

#if APP_WIN32 && APP_D3D11 && APP_TRIANGLE

namespace {

struct Vertex
{
	float position[3];
	float color[4];
};

} // namespace

D3D11TriangleApp::D3D11TriangleApp(LPCWSTR title, UINT width, UINT height)
    : D3D11App(title, width, height)
    , m_vertex_buffer()
    , m_vertex_shader()
    , m_input_layout()
    , m_pixel_shader()
{
}

bool D3D11TriangleApp::OnInitialize()
{
	ASSERT_RETURN(D3D11App::OnInitialize(), false);

	const Vertex vertices[] = {
		{{ +0, +1, +0 },  { 1, 0, 0, 1 }},
		{ { +1, -1, +0 }, { 0, 1, 0, 1 }},
		{ { -1, -1, +0 }, { 0, 0, 1, 1 }},
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
	auto hr = m_d3d11_device->CreateBuffer(&buffer_desc, &subresource, m_vertex_buffer.GetAddressOf());
	RETURN_FALSE_IF_FAILED(hr);

	ComPtr<ID3DBlob> vs;
	if (!loadShader(L"../../Win32/HLSL/PositionColor_vs.hlsl", "main", "vs_5_0", vs.GetAddressOf()))
		return false;

	ComPtr<ID3DBlob> ps;
	if (!loadShader(L"../../Win32/HLSL/PositionColor_ps.hlsl", "main", "ps_5_0", ps.GetAddressOf()))
		return false;

	hr = m_d3d11_device->CreateVertexShader(vs->GetBufferPointer(), vs->GetBufferSize(), nullptr, m_vertex_shader.GetAddressOf());
	RETURN_FALSE_IF_FAILED(hr);

	D3D11_INPUT_ELEMENT_DESC input_layout[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{ "COLOR",   0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};
	hr = m_d3d11_device->CreateInputLayout(input_layout, _countof(input_layout), vs->GetBufferPointer(), vs->GetBufferSize(), m_input_layout.GetAddressOf());
	RETURN_FALSE_IF_FAILED(hr);

	hr = m_d3d11_device->CreatePixelShader(ps->GetBufferPointer(), ps->GetBufferSize(), nullptr, m_pixel_shader.GetAddressOf());
	RETURN_FALSE_IF_FAILED(hr);

	return true;
}

void D3D11TriangleApp::OnFinalize()
{
}

void D3D11TriangleApp::OnUpdate()
{
}

void D3D11TriangleApp::OnRender()
{
	setViewport(static_cast<float>(m_width), static_cast<float>(m_height));
	setBackBuffer();

	{
		m_d3d11_immediate_context->VSSetShader(m_vertex_shader.Get(), nullptr, 0);
		m_d3d11_immediate_context->PSSetShader(m_pixel_shader.Get(), nullptr, 0);

		UINT offset = 0;
		UINT stride = sizeof(Vertex);
		m_d3d11_immediate_context->IASetInputLayout(m_input_layout.Get());
		m_d3d11_immediate_context->IASetVertexBuffers(0, 1, m_vertex_buffer.GetAddressOf(), &stride, &offset);
		m_d3d11_immediate_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_d3d11_immediate_context->Draw(3, 0);
	}

	present(1);
}

#endif
