#pragma once

#include "DeviceObject.h"
#include "Platform.h"

namespace RHI {

struct ColorTargetDesc
{
};

struct RawColorTarget
{
#if _D3D11
	MSWRL::ComPtr<ID3D11RenderTargetView> d3d11_render_target_view;
#else
#error Not supported.
#endif
};

class IColorTarget : public IDeviceObject
{
public:
	virtual ~IColorTarget() = default;

	virtual bool GetRawColorTarget(RawColorTarget* raw_color_target) const = 0;
};

} // namespace RHI
