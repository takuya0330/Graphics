#pragma once

#include "D3D12App.h"

class D3D12WaitableSwapChainApp : public D3D12App
{
public:
	D3D12WaitableSwapChainApp(LPCWSTR title, UINT width, UINT height);

	virtual ~D3D12WaitableSwapChainApp() override = default;

protected:
	virtual bool OnInitialize() override;

	virtual void OnRender() override;

protected:
    void waitSwapChain();

protected:
	HANDLE m_frame_latency_waitable_object;
};
