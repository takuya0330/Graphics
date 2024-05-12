#pragma once

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

#include <d3d12.h>
#include <dxgi.h>
#include <wrl.h>

#include <array>

#include "Win32App.h"

#define RETURN_HRESULT_IF_FAILED(hr)             \
	if (FAILED(hr))                              \
	{                                            \
		printf("ERROR: HRESULT = 0x%08X\n", hr); \
		return hr;                               \
	}

#define RETURN_FALSE_IF_FAILED(hr)               \
	if (FAILED(hr))                              \
	{                                            \
		printf("ERROR: HRESULT = 0x%08X\n", hr); \
		return false;                            \
	}

class D3D11App : public Win32App
{
public:
	static constexpr UINT kBackBufferCount = 2;

	template<class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	template<class T>
	using DoubleBufferArray = std::array<T, kBackBufferCount>;

	template<class T>
	using ComPtrs = DoubleBufferArray<ComPtr<T>>;

public:
	D3D11App(LPCWSTR title, UINT width, UINT height);

	virtual ~D3D11App() override = default;

protected:

private:
	bool OnInitialize() override;

	void OnFinalize() override;

	void OnUpdate() override;

	void OnRender() override;

protected:
};
