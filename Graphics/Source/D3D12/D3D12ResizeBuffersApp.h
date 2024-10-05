#pragma once

#include "D3D12WaitableSwapChainApp.h"

class D3D12ResizeBuffersApp : public D3D12WaitableSwapChainApp
{
public:
	D3D12ResizeBuffersApp(LPCWSTR title, UINT width, UINT height);

	virtual ~D3D12ResizeBuffersApp() override = default;

protected:
	LRESULT CALLBACK OnWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

private:
	void resize(UINT width, UINT height);

    void toggleFullscreen();
};
