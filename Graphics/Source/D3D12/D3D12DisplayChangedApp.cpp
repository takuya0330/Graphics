#include "D3D12DisplayChangedApp.h"

#include "Win32/String.h"

// clang-format off
#include "External/ImGui/imgui.h"
#include "External/ImGui/backends/imgui_impl_win32.h"
#include "External/ImGui/backends/imgui_impl_dx12.h"
// clang-format on

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

D3D12DisplayChangedApp::D3D12DisplayChangedApp(LPCWSTR title, UINT width, UINT height)
    : D3D12App(title, width, height)
    , m_display_modes()
    , m_display_mode_index(0)
    , m_is_fullscreen(true)
    , m_imgui_heap()
{
}

bool D3D12DisplayChangedApp::OnInitialize()
{
	UINT dxgi_flags = 0;
	ASSERT_RETURN(enableDebugLayer(dxgi_flags), false);
	ASSERT_RETURN(createFactory(dxgi_flags), false);
	ASSERT_RETURN(searchAdapter(), false);
	ASSERT_RETURN(createDevice(), false);
	ASSERT_RETURN(createCommand(), false);

	{
		MSWRL::ComPtr<IDXGIOutput> output;

		auto hr = m_adapters.at(m_adapter_index)->EnumOutputs(0, output.ReleaseAndGetAddressOf());
		ASSERT_IF_FAILED(hr);

		UINT num_modes = 0;
		output->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &num_modes, nullptr);

		std::vector<DXGI_MODE_DESC> mode_descs(num_modes);
		output->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &num_modes, mode_descs.data());

		// TODO: m_hwnd を使ったディスプレイ情報の取得
		DEVMODE devmode = {};
		EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devmode);

		float min_diff = FLT_MAX;

		m_display_modes.reserve(num_modes);
		for (uint32_t i = 0; i < num_modes; ++i)
		{
			const auto& mode = mode_descs[i];

			DisplayMode dm = {
				.width        = mode.Width,
				.height       = mode.Height,
				.refresh_rate = {mode.RefreshRate.Numerator, mode.RefreshRate.Denominator},
			};
			m_display_modes.emplace_back(dm);

			if (!(dm.width == devmode.dmPelsWidth && dm.height == devmode.dmPelsHeight))
				continue;

			float dst_hz = static_cast<float>(dm.refresh_rate.numerator) / dm.refresh_rate.denominator;
			float src_hz = static_cast<float>(devmode.dmDisplayFrequency);
			float diff   = src_hz - dst_hz;
			if (diff < min_diff)
			{
				min_diff             = diff;
				m_display_mode_index = i;
			}
		}

		m_back_buffers.resize(m_num_back_buffers);

		DXGI_SWAP_CHAIN_DESC1 swap_chain_desc1 = {
			.Width       = mode_descs.at(m_display_mode_index).Width,
			.Height      = mode_descs.at(m_display_mode_index).Height,
			.Format      = DXGI_FORMAT_R8G8B8A8_UNORM,
			.Stereo      = false,
			.SampleDesc  = {.Count = 1, .Quality = 0},
			.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
			.BufferCount = m_num_back_buffers,
			.Scaling     = DXGI_SCALING_NONE,
			.SwapEffect  = DXGI_SWAP_EFFECT_FLIP_DISCARD,
			.AlphaMode   = DXGI_ALPHA_MODE_UNSPECIFIED,
			.Flags       = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
		};

		DXGI_SWAP_CHAIN_FULLSCREEN_DESC swap_chain_fullscreen_desc = {
			.RefreshRate      = mode_descs.at(m_display_mode_index).RefreshRate,
			.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED,
			.Scaling          = DXGI_MODE_SCALING_UNSPECIFIED,
			.Windowed         = !m_is_fullscreen
		};

		ComPtr<IDXGISwapChain1> dxgi_swap_chain1;
		hr = m_factory->CreateSwapChainForHwnd(m_gfx_cmd_queue.Get(), m_hwnd, &swap_chain_desc1, &swap_chain_fullscreen_desc, nullptr, dxgi_swap_chain1.GetAddressOf());
		RETURN_FALSE_IF_FAILED(hr);

		hr = m_factory->MakeWindowAssociation(m_hwnd, DXGI_MWA_NO_ALT_ENTER);
		RETURN_FALSE_IF_FAILED(hr);

		hr = dxgi_swap_chain1.As(&m_swap_chain4);
		RETURN_FALSE_IF_FAILED(hr);

		m_back_buffer_index = m_swap_chain4->GetCurrentBackBufferIndex();
	}

	ASSERT_RETURN(createBackBuffer(), false);
	ASSERT_RETURN(createFence(), false);

	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ASSERT_RETURN(ImGui_ImplWin32_Init(m_hwnd), false);

		D3D12_DESCRIPTOR_HEAP_DESC imgui_heap_desc = {
			.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
			.NumDescriptors = 1,
			.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
			.NodeMask       = 0
		};
		auto hr = m_device->CreateDescriptorHeap(&imgui_heap_desc, IID_PPV_ARGS(m_imgui_heap.GetAddressOf()));
		RETURN_FALSE_IF_FAILED(hr);

		auto imgui_cpu_handle = m_imgui_heap->GetCPUDescriptorHandleForHeapStart();
		auto imgui_gpu_handle = m_imgui_heap->GetGPUDescriptorHandleForHeapStart();
		ASSERT_RETURN(ImGui_ImplDX12_Init(m_device.Get(), m_num_back_buffers, DXGI_FORMAT_R8G8B8A8_UNORM, m_imgui_heap.Get(), imgui_cpu_handle, imgui_gpu_handle), false);
	}

	return true;
}

