#include "D3D12WaitableSwapChainApp.h"

D3D12WaitableSwapChainApp::D3D12WaitableSwapChainApp(LPCWSTR title, UINT width, UINT height)
    : D3D12App(title, width, height)
    , m_frame_latency_waitable_object()
{
}

bool D3D12WaitableSwapChainApp::OnInitialize()
{
	m_num_back_buffers = 3;

	UINT dxgi_flags = 0;
	ASSERT_RETURN(enableDebugLayer(dxgi_flags), false);
	ASSERT_RETURN(createFactory(dxgi_flags), false);
	ASSERT_RETURN(searchAdapter(), false);
	ASSERT_RETURN(createDevice(), false);
	ASSERT_RETURN(createCommand(), false);

    {
		HRESULT hr = S_OK;

		m_back_buffers.resize(m_num_back_buffers);

		DXGI_SWAP_CHAIN_DESC1 swap_chain_desc1 = {
			.Width       = m_width,
			.Height      = m_height,
			.Format      = DXGI_FORMAT_R8G8B8A8_UNORM,
			.Stereo      = false,
			.SampleDesc  = {.Count = 1, .Quality = 0},
			.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
			.BufferCount = m_num_back_buffers,
			.Scaling     = DXGI_SCALING_NONE,
			.SwapEffect  = DXGI_SWAP_EFFECT_FLIP_DISCARD,
			.AlphaMode   = DXGI_ALPHA_MODE_UNSPECIFIED,
			.Flags       = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT
		};
		ComPtr<IDXGISwapChain1> dxgi_swap_chain1;
		hr = m_factory->CreateSwapChainForHwnd(m_gfx_cmd_queue.Get(), m_hwnd, &swap_chain_desc1, nullptr, nullptr, dxgi_swap_chain1.GetAddressOf());
		RETURN_FALSE_IF_FAILED(hr);

		hr = m_factory->MakeWindowAssociation(m_hwnd, DXGI_MWA_NO_ALT_ENTER);
		RETURN_FALSE_IF_FAILED(hr);

		hr = dxgi_swap_chain1.As(&m_swap_chain4);
		RETURN_FALSE_IF_FAILED(hr);

        m_swap_chain4->SetMaximumFrameLatency(2);
		m_frame_latency_waitable_object = m_swap_chain4->GetFrameLatencyWaitableObject();

		m_back_buffer_index = m_swap_chain4->GetCurrentBackBufferIndex();
    }

	ASSERT_RETURN(createBackBuffer(), false);
	ASSERT_RETURN(createFence(), false);

	return true;
}

void D3D12WaitableSwapChainApp::OnRender()
{
	waitSwapChain();
	waitPreviousFrame();
	reset();
	setViewport(static_cast<float>(m_width), static_cast<float>(m_height));
	setScissorRect(static_cast<LONG>(m_width), static_cast<LONG>(m_height));
	setBackBuffer();
	executeCommandList();
	present(1);
}

void D3D12WaitableSwapChainApp::waitSwapChain()
{
	WaitForSingleObjectEx(m_frame_latency_waitable_object, 1000, true);
}
