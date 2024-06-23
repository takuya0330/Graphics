#pragma once

#include "RHI/ColorTarget.h"

namespace RHI {
namespace D3D11 {

class CColorTarget : public IColorTarget
{
public:
	CColorTarget(IDevice* device);

    bool Initialize(const ColorTargetDesc* render_target_desc);

    bool Initialize(ISwapChain* swap_chain);

public: // IRenderTarget override.
	bool GetRawColorTarget(RawColorTarget* raw_render_target) const override;

public: // IDeviceObject override.
	void AddRef() noexcept override;

	void Release() noexcept override;

	uint32_t GetRefCount() const noexcept override;

	bool GetDevice(IDevice** device) const override;

private: // IDeviceObject override.
	void dispose() noexcept override;

private:
	ColorTargetDesc      m_color_target_desc;
	RawColorTarget       m_raw_color_target;
	IDevice*             m_device;
	std::atomic_uint32_t m_ref_count;
};

} // namespace D3D11
} // namespace RHI