void D3D12DisplayChangedApp::OnFinalize()
{
	D3D12App::OnFinalize();

	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void D3D12DisplayChangedApp::OnUpdate()
{
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	if (ImGui::Begin("DisplayMode"))
	{
		std::vector<std::string> previews(m_display_modes.size());
		for (size_t i = 0; i < m_display_modes.size(); ++i)
		{
			const auto& dm = m_display_modes.at(i);
			previews.at(i) += std::to_string(dm.width);
			previews.at(i) += "x";
			previews.at(i) += std::to_string(dm.height) + " ";
			previews.at(i) += std::to_string(static_cast<float>(dm.refresh_rate.numerator) / dm.refresh_rate.denominator) + " Hz";
		}

		if (ImGui::BeginCombo("Current", previews.at(m_display_mode_index).c_str()))
		{
			for (size_t i = 0; i < m_display_modes.size(); ++i)
			{
				const bool is_selected = (m_display_mode_index == i);
				if (ImGui::Selectable(previews.at(i).c_str(), is_selected))
					m_display_mode_index = i;

				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

        static bool check = m_is_fullscreen;
		if (ImGui::Checkbox("Fullscreen", &check))
		{
			m_is_fullscreen = false;
		}

		if (ImGui::Button("Apply"))
		{
			changeDisplayMode();
		}
	}
	ImGui::End();
}

void D3D12DisplayChangedApp::OnRender()
{
	reset();
	setViewport(static_cast<float>(m_width), static_cast<float>(m_height));
	setScissorRect(static_cast<LONG>(m_width), static_cast<LONG>(m_height));
	setBackBuffer();

	{
		ImGui::Render();

		m_gfx_cmd_list->SetDescriptorHeaps(1, m_imgui_heap.GetAddressOf());
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_gfx_cmd_list.Get());
	}

	executeCommandList();
	present(1);
	waitPreviousFrame();
}

LRESULT CALLBACK D3D12DisplayChangedApp::OnWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
		return true;

    // TODO: ウィンドウのフォーカスが外れたとき

	return D3D12App::OnWindowProc(hWnd, uMsg, wParam, lParam);
}

void D3D12DisplayChangedApp::resize(UINT width, UINT height)
{
	if (!m_swap_chain4)
		return;

	waitForGPU(m_gfx_cmd_queue.Get());

    // 解放
	for (auto& it : m_back_buffers)
	{
		it.Reset();
	}
	m_depth_buffer.Reset();

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

    // 現在のバックバッファインデックスを更新
    m_back_buffer_index = m_swap_chain4->GetCurrentBackBufferIndex();
}

void D3D12DisplayChangedApp::changeDisplayMode()
{
    // TODO: ウィンドウサイズの変更

	BOOL fullscreen;
	if (FAILED(m_swap_chain4->GetFullscreenState(&fullscreen, nullptr)))
	{
		fullscreen      = FALSE;
		m_is_fullscreen = false;
	}

	if (!m_is_fullscreen)
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
			mode_desc.Width                   = m_display_modes.at(m_display_mode_index).width;
			mode_desc.Height                  = m_display_modes.at(m_display_mode_index).height;
			mode_desc.RefreshRate.Numerator   = m_display_modes.at(m_display_mode_index).refresh_rate.numerator;
			mode_desc.RefreshRate.Denominator = m_display_modes.at(m_display_mode_index).refresh_rate.denominator;
			mode_desc.ScanlineOrdering        = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
			mode_desc.Scaling                 = DXGI_MODE_SCALING_UNSPECIFIED;
		}
		m_swap_chain4->ResizeTarget(&mode_desc);
		m_swap_chain4->SetFullscreenState(TRUE, nullptr);
	}
	resize(m_display_modes.at(m_display_mode_index).width, m_display_modes.at(m_display_mode_index).height);
}
