#pragma once

#include "RHI/CommandBuffer.h"

namespace RHI {
namespace D3D11 {

class CCommandBuffer : public ICommandBuffer
{
public:
	CCommandBuffer(IDevice* device);

	bool Initialize(const CommandBufferDesc* command_buffer_desc, ID3D11DeviceContext* d3d11_immediate_context);

public: // ICommandBuffer override.
	void Reset() override;

	void ClearColorTarget(const IColorTarget* color_target, const float clear_color[4]) override;

	void SetColorTargets(uint32_t num_colot_targets, IColorTarget* const* color_targets) override;

	bool GetRawCommandBuffer(RawCommandBuffer* raw_command_buffer) const override;

public: // IDeviceObject override.
	void AddRef() noexcept override;

	void Release() noexcept override;

	uint32_t GetRefCount() const noexcept override;

	bool GetDevice(IDevice** device) const override;

private: // IDeviceObject override.
	void dispose() noexcept override;

private:
	CommandBufferDesc    m_command_buffer_desc;
	RawCommandBuffer     m_raw_command_buffer;
	IDevice*             m_device;
	std::atomic_uint32_t m_ref_count;
};

} // namespace D3D11
} // namespace RHI
