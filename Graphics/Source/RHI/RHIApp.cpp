#include "RHIApp.h"

RHIApp::RHIApp(LPCWSTR title, UINT width, UINT height)
    : Win32App(title, width, height)
    , m_rhi_device()
    , m_rhi_command_buffer()
    , m_rhi_swap_chain()
{
}

bool RHIApp::OnInitialize()
{
	ASSERT_RETURN(Win32App::OnInitialize(), false);

	RHI::DeviceDesc device_desc = {};
	{
	}
	auto ret = RHI::CreateDevice(&device_desc, &m_rhi_device);
	ASSERT_RETURN(ret, false);

	RHI::CommandBufferDesc command_buffer_desc = {};
	{
	}
	ret = m_rhi_device->CreateCommandBuffer(&command_buffer_desc, &m_rhi_command_buffer);
	ASSERT_RETURN(ret, false);

	RHI::SwapChainDesc swap_chain_desc = {};
	{
		swap_chain_desc.hwnd        = m_hwnd;
		swap_chain_desc.width       = m_width;
		swap_chain_desc.height      = m_height;
		swap_chain_desc.num_buffers = 2;
	}
	ret = m_rhi_device->CreateSwapChain(&swap_chain_desc, &m_rhi_swap_chain);
	ASSERT_RETURN(ret, false);

	return true;
}

void RHIApp::OnFinalize()
{
}

void RHIApp::OnUpdate()
{
}

void RHIApp::OnRender()
{
	m_rhi_command_buffer->Reset();

	RHI::IColorTarget* color_targets[] = { m_rhi_swap_chain->GetColorBackBuffer() };
	m_rhi_command_buffer->SetColorTargets(1, color_targets);

	constexpr float clear_color[] = { 0.2f, 0.2f, 0.2f, 1.0f };
	m_rhi_command_buffer->ClearColorTarget(color_targets[0], clear_color);

	m_rhi_swap_chain->Present();
}
