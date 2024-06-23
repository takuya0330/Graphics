#pragma once

#include "DeviceObject.h"
#include "Platform.h"

namespace RHI {

struct CommandBufferDesc
{
};

struct RawCommandBuffer
{
#if _D3D11
	MSWRL::ComPtr<ID3D11DeviceContext> d3d11_immediate_context;
#elif _D3D12
	MSWRL::ComPtr<ID3D12GraphicsCommandList> d3d12_gfx_command_list;
#else
#error Not supported.
#endif
};

class ICommandBuffer : public IDeviceObject
{
public:
	virtual ~ICommandBuffer() = default;

	virtual void Reset() = 0;

	virtual void ClearColorTarget(const IColorTarget* color_target, const float clear_color[4]) = 0;

	virtual void SetColorTargets(uint32_t num_colot_targets, IColorTarget* const* color_targets) = 0;

	virtual bool GetRawCommandBuffer(RawCommandBuffer* raw_command_buffer) const = 0;
};

} // namespace RHI
