#pragma once

#include "D3D12App.h"

#if APP_WIN32 && APP_D3D12 && APP_IMGUI

class D3D12ImGuiApp : public D3D12App
{
public:
	D3D12ImGuiApp(LPCWSTR title, UINT width, UINT height);

	virtual ~D3D12ImGuiApp() override = default;

protected:
	virtual bool OnInitialize() override;

	virtual void OnFinalize() override;

	virtual void OnUpdate() override;

	virtual void OnRender() override;

	virtual LRESULT CALLBACK OnWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

protected:
	void renderImGui();

private:
	ComPtr<ID3D12DescriptorHeap> m_imgui_heap;
#if ENABLE_IMGUI_DEMO_WINDOW
	bool m_enable_demo_window;
#endif
};

#endif
