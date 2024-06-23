#pragma once

#include "DeviceObject.h"
#include "Platform.h"

namespace RHI {

//! \brief スワップチェイン作成情報
struct SwapChainDesc
{
	void*    hwnd;
	uint32_t width;
	uint32_t height;
	uint32_t num_buffers;
};

//! \brief プラットフォーム固有のスワップチェイン
struct RawSwapChain
{
#if _D3D11
	MSWRL::ComPtr<IDXGISwapChain4> dxgi_swap_chain4;
#elif _D3D12
	MSWRL::ComPtr<IDXGISwapChain4> dxgi_swap_chain4;
#else
#error Not supported.
#endif
};

//! \brief スワップチェイン基底クラス
class ISwapChain : public IDeviceObject
{
public:
	virtual ~ISwapChain() = default;

	virtual void Present() = 0;

    virtual IColorTarget* GetColorBackBuffer() const = 0;

	virtual bool GetRawSwapChain(RawSwapChain* raw_swap_chain) const = 0;
};

} // namespace RHI
