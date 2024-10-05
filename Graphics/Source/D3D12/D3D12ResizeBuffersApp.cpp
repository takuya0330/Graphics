#include "D3D12ResizeBuffersApp.h"

D3D12ResizeBuffersApp::D3D12ResizeBuffersApp(LPCWSTR title, UINT width, UINT height)
    : D3D12WaitableSwapChainApp(title, width, height)
{
}

LRESULT CALLBACK D3D12ResizeBuffersApp::OnWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_SIZE:
		{
			RECT rect = {};
			GetClientRect(hWnd, &rect);
			resize(rect.right - rect.left, rect.bottom - rect.top);
		}
		break;
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_F11:
			toggleFullscreen();
			break;
		}
		break;
	default:
		return Win32App::OnWindowProc(hWnd, uMsg, wParam, lParam);
	}
	return 0;
}

void D3D12ResizeBuffersApp::resize(UINT width, UINT height)
{
	if (!m_swap_chain4)
		return;

	waitForGPU(m_gfx_cmd_queue.Get());

	// 一旦解放
	for (auto& it : m_back_buffers)
	{
		it.Reset();
	}

	HRESULT hr = S_OK;

	// バックバッファの再作成
	{
		DXGI_SWAP_CHAIN_DESC1 swap_chain_desc1 = {};

		hr = m_swap_chain4->GetDesc1(&swap_chain_desc1);
		ASSERT_IF_FAILED(hr);

		hr = m_swap_chain4->ResizeBuffers(swap_chain_desc1.BufferCount, width, height, swap_chain_desc1.Format, swap_chain_desc1.Flags);
		ASSERT_IF_FAILED(hr);

		auto handle = m_rtv_heap->GetCPUDescriptorHandleForHeapStart();
		for (UINT i = 0; i < swap_chain_desc1.BufferCount; ++i)
		{
			auto& back_buffer = m_back_buffers.at(i);
			hr                = m_swap_chain4->GetBuffer(i, IID_PPV_ARGS(back_buffer.GetAddressOf()));
			ASSERT_IF_FAILED(hr);

			m_device->CreateRenderTargetView(back_buffer.Get(), nullptr, handle);
			handle.ptr += m_rtv_heap_size;
		}
	}

	// 深度バッファの再作成
	{
		m_depth_buffer.Reset();

		D3D12_CLEAR_VALUE d3d12_clear_value = {
			.Format       = DXGI_FORMAT_D32_FLOAT,
			.DepthStencil = {.Depth = 1.0f, .Stencil = 0}
		};

		if (!createTexture2D(
		        D3D12_HEAP_TYPE_DEFAULT,
		        width,
		        height,
		        DXGI_FORMAT_D32_FLOAT,
		        D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL,
		        D3D12_RESOURCE_STATE_DEPTH_WRITE,
		        &d3d12_clear_value,
		        m_depth_buffer.GetAddressOf()))
			ASSERT(false);

		D3D12_DEPTH_STENCIL_VIEW_DESC dsv_desc = {
			.Format        = DXGI_FORMAT_D32_FLOAT,
			.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D,
			.Flags         = D3D12_DSV_FLAG_NONE,
			.Texture2D     = { .MipSlice = 0 }
		};
		auto handle = m_dsv_heap->GetCPUDescriptorHandleForHeapStart();
		m_device->CreateDepthStencilView(m_depth_buffer.Get(), &dsv_desc, handle);
	}

	// 新しいウィンドウサイズを保存
	m_width  = width;
	m_height = height;
}

void D3D12ResizeBuffersApp::toggleFullscreen()
{
	BOOL fullscreen;
	if (FAILED(m_swap_chain4->GetFullscreenState(&fullscreen, nullptr)))
	{
		fullscreen = FALSE;
	}

	if (fullscreen)
	{
		m_swap_chain4->SetFullscreenState(FALSE, nullptr);
		SetWindowLong(m_hwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
		ShowWindow(m_hwnd, SW_NORMAL);
	}
	else
	{
		DXGI_MODE_DESC mode_desc = {};
        {
			mode_desc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
			mode_desc.Width                   = m_width;
			mode_desc.Height                  = m_height;
			mode_desc.RefreshRate.Numerator   = 60;
			mode_desc.RefreshRate.Denominator = 1;
			mode_desc.ScanlineOrdering        = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
			mode_desc.Scaling                 = DXGI_MODE_SCALING_UNSPECIFIED;
        }
		m_swap_chain4->ResizeTarget(&mode_desc);
		m_swap_chain4->SetFullscreenState(TRUE, nullptr);
	}
	resize(m_width, m_height);
}
