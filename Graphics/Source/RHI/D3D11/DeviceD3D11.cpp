#include "DeviceD3D11.h"

#include "RHI/D3D11/CommandBufferD3D11.h"
//#include "RHI/D3D11/RenderTargetD3D11.h"
#include "RHI/D3D11/SwapChainD3D11.h"
#include "RHI/DeviceObject.h"
#include "Win32/Debug.h"

namespace RHI {

bool CreateDevice(const DeviceDesc* device_desc, IDevice** device)
{
	D3D11::CDevice* d3d11_device = new D3D11::CDevice;
	ASSERT_RETURN(d3d11_device, false);
	ASSERT_RETURN(d3d11_device->Initialize(device_desc), false);

	(*device) = d3d11_device;

	return true;
}

namespace D3D11 {

CDevice::CDevice()
    : m_device_desc()
    , m_raw_device()
    , m_d3d11_immediate_context()
{
}

bool CDevice::Initialize(const DeviceDesc* device_desc)
{
	ASSERT_RETURN(device_desc, false);
	m_device_desc = (*device_desc);

	HRESULT hr = S_OK;

	MSWRL::ComPtr<IDXGIFactory2> dxgi_factory2;
	{
		hr = ::CreateDXGIFactory(IID_PPV_ARGS(dxgi_factory2.GetAddressOf()));
		RETURN_FALSE_IF_FAILED(hr);

		hr = dxgi_factory2.As(&m_dxgi_factory6);
		RETURN_FALSE_IF_FAILED(hr);
	}

	MSWRL::ComPtr<IDXGIAdapter> dxgi_adapter;
	{
		std::vector<MSWRL::ComPtr<IDXGIAdapter>> dxgi_adapters;
		{
			IDXGIAdapter* dxgi_adapter;
			for (UINT i = 0; dxgi_factory2->EnumAdapters(i, &dxgi_adapter) != DXGI_ERROR_NOT_FOUND; ++i)
			{
				dxgi_adapters.emplace_back(dxgi_adapter);
			}
		}
		ASSERT_RETURN(!dxgi_adapters.empty(), false);
		dxgi_adapter = dxgi_adapters.at(0); // TODO
	}

	UINT create_device_flags = 0;
#if defined(_DEBUG)
	create_device_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	constexpr D3D_DRIVER_TYPE kDriverTypes[] = {
		D3D_DRIVER_TYPE_UNKNOWN,
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};

	constexpr D3D_FEATURE_LEVEL kFeatureLevels[] = {
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1,
	};

	D3D_FEATURE_LEVEL current_feature_level;
	for (const auto& driver_type : kDriverTypes)
	{
		hr = D3D11CreateDevice(
		    dxgi_adapter.Get(),
		    driver_type,
		    nullptr,
		    create_device_flags,
		    kFeatureLevels,
		    _countof(kFeatureLevels),
		    D3D11_SDK_VERSION,
		    m_raw_device.d3d11_device.GetAddressOf(),
		    &current_feature_level,
		    m_d3d11_immediate_context.GetAddressOf());
		if (SUCCEEDED(hr))
			break;
		RETURN_FALSE_IF_FAILED(hr);
	}

	return true;
}

void CDevice::Release(IDeviceObject* object)
{
}

bool CDevice::CreateCommandBuffer(const CommandBufferDesc* command_buffer_desc, ICommandBuffer** command_buffer)
{
	ASSERT_RETURN(command_buffer_desc, false);

	D3D11::CCommandBuffer* d3d11_command_buffer = new D3D11::CCommandBuffer(this);
	ASSERT_RETURN(d3d11_command_buffer, false);
	ASSERT_RETURN(d3d11_command_buffer->Initialize(command_buffer_desc, m_d3d11_immediate_context.Get()), false);

	(*command_buffer) = d3d11_command_buffer;
	(*command_buffer)->AddRef();

	return true;
}

bool CDevice::CreateSwapChain(const SwapChainDesc* swap_chain_desc, ISwapChain** swap_chain)
{
	ASSERT_RETURN(swap_chain_desc, false);

	D3D11::CSwapChain* d3d11_swap_chain = new D3D11::CSwapChain(this);
	ASSERT_RETURN(d3d11_swap_chain, false);
	ASSERT_RETURN(d3d11_swap_chain->Initialize(swap_chain_desc, m_dxgi_factory6.Get()), false);

	(*swap_chain) = d3d11_swap_chain;
	(*swap_chain)->AddRef();

	return true;
}

//bool CDevice::CreateRenderTarget(const RenderTargetDesc* render_target_desc, IRenderTarget** render_target)
//{
//	ASSERT_RETURN(render_target_desc, false);
//
//    D3D11::CRenderTarget* d3d11_render_target = new D3D11::CRenderTarget(this);
//	ASSERT_RETURN(d3d11_render_target, false);
//	ASSERT_RETURN(d3d11_render_target->Initialize(render_target_desc), false);
//
//    (*render_target) = d3d11_render_target;
//	(*render_target)->AddRef();
//
//    return true;
//}

bool CDevice::GetRawDevice(RawDevice* raw_device) const
{
	ASSERT_RETURN(m_raw_device.d3d11_device, false);

	(*raw_device) = m_raw_device;

	return true;
}

} // namespace D3D11
} // namespace RHI
