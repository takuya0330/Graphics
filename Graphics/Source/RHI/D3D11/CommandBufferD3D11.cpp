#include "CommandBufferD3D11.h"

#include "ColorTargetD3D11.h"
#include "DeviceD3D11.h"
#include "Win32/Debug.h"

namespace RHI {
namespace D3D11 {

CCommandBuffer::CCommandBuffer(IDevice* device)
    : m_command_buffer_desc()
    , m_raw_command_buffer()
    , m_device(device)
    , m_ref_count(0)
{
}

bool CCommandBuffer::Initialize(const CommandBufferDesc* command_buffer_desc, ID3D11DeviceContext* d3d11_immediate_context)
{
	ASSERT_RETURN(command_buffer_desc, false);
	m_command_buffer_desc = (*command_buffer_desc);

	m_raw_command_buffer.d3d11_immediate_context = d3d11_immediate_context;

	return true;
}

void CCommandBuffer::Reset()
{
	m_raw_command_buffer.d3d11_immediate_context->ClearState();
}

void CCommandBuffer::ClearColorTarget(const IColorTarget* color_target, const float clear_color[4])
{
	ASSERT_RETURN(color_target);

	RawColorTarget raw_color_target;
	ASSERT_RETURN(color_target->GetRawColorTarget(&raw_color_target));

	m_raw_command_buffer.d3d11_immediate_context->ClearRenderTargetView(raw_color_target.d3d11_render_target_view.Get(), clear_color);
}

void CCommandBuffer::SetColorTargets(uint32_t num_colot_targets, IColorTarget* const* color_targets)
{
	std::vector<ID3D11RenderTargetView*> d3d11_rtv;
	d3d11_rtv.reserve(num_colot_targets);
	for (uint32_t i = 0; i < num_colot_targets; ++i)
	{
		auto color_target = color_targets[i];

		RawColorTarget raw_color_target;
		ASSERT_RETURN(color_target->GetRawColorTarget(&raw_color_target));

		d3d11_rtv.emplace_back(raw_color_target.d3d11_render_target_view.Get());
	}
	m_raw_command_buffer.d3d11_immediate_context->OMSetRenderTargets(static_cast<UINT>(d3d11_rtv.size()), d3d11_rtv.data(), nullptr);
}

bool CCommandBuffer::GetRawCommandBuffer(RawCommandBuffer* raw_command_buffer) const
{
	ASSERT_RETURN(m_raw_command_buffer.d3d11_immediate_context, false);

	(*raw_command_buffer) = m_raw_command_buffer;

	return true;
}

void CCommandBuffer::AddRef() noexcept
{
	m_ref_count.fetch_add(1);
}

void CCommandBuffer::Release() noexcept
{
	m_ref_count.fetch_sub(1);
	if (m_ref_count.load() == 0)
	{
		m_device->Release(this);
	}
}

uint32_t CCommandBuffer::GetRefCount() const noexcept
{
	return m_ref_count.load();
}

bool CCommandBuffer::GetDevice(IDevice** device) const
{
	ASSERT_RETURN(m_device, false);

	(*device) = m_device;

	return true;
}

void CCommandBuffer::dispose() noexcept
{
	delete this;
}

} // namespace D3D11
} // namespace RHI
