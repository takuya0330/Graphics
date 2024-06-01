#pragma once

#include "D3D12App.h"

class D3D12ImGui : public D3D12App
{
public:
	D3D12ImGui(LPCWSTR title, UINT width, UINT height);

	virtual ~D3D12ImGui() override = default;

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
