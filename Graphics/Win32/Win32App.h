#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <Windows.h>

#include <cassert>
#include <iostream>

#define ASSERT_RETURN(expr, ...) \
	if (!(expr))                 \
	{                            \
		assert(expr);            \
		return __VA_ARGS__;      \
	}

class Win32App
{
public:
	Win32App(LPCWSTR title, UINT width, UINT height);

	virtual ~Win32App() = default;

	int Run();

protected:
	virtual bool OnInitialize();

	virtual void OnFinalize();

	virtual void OnUpdate();

	virtual void OnRender();

	LRESULT CALLBACK OnWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
	HWND    m_hwnd;
	LPCWSTR m_title;
	UINT    m_width;
	UINT    m_height;

private:
	friend LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};
