#include "ColorTargetD3D11.h"

#include "DeviceD3D11.h"
#include "SwapChainD3D11.h"
#include "Win32/Debug.h"

namespace RHI {
namespace D3D11 {

CColorTarget::CColorTarget(IDevice* device)
    : m_color_target_desc()
    , m_raw_color_target()
    , m_device(device)
    , m_ref_count(0)
{
}

bool CColorTarget::Initialize(const ColorTargetDesc* color_target_desc)
{
	ASSERT_RETURN(color_target_desc, false);
	m_color_target_desc = (*color_target_desc);

	RawDevice raw_device;
	ASSERT_RETURN(m_device->GetRawDevice(&raw_device), false);

	HRESULT hr = S_OK;

	return true;
}

bool CColorTarget::Initialize(ISwapChain* swap_chain)
{
	ASSERT_RETURN(swap_chain, false);

	RawSwapChain raw_swap_chain;
	ASSERT_RETURN(swap_chain->GetRawSwapChain(&raw_swap_chain), false);

	RawDevice raw_device;
	ASSERT_RETURN(m_device->GetRawDevice(&raw_device), false);

	HRESULT hr = S_OK;

	MSWRL::ComPtr<ID3D11Texture2D> back_buffer;
	hr = raw_swap_chain.dxgi_swap_chain4->GetBuffer(0, IID_PPV_ARGS(back_buffer.GetAddressOf()));
	RETURN_FALSE_IF_FAILED(hr);

	hr = raw_device.d3d11_device->CreateRenderTargetView(back_buffer.Get(), nullptr, m_raw_color_target.d3d11_render_target_view.GetAddressOf());
	RETURN_FALSE_IF_FAILED(hr);

	return true;
}

bool CColorTarget::GetRawColorTarget(RawColorTarget* raw_color_target) const
{
	ASSERT_RETURN(m_raw_color_target.d3d11_render_target_view, false);

	(*raw_color_target) = m_raw_color_target;

	return true;
}

void CColorTarget::AddRef() noexcept
{
	m_ref_count.fetch_add(1);
}

void CColorTarget::Release() noexcept
{
	m_ref_count.fetch_sub(1);
	if (m_ref_count.load() == 0)
	{
		m_device->Release(this);
	}
}

uint32_t CColorTarget::GetRefCount() const noexcept
{
	return m_ref_count.load();
}

bool CColorTarget::GetDevice(IDevice** device) const
{
	ASSERT_RETURN(m_device, false);

	(*device) = m_device;

	return true;
}

void CColorTarget::dispose() noexcept
{
	delete this;
}

} // namespace D3D11
} // namespace RHI
