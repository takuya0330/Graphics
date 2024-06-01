#include "D3D11App.h"

#include <vector>

D3D11App::D3D11App(LPCWSTR title, UINT width, UINT height)
    : Win32App(title, width, height)
    , m_d3d11_device()
    , m_d3d11_immediate_context()
    , m_dxgi_swap_chain()
    , m_d3d11_render_target_view()
    , m_d3d11_depth_stencil_view()
{
}

bool D3D11App::OnInitialize()
{
	HRESULT hr = S_OK;

	ComPtr<IDXGIFactory> dxgi_factory;
	{
		hr = ::CreateDXGIFactory(IID_PPV_ARGS(dxgi_factory.GetAddressOf()));
		RETURN_FALSE_IF_FAILED(hr);
	}

	ComPtr<IDXGIAdapter> dxgi_adapter;
	{
		std::vector<ComPtr<IDXGIAdapter>> dxgi_adapters;
		{
			IDXGIAdapter* dxgi_adapter;
			for (UINT i = 0; dxgi_factory->EnumAdapters(i, &dxgi_adapter) != DXGI_ERROR_NOT_FOUND; ++i)
			{
				dxgi_adapters.emplace_back(dxgi_adapter);
			}
		}
		ASSERT_RETURN(!dxgi_adapters.empty(), false);
		dxgi_adapter = dxgi_adapters.at(0); // TODO
	}

	UINT create_device_flags = 0;
#if defined(_DEBUG)
	create_device_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	{
		constexpr D3D_DRIVER_TYPE kDriverTypes[] = {
			D3D_DRIVER_TYPE_UNKNOWN,
			D3D_DRIVER_TYPE_HARDWARE,
			D3D_DRIVER_TYPE_WARP,
			D3D_DRIVER_TYPE_REFERENCE,
		};

		constexpr D3D_FEATURE_LEVEL kFeatureLevels[] = {
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
			D3D_FEATURE_LEVEL_9_3,
			D3D_FEATURE_LEVEL_9_2,
			D3D_FEATURE_LEVEL_9_1,
		};

		for (const auto& driver_type : kDriverTypes)
		{
			D3D_FEATURE_LEVEL current_feature_level;
			hr = D3D11CreateDevice(dxgi_adapter.Get(), driver_type, nullptr, create_device_flags, kFeatureLevels, _countof(kFeatureLevels), D3D11_SDK_VERSION, m_d3d11_device.GetAddressOf(), &current_feature_level, m_d3d11_immediate_context.GetAddressOf());
			if (SUCCEEDED(hr))
				break;
			RETURN_FALSE_IF_FAILED(hr);
		}
	}

	{
		DXGI_SWAP_CHAIN_DESC swap_chain_desc = {};
		{
			swap_chain_desc.BufferDesc.Width                   = m_width;
			swap_chain_desc.BufferDesc.Height                  = m_height;
			swap_chain_desc.BufferDesc.RefreshRate.Numerator   = 60;
			swap_chain_desc.BufferDesc.RefreshRate.Denominator = 1;
			swap_chain_desc.BufferDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
			swap_chain_desc.BufferDesc.ScanlineOrdering        = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
			swap_chain_desc.BufferDesc.Scaling                 = DXGI_MODE_SCALING_UNSPECIFIED;
			swap_chain_desc.SampleDesc.Count                   = 1;
			swap_chain_desc.SampleDesc.Quality                 = 0;
			swap_chain_desc.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swap_chain_desc.BufferCount                        = kBackBufferCount;
			swap_chain_desc.OutputWindow                       = m_hwnd;
			swap_chain_desc.Windowed                           = TRUE;
			swap_chain_desc.SwapEffect                         = DXGI_SWAP_EFFECT_DISCARD;
			swap_chain_desc.Flags                              = 0;
		}
		hr = dxgi_factory->CreateSwapChain(m_d3d11_device.Get(), &swap_chain_desc, m_dxgi_swap_chain.GetAddressOf());
		RETURN_FALSE_IF_FAILED(hr);

		hr = dxgi_factory->MakeWindowAssociation(m_hwnd, DXGI_MWA_NO_ALT_ENTER);
		RETURN_FALSE_IF_FAILED(hr);
	}

	{
		ComPtr<ID3D11Texture2D> back_buffer;

		hr = m_dxgi_swap_chain->GetBuffer(0, IID_PPV_ARGS(back_buffer.GetAddressOf()));
		RETURN_FALSE_IF_FAILED(hr);

		hr = m_d3d11_device->CreateRenderTargetView(back_buffer.Get(), nullptr, m_d3d11_render_target_view.GetAddressOf());
		RETURN_FALSE_IF_FAILED(hr);
	}

	{
		ComPtr<ID3D11Texture2D> depth_buffer;
		D3D11_TEXTURE2D_DESC    depth_buffer_desc = {};
		{
			depth_buffer_desc.Width              = m_width;
			depth_buffer_desc.Height             = m_height;
			depth_buffer_desc.MipLevels          = 1;
			depth_buffer_desc.ArraySize          = 1;
			depth_buffer_desc.Format             = DXGI_FORMAT_D24_UNORM_S8_UINT;
			depth_buffer_desc.SampleDesc.Count   = 1;
			depth_buffer_desc.SampleDesc.Quality = 0;
			depth_buffer_desc.Usage              = D3D11_USAGE_DEFAULT;
			depth_buffer_desc.BindFlags          = D3D11_BIND_DEPTH_STENCIL;
			depth_buffer_desc.CPUAccessFlags     = 0;
			depth_buffer_desc.MiscFlags          = 0;
		}
		hr = m_d3d11_device->CreateTexture2D(&depth_buffer_desc, 0, depth_buffer.GetAddressOf());
		RETURN_FALSE_IF_FAILED(hr);

		D3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc = {};
		{
			depth_stencil_view_desc.Format             = DXGI_FORMAT_D24_UNORM_S8_UINT;
			depth_stencil_view_desc.ViewDimension      = D3D11_DSV_DIMENSION_TEXTURE2D;
			depth_stencil_view_desc.Flags              = 0;
			depth_stencil_view_desc.Texture2D.MipSlice = 0;
		}
		hr = m_d3d11_device->CreateDepthStencilView(depth_buffer.Get(), &depth_stencil_view_desc, m_d3d11_depth_stencil_view.GetAddressOf());
		RETURN_FALSE_IF_FAILED(hr);
	}

	return true;
}

void D3D11App::OnFinalize()
{
}

void D3D11App::OnUpdate()
{
}

void D3D11App::OnRender()
{
	{
		D3D11_VIEWPORT viewport = {
			.TopLeftX = 0,
			.TopLeftY = 0,
			.Width    = static_cast<float>(m_width),
			.Height   = static_cast<float>(m_height),
			.MinDepth = 0.0f,
			.MaxDepth = 1.0f
		};
		m_d3d11_immediate_context->RSSetViewports(1, &viewport);
	}

	{
		constexpr float kClearColor[] = { 0, 0, 0, 1 };
		m_d3d11_immediate_context->ClearRenderTargetView(m_d3d11_render_target_view.Get(), kClearColor);
		m_d3d11_immediate_context->ClearDepthStencilView(m_d3d11_depth_stencil_view.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		m_d3d11_immediate_context->OMSetRenderTargets(1, m_d3d11_render_target_view.GetAddressOf(), m_d3d11_depth_stencil_view.Get());
	}

	m_dxgi_swap_chain->Present(1, 0);
}
