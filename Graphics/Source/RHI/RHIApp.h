#pragma once

#include "CommandBuffer.h"
#include "Device.h"
#include "ScopedPtr.h"
#include "SwapChain.h"
#include "Win32/Win32App.h"

class RHIApp : public Win32App
{
public:
	RHIApp(LPCWSTR title, UINT width, UINT height);

	virtual ~RHIApp() override = default;

protected:
	virtual bool OnInitialize() override;

	virtual void OnFinalize() override;

	virtual void OnUpdate() override;

	virtual void OnRender() override;

private:
	//RHI::ScopedPtr<RHI::IDevice>        m_rhi_device;
	//RHI::ScopedPtr<RHI::ICommandBuffer> m_rhi_command_buffer;
	//RHI::ScopedPtr<RHI::ISwapChain>     m_rhi_swap_chain;

	RHI::IDevice*        m_rhi_device;
	RHI::ICommandBuffer* m_rhi_command_buffer;
	RHI::ISwapChain*     m_rhi_swap_chain;
};
