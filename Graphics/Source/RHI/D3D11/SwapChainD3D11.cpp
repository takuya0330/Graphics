#include "SwapChainD3D11.h"

#include "ColorTargetD3D11.h"
#include "RHI/Device.h"
#include "Win32/Debug.h"

namespace RHI {
namespace D3D11 {

CSwapChain::CSwapChain(IDevice* device)
    : m_swap_chain_desc()
    , m_raw_swap_chain()
    , m_color_back_buffer()
    , m_device(device)
    , m_ref_count(0)
{
}

bool CSwapChain::Initialize(const SwapChainDesc* swap_chain_desc, IDXGIFactory6* dxgi_factory6)
{
	ASSERT_RETURN(swap_chain_desc, false);
	m_swap_chain_desc = (*swap_chain_desc);

	RawDevice raw_device;
	ASSERT_RETURN(m_device->GetRawDevice(&raw_device), false);

	HRESULT hr = S_OK;

	MSWRL::ComPtr<IDXGISwapChain> dxgi_swap_chain;
	{
		DXGI_SWAP_CHAIN_DESC dxgi_swap_chain_desc = {};
		{
			dxgi_swap_chain_desc.BufferDesc.Width                   = m_swap_chain_desc.width;
			dxgi_swap_chain_desc.BufferDesc.Height                  = m_swap_chain_desc.height;
			dxgi_swap_chain_desc.BufferDesc.RefreshRate.Numerator   = 60;
			dxgi_swap_chain_desc.BufferDesc.RefreshRate.Denominator = 1;
			dxgi_swap_chain_desc.BufferDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
			dxgi_swap_chain_desc.BufferDesc.ScanlineOrdering        = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
			dxgi_swap_chain_desc.BufferDesc.Scaling                 = DXGI_MODE_SCALING_UNSPECIFIED;
			dxgi_swap_chain_desc.SampleDesc.Count                   = 1;
			dxgi_swap_chain_desc.SampleDesc.Quality                 = 0;
			dxgi_swap_chain_desc.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			dxgi_swap_chain_desc.BufferCount                        = m_swap_chain_desc.num_buffers;
			dxgi_swap_chain_desc.OutputWindow                       = static_cast<HWND>(m_swap_chain_desc.hwnd);
			dxgi_swap_chain_desc.Windowed                           = TRUE;
			dxgi_swap_chain_desc.SwapEffect                         = DXGI_SWAP_EFFECT_DISCARD;
			dxgi_swap_chain_desc.Flags                              = 0;
		}
		hr = dxgi_factory6->CreateSwapChain(
		    raw_device.d3d11_device.Get(),
		    &dxgi_swap_chain_desc,
		    dxgi_swap_chain.GetAddressOf());
		RETURN_FALSE_IF_FAILED(hr);

		hr = dxgi_swap_chain.As(&m_raw_swap_chain.dxgi_swap_chain4);
		RETURN_FALSE_IF_FAILED(hr);

		hr = dxgi_factory6->MakeWindowAssociation(static_cast<HWND>(m_swap_chain_desc.hwnd), DXGI_MWA_NO_ALT_ENTER);
		RETURN_FALSE_IF_FAILED(hr);
	}

	{
		D3D11::CColorTarget* color_target = new D3D11::CColorTarget(m_device);
		ASSERT_RETURN(color_target, false);
		ASSERT_RETURN(color_target->Initialize(this), false);

        m_color_back_buffer = color_target;
		m_color_back_buffer->AddRef();
	}

	return true;
}

void CSwapChain::Present()
{
	m_raw_swap_chain.dxgi_swap_chain4->Present(1, 0);
}

IColorTarget* CSwapChain::GetColorBackBuffer() const
{
	return m_color_back_buffer;
}

bool CSwapChain::GetRawSwapChain(RawSwapChain* raw_swap_chain) const
{
	ASSERT_RETURN(m_raw_swap_chain.dxgi_swap_chain4, false);

	(*raw_swap_chain) = m_raw_swap_chain;

	return true;
}

void CSwapChain::AddRef() noexcept
{
	m_ref_count.fetch_add(1);
}

void CSwapChain::Release() noexcept
{
	m_ref_count.fetch_sub(1);
	if (m_ref_count.load() == 0)
	{
		m_device->Release(this);
	}
}

uint32_t CSwapChain::GetRefCount() const noexcept
{
	return m_ref_count.load();
}

bool CSwapChain::GetDevice(IDevice** device) const
{
	ASSERT_RETURN(m_device, false);

	(*device) = m_device;

	return true;
}

void CSwapChain::dispose() noexcept
{
	delete this;
}

} // namespace D3D11
} // namespace RHI
