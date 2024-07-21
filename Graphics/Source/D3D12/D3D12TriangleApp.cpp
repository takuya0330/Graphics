#include "D3D12TriangleApp.h"

namespace {

struct Vertex
{
	float position[3];
	float color[4];
};

} // namespace

D3D12TriangleApp::D3D12TriangleApp(LPCWSTR title, UINT width, UINT height)
    : D3D12App(title, width, height)
    , m_vertex_buffer()
    , m_root_signature()
    , m_pipeline_state()
{
}

bool D3D12TriangleApp::OnInitialize()
{
	ASSERT_RETURN(D3D12App::OnInitialize(), false);

	const Vertex vertices[] = {
		{{ +0, +1, +0 },  { 1, 0, 0, 1 }},
		{ { +1, -1, +0 }, { 0, 1, 0, 1 }},
		{ { -1, -1, +0 }, { 0, 0, 1, 1 }},
	};

	ComPtr<ID3D12Resource> upload_buffer;
	if (!createBuffer(D3D12_HEAP_TYPE_UPLOAD, sizeof(vertices), D3D12_RESOURCE_STATE_GENERIC_READ, upload_buffer.GetAddressOf()))
		return false;

	{
		void* data = nullptr;
		ASSERT_IF_FAILED(upload_buffer->Map(0, nullptr, &data));
		std::memcpy(data, vertices, sizeof(vertices));
		upload_buffer->Unmap(0, nullptr);
	}

	if (!createBuffer(D3D12_HEAP_TYPE_DEFAULT, sizeof(vertices), D3D12_RESOURCE_STATE_COMMON, m_vertex_buffer.GetAddressOf()))
		return false;

	{
		ComPtr<ID3D12CommandQueue> copy_cmd_queue;
		if (!createCommandQueue(D3D12_COMMAND_LIST_TYPE_COPY, copy_cmd_queue.GetAddressOf()))
			return false;

		ComPtr<ID3D12CommandAllocator> copy_cmd_allocator;
		if (!createCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY, copy_cmd_allocator.GetAddressOf()))
			return false;

		ComPtr<ID3D12GraphicsCommandList> copy_cmd_list;
		if (!createCommandList(D3D12_COMMAND_LIST_TYPE_COPY, copy_cmd_allocator.Get(), copy_cmd_list.GetAddressOf()))
			return false;

		copy_cmd_allocator->Reset();
		copy_cmd_list->Reset(copy_cmd_allocator.Get(), nullptr);

		copy_cmd_list->CopyResource(m_vertex_buffer.Get(), upload_buffer.Get());
		copy_cmd_list->Close();

		ID3D12CommandList* cmd_lists[] = { copy_cmd_list.Get() };
		copy_cmd_queue->ExecuteCommandLists(1, cmd_lists);

		waitForGPU(copy_cmd_queue.Get());
	}

	ComPtr<IDxcBlob> vs;
	if (!loadShader(L"PositionColor_vs.hlsl", L"main", L"vs_6_0", vs.GetAddressOf()))
		return false;

	ComPtr<IDxcBlob> ps;
	if (!loadShader(L"PositionColor_ps.hlsl", L"main", L"ps_6_0", ps.GetAddressOf()))
		return false;

	HRESULT hr = S_OK;

	D3D12_INPUT_ELEMENT_DESC input_layout[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{ "COLOR",   0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	};

	{
		D3D12_ROOT_SIGNATURE_DESC root_sig_desc = {};
		{
			root_sig_desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
		}

		ComPtr<ID3DBlob> root_sig_blob;
		ComPtr<ID3DBlob> err;

		hr = D3D12SerializeRootSignature(&root_sig_desc, D3D_ROOT_SIGNATURE_VERSION_1_0, root_sig_blob.GetAddressOf(), err.GetAddressOf());
		RETURN_FALSE_IF_FAILED(hr);

		hr = m_device->CreateRootSignature(0, root_sig_blob->GetBufferPointer(), root_sig_blob->GetBufferSize(), IID_PPV_ARGS(m_root_signature.GetAddressOf()));
		RETURN_FALSE_IF_FAILED(hr);
	}

	D3D12_GRAPHICS_PIPELINE_STATE_DESC gfx_pipeline_state_desc = {};
	{
		gfx_pipeline_state_desc.pRootSignature                                   = m_root_signature.Get();
		gfx_pipeline_state_desc.VS.pShaderBytecode                               = vs->GetBufferPointer();
		gfx_pipeline_state_desc.VS.BytecodeLength                                = vs->GetBufferSize();
		gfx_pipeline_state_desc.PS.pShaderBytecode                               = ps->GetBufferPointer();
		gfx_pipeline_state_desc.PS.BytecodeLength                                = ps->GetBufferSize();
		gfx_pipeline_state_desc.SampleMask                                       = D3D12_DEFAULT_SAMPLE_MASK;
		gfx_pipeline_state_desc.BlendState.AlphaToCoverageEnable                 = false;
		gfx_pipeline_state_desc.BlendState.IndependentBlendEnable                = false;
		gfx_pipeline_state_desc.BlendState.RenderTarget[0].BlendEnable           = false;
		gfx_pipeline_state_desc.BlendState.RenderTarget[0].LogicOpEnable         = false;
		gfx_pipeline_state_desc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
		gfx_pipeline_state_desc.RasterizerState.MultisampleEnable                = false;
		gfx_pipeline_state_desc.RasterizerState.CullMode                         = D3D12_CULL_MODE_NONE;
		gfx_pipeline_state_desc.RasterizerState.FillMode                         = D3D12_FILL_MODE_SOLID;
		gfx_pipeline_state_desc.RasterizerState.DepthClipEnable                  = true;
		gfx_pipeline_state_desc.RasterizerState.FrontCounterClockwise            = false;
		gfx_pipeline_state_desc.RasterizerState.DepthBias                        = D3D12_DEFAULT_DEPTH_BIAS;
		gfx_pipeline_state_desc.RasterizerState.DepthBiasClamp                   = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
		gfx_pipeline_state_desc.RasterizerState.SlopeScaledDepthBias             = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		gfx_pipeline_state_desc.RasterizerState.AntialiasedLineEnable            = false;
		gfx_pipeline_state_desc.RasterizerState.ForcedSampleCount                = 0;
		gfx_pipeline_state_desc.RasterizerState.ConservativeRaster               = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
		gfx_pipeline_state_desc.DepthStencilState.DepthEnable                    = false;
		gfx_pipeline_state_desc.DepthStencilState.StencilEnable                  = false;
		gfx_pipeline_state_desc.InputLayout.pInputElementDescs                   = input_layout;
		gfx_pipeline_state_desc.InputLayout.NumElements                          = _countof(input_layout);
		gfx_pipeline_state_desc.IBStripCutValue                                  = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
		gfx_pipeline_state_desc.PrimitiveTopologyType                            = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		gfx_pipeline_state_desc.NumRenderTargets                                 = 1;
		gfx_pipeline_state_desc.RTVFormats[0]                                    = DXGI_FORMAT_R8G8B8A8_UNORM;
		gfx_pipeline_state_desc.SampleDesc.Count                                 = 1;
		gfx_pipeline_state_desc.SampleDesc.Quality                               = 0;
	}
	hr = m_device->CreateGraphicsPipelineState(&gfx_pipeline_state_desc, IID_PPV_ARGS(m_pipeline_state.GetAddressOf()));
	RETURN_FALSE_IF_FAILED(hr);

	return true;
}

void D3D12TriangleApp::OnFinalize()
{
	D3D12App::OnFinalize();
}

void D3D12TriangleApp::OnUpdate()
{
	D3D12App::OnUpdate();
}

void D3D12TriangleApp::OnRender()
{
	reset();
	setViewport(static_cast<float>(m_width), static_cast<float>(m_height));
	setScissorRect(static_cast<LONG>(m_width), static_cast<LONG>(m_height));
	setBackBuffer();

	{
		m_gfx_cmd_list->SetGraphicsRootSignature(m_root_signature.Get());
		m_gfx_cmd_list->SetPipelineState(m_pipeline_state.Get());

		D3D12_VERTEX_BUFFER_VIEW view = {};
		{
			view.BufferLocation = m_vertex_buffer->GetGPUVirtualAddress();
			view.SizeInBytes    = sizeof(Vertex) * 3;
			view.StrideInBytes  = sizeof(Vertex);
		}
		m_gfx_cmd_list->IASetVertexBuffers(0, 1, &view);
		m_gfx_cmd_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_gfx_cmd_list->DrawInstanced(3, 1, 0, 0);
	}

	executeCommandList();
	present(1);
	waitPreviousFrame();
}
