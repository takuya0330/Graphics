#pragma once

#include "RHI/Device.h"

namespace RHI {
namespace D3D11 {

class CDevice : public IDevice
{
public:
	CDevice();

	bool Initialize(const DeviceDesc* device_desc);

public: // IDevice override.
	void Release(IDeviceObject* object) override;

    bool CreateCommandBuffer(const CommandBufferDesc* command_buffer_desc, ICommandBuffer** command_buffer) override;

    bool CreateSwapChain(const SwapChainDesc* swap_chain_desc, ISwapChain** swap_chain) override;

    //bool CreateRenderTarget(const RenderTargetDesc* render_target_desc, IRenderTarget** render_target) override;

	bool GetRawDevice(RawDevice* raw_device) const override;

private:
	DeviceDesc                         m_device_desc;
	RawDevice                          m_raw_device;
	MSWRL::ComPtr<ID3D11DeviceContext> m_d3d11_immediate_context;
	MSWRL::ComPtr<IDXGIFactory6>       m_dxgi_factory6;
};

} // namespace D3D11
} // namespace RHI
