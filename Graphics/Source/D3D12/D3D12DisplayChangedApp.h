#pragma once

#include "D3D12App.h"
#include "Win32/Display.h"

class D3D12DisplayChangedApp : public D3D12App
{
public:
	D3D12DisplayChangedApp(LPCWSTR title, UINT width, UINT height);

	virtual ~D3D12DisplayChangedApp() override = default;

protected:
	virtual bool OnInitialize() override;

	virtual void OnFinalize() override;

	virtual void OnUpdate() override;

	virtual void OnRender() override;

	virtual LRESULT CALLBACK OnWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

private:
	void resize(UINT width, UINT height);

    void changeDisplayMode();

private:
	std::vector<DisplayMode> m_display_modes;
	uint32_t                 m_display_mode_index;
	bool                     m_is_fullscreen;

	ComPtr<ID3D12DescriptorHeap> m_imgui_heap;
};
