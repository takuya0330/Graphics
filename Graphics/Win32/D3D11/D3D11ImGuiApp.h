#pragma once

#include "D3D11App.h"

#if APP_WIN32 && APP_D3D11 && APP_IMGUI

class D3D11ImGuiApp : public D3D11App
{
public:
	D3D11ImGuiApp(LPCWSTR title, UINT width, UINT height);

	virtual ~D3D11ImGuiApp() override = default;

protected:
	virtual bool OnInitialize() override;

	virtual void OnFinalize() override;

	virtual void OnUpdate() override;

	virtual void OnRender() override;

	virtual LRESULT CALLBACK OnWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

private:
#if ENABLE_IMGUI_DEMO_WINDOW
	bool m_enable_demo_window;
#endif
};

#endif
