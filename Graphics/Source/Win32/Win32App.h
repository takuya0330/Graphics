#pragma once

#include "CoreApp.h"
#include "Debug.h"

class Win32App : public CoreApp
{
public:
	Win32App(LPCWSTR title, UINT width, UINT height);

	virtual ~Win32App() = default;

	int Run() final override;

protected:
	virtual bool OnInitialize();

	virtual void OnFinalize();

	virtual void OnUpdate();

	virtual void OnRender();

	virtual LRESULT CALLBACK OnWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
	HWND         m_hwnd;
	LPCWSTR      m_title;
	UINT         m_width;
	UINT         m_height;
	float        m_aspect_ratio;
	std::wstring m_out_dir;
	std::wstring m_asset_dir;
	std::wstring m_hlsl_dir;

private:
	friend LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};
