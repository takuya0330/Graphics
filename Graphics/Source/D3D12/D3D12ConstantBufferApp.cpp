#include "D3D12ConstantBufferApp.h"

#include <DirectXMath.h>

namespace {

struct ConstantBufferData
{
	DirectX::XMMATRIX world;
	DirectX::XMMATRIX view;
	DirectX::XMMATRIX proj;
};

struct Vertex
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT4 color;

	Vertex(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT4& color)
	    : position(position)
	    , color(color)
	{
	}
};

} // namespace

D3D12ConstantBufferApp::D3D12ConstantBufferApp(LPCWSTR title, UINT width, UINT height)
    : D3D12App(title, width, height)
    , m_cbv_heap()
    , m_constant_buffer()
    , m_vertex_buffer()
    , m_root_signature()
    , m_pipeline_state()
{
}

bool D3D12ConstantBufferApp::OnInitialize()
{
	ASSERT_RETURN(D3D12App::OnInitialize(), false);

	const Vertex vertices[] = {
		Vertex(DirectX::XMFLOAT3(-0.5f, +0.5f, 0), DirectX::XMFLOAT4(1, 0, 0, 1)),
		Vertex(DirectX::XMFLOAT3(+0.5f, +0.5f, 0), DirectX::XMFLOAT4(0, 1, 0, 1)),
		Vertex(DirectX::XMFLOAT3(-0.5f, -0.5f, 0), DirectX::XMFLOAT4(0, 0, 1, 1)),
		Vertex(DirectX::XMFLOAT3(+0.5f, -0.5f, 0), DirectX::XMFLOAT4(1, 1, 1, 1)),
	};

	if (!createBuffer(
	        D3D12_HEAP_TYPE_UPLOAD,
	        sizeof(vertices),
	        D3D12_RESOURCE_STATE_GENERIC_READ,
	        m_vertex_buffer.GetAddressOf()))
		return false;
	{
		void* ptr = nullptr;
		ASSERT_IF_FAILED(m_vertex_buffer->Map(0, nullptr, &ptr));
		std::memcpy(ptr, vertices, sizeof(vertices));
		m_vertex_buffer->Unmap(0, nullptr);
	}

	if (!createBuffer(
	        D3D12_HEAP_TYPE_UPLOAD,
	        calcAlignment<UINT64>(sizeof(ConstantBufferData), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT),
	        D3D12_RESOURCE_STATE_GENERIC_READ,
	        m_constant_buffer.GetAddressOf()))
		return false;

	if (!createDescriptorHeap(
	        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
	        1,
	        D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
	        m_cbv_heap.GetAddressOf()))
		return false;

    D3D12_CONSTANT_BUFFER_VIEW_DESC cbv_desc = {};
    {
		cbv_desc.BufferLocation = m_constant_buffer->GetGPUVirtualAddress();
		cbv_desc.SizeInBytes    = calcAlignment<UINT64>(sizeof(ConstantBufferData), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
    }
	m_device->CreateConstantBufferView(&cbv_desc, m_cbv_heap->GetCPUDescriptorHandleForHeapStart());

	ComPtr<IDxcBlob> vs;
	if (!loadShader(L"PositionColor1_vs.hlsl", L"main", L"vs_6_0", vs.GetAddressOf()))
		return false;

	ComPtr<IDxcBlob> ps;
	if (!loadShader(L"PositionColor_ps.hlsl", L"main", L"ps_6_0", ps.GetAddressOf()))
		return false;

	HRESULT hr = S_OK;

	D3D12_INPUT_ELEMENT_DESC input_layout[2] = {};
	{
		input_layout[0] = {
			.SemanticName         = "POSITION",
			.SemanticIndex        = 0,
			.Format               = DXGI_FORMAT_R32G32B32_FLOAT,
			.InputSlot            = 0,
			.AlignedByteOffset    = D3D12_APPEND_ALIGNED_ELEMENT,
			.InputSlotClass       = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			.InstanceDataStepRate = 0
		};
		input_layout[1] = {
			.SemanticName         = "COLOR",
			.SemanticIndex        = 0,
			.Format               = DXGI_FORMAT_R32G32B32A32_FLOAT,
			.InputSlot            = 0,
			.AlignedByteOffset    = D3D12_APPEND_ALIGNED_ELEMENT,
			.InputSlotClass       = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			.InstanceDataStepRate = 0
		};
	}

	{
		D3D12_DESCRIPTOR_RANGE cbv_desc_range = {};
		{
			cbv_desc_range.RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
			cbv_desc_range.NumDescriptors                    = 1;
			cbv_desc_range.BaseShaderRegister                = 0;
			cbv_desc_range.RegisterSpace                     = 0;
			cbv_desc_range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
		}

        D3D12_ROOT_PARAMETER root_param = {};
		{
			root_param.ParameterType                       = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			root_param.DescriptorTable.NumDescriptorRanges = 1;
			root_param.DescriptorTable.pDescriptorRanges   = &cbv_desc_range;
			root_param.ShaderVisibility                    = D3D12_SHADER_VISIBILITY_VERTEX;
		}

		D3D12_ROOT_SIGNATURE_DESC root_sig_desc = {};
		{
			root_sig_desc.NumParameters = 1;
			root_sig_desc.pParameters   = &root_param;
			root_sig_desc.Flags         = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
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

void D3D12ConstantBufferApp::OnRender()
{
	reset();
	setViewport(static_cast<float>(m_width), static_cast<float>(m_height));
	setScissorRect(static_cast<LONG>(m_width), static_cast<LONG>(m_height));
	setBackBuffer();

	{
		m_gfx_cmd_list->SetGraphicsRootSignature(m_root_signature.Get());
		m_gfx_cmd_list->SetPipelineState(m_pipeline_state.Get());

		{
			static float angle = 0.0f;
			angle += 0.05f;

			ConstantBufferData cb_data = {};
			{
				const auto scale     = DirectX::XMMatrixScaling(10, 10, 1);
				const auto rotate    = DirectX::XMMatrixRotationZ(angle);
				const auto translate = DirectX::XMMatrixTranslation(0, 0, 0);
				cb_data.world        = scale * rotate * translate;

				const auto eye   = DirectX::XMFLOAT3(0, 0, -10);
				const auto focus = DirectX::XMFLOAT3(0, 0, 0);
				const auto up    = DirectX::XMFLOAT3(0, 1, 0);

				cb_data.view = DirectX::XMMatrixLookAtLH(
				    DirectX::XMLoadFloat3(&eye),
				    DirectX::XMLoadFloat3(&focus),
				    DirectX::XMLoadFloat3(&up));

				cb_data.proj = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(90), m_aspect_ratio, 1, 100);
			}

			void* ptr = nullptr;
			ASSERT_IF_FAILED(m_constant_buffer->Map(0, nullptr, &ptr));
			{
				std::memcpy(ptr, &cb_data, sizeof(ConstantBufferData));
			}
			m_constant_buffer->Unmap(0, nullptr);
		}

		ID3D12DescriptorHeap* heaps[] = { m_cbv_heap.Get() };
		m_gfx_cmd_list->SetDescriptorHeaps(_countof(heaps), heaps);
		m_gfx_cmd_list->SetGraphicsRootDescriptorTable(0, m_cbv_heap->GetGPUDescriptorHandleForHeapStart());

		D3D12_VERTEX_BUFFER_VIEW view = {};
		{
			view.BufferLocation = m_vertex_buffer->GetGPUVirtualAddress();
			view.SizeInBytes    = sizeof(Vertex) * 4;
			view.StrideInBytes  = sizeof(Vertex);
		}
		m_gfx_cmd_list->IASetVertexBuffers(0, 1, &view);
		m_gfx_cmd_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		m_gfx_cmd_list->DrawInstanced(4, 1, 0, 0);
	}

	executeCommandList();
	present(1);
	waitPreviousFrame();
}
