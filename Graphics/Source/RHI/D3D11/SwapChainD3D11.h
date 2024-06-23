#pragma once

#include "RHI/SwapChain.h"

namespace RHI {
namespace D3D11 {

class CSwapChain : public ISwapChain
{
public:
	CSwapChain(IDevice* device);

	bool Initialize(const SwapChainDesc* swap_chain_desc, IDXGIFactory6* dxgi_factory6);

public: // ISwapChain override.
	void Present() override;

	IColorTarget* GetColorBackBuffer() const override;

	bool GetRawSwapChain(RawSwapChain* raw_swap_chain) const override;

public: // IDeviceObject override.
	void AddRef() noexcept override;

	void Release() noexcept override;

	uint32_t GetRefCount() const noexcept override;

	bool GetDevice(IDevice** device) const override;

private: // IDeviceObject override.
	void dispose() noexcept override;

private:
	SwapChainDesc        m_swap_chain_desc;
	RawSwapChain         m_raw_swap_chain;
	IColorTarget*        m_color_back_buffer;
	IDevice*             m_device;
	std::atomic_uint32_t m_ref_count;
};

} // namespace D3D11
} // namespace RHI
