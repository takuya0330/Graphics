#pragma once

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

#include <d3d11.h>
#include <dxgi.h>
#include <wrl.h>

#include <array>

#include "Win32App.h"

class D3D11App : public Win32App
{
public:
	D3D11App(LPCWSTR title, UINT width, UINT height);

	virtual ~D3D11App() override = default;

protected:
	virtual bool OnInitialize() override;

	virtual void OnFinalize() override;

	virtual void OnUpdate() override;

	virtual void OnRender() override;

protected:
	static constexpr UINT kBackBufferCount = 2;

	template<class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	template<class T>
	using Array2 = std::array<T, kBackBufferCount>;

	template<class T>
	using ComPtr2 = Array2<ComPtr<T>>;

protected:
	ComPtr<ID3D11Device>           m_d3d11_device;
	ComPtr<ID3D11DeviceContext>    m_d3d11_immediate_context;
	ComPtr<IDXGISwapChain>         m_dxgi_swap_chain;
	ComPtr<ID3D11RenderTargetView> m_d3d11_render_target_view;
	ComPtr<ID3D11DepthStencilView> m_d3d11_depth_stencil_view;
};
