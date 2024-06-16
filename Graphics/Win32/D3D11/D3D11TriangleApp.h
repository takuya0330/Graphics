#pragma once

#include "D3D11App.h"

#if APP_WIN32 && APP_D3D11 && APP_TRIANGLE

class D3D11TriangleApp : public D3D11App
{
public:
	D3D11TriangleApp(LPCWSTR title, UINT width, UINT height);

	virtual ~D3D11TriangleApp() override = default;

protected:
	virtual bool OnInitialize() override;

	virtual void OnFinalize() override;

	virtual void OnUpdate() override;

	virtual void OnRender() override;

private:
	ComPtr<ID3D11Buffer>       m_vertex_buffer;
	ComPtr<ID3D11VertexShader> m_vertex_shader;
	ComPtr<ID3D11InputLayout>  m_input_layout;
	ComPtr<ID3D11PixelShader>  m_pixel_shader;
};

#endif
