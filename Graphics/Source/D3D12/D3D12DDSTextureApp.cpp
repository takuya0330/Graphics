#include "D3D12DDSTextureApp.h"

#include <fstream>

#include "DDS.h"
#include "Win32/String.h"

namespace {

struct Vertex
{
	float position[3];
	float texcoord[2];
};

} // namespace

D3D12DDSTextureApp::D3D12DDSTextureApp(LPCWSTR title, UINT width, UINT height)
    : D3D12App(title, width, height)
    , m_vertex_buffer()
    , m_index_buffer()
    , m_texture()
    , m_texture_heap()
    , m_sampler_heap()
    , m_root_signature()
    , m_pipeline_state()
{
}

bool D3D12DDSTextureApp::OnInitialize()
{
	ASSERT_RETURN(D3D12App::OnInitialize(), false);

	ASSERT_RETURN(createTexture("1718889183_sample_640×426.dds"), false);
	ASSERT_RETURN(createPipelineState(), false);

	return true;
}

void D3D12DDSTextureApp::OnRender()
{
	reset();
	setViewport(static_cast<float>(m_width), static_cast<float>(m_height));
	setScissorRect(static_cast<LONG>(m_width), static_cast<LONG>(m_height));
	setBackBuffer();

	{
		m_gfx_cmd_list->SetGraphicsRootSignature(m_root_signature.Get());
		m_gfx_cmd_list->SetPipelineState(m_pipeline_state.Get());

		ID3D12DescriptorHeap* heaps[] = { m_texture_heap.Get(), m_sampler_heap.Get() };
		m_gfx_cmd_list->SetDescriptorHeaps(_countof(heaps), heaps);
		m_gfx_cmd_list->SetGraphicsRootDescriptorTable(0, m_texture_heap->GetGPUDescriptorHandleForHeapStart());
		m_gfx_cmd_list->SetGraphicsRootDescriptorTable(1, m_sampler_heap->GetGPUDescriptorHandleForHeapStart());

		D3D12_VERTEX_BUFFER_VIEW vbv = {};
		{
			vbv.BufferLocation = m_vertex_buffer->GetGPUVirtualAddress();
			vbv.SizeInBytes    = sizeof(Vertex) * 4;
			vbv.StrideInBytes  = sizeof(Vertex);
		}
		m_gfx_cmd_list->IASetVertexBuffers(0, 1, &vbv);

		D3D12_INDEX_BUFFER_VIEW ibv = {};
		{
			ibv.BufferLocation = m_index_buffer->GetGPUVirtualAddress();
			ibv.SizeInBytes    = sizeof(uint32_t) * 6;
			ibv.Format         = DXGI_FORMAT_R32_UINT;
		}
		m_gfx_cmd_list->IASetIndexBuffer(&ibv);

		m_gfx_cmd_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_gfx_cmd_list->DrawIndexedInstanced(6, 1, 0, 0, 0);
	}

	executeCommandList();
	present(1);
	waitPreviousFrame();
}

bool D3D12DDSTextureApp::createTexture(const char* filename)
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

	ComPtr<ID3D12Resource> upload_vbuffer;
	ComPtr<ID3D12Resource> upload_ibuffer;
	{
		{
			const Vertex vertices[] = {
				{{ -0.5f, +0.5f, 0 }, { 0, 0 }},
				{{ +0.5f, +0.5f, 0 }, { 1, 0 }},
				{{ -0.5f, -0.5f, 0 }, { 0, 1 }},
				{{ +0.5f, -0.5f, 0 }, { 1, 1 }},
			};

			if (!createBuffer(D3D12_HEAP_TYPE_DEFAULT, sizeof(vertices), D3D12_RESOURCE_STATE_COMMON, m_vertex_buffer.GetAddressOf()))
				return false;

			if (!createBuffer(D3D12_HEAP_TYPE_UPLOAD, sizeof(vertices), D3D12_RESOURCE_STATE_GENERIC_READ, upload_vbuffer.GetAddressOf()))
				return false;

			{
				void* data = nullptr;
				ASSERT_IF_FAILED(upload_vbuffer->Map(0, nullptr, &data));
				std::memcpy(data, vertices, sizeof(vertices));
				upload_vbuffer->Unmap(0, nullptr);
			}

			copy_cmd_list->CopyResource(m_vertex_buffer.Get(), upload_vbuffer.Get());
		}

		{
			const uint32_t indices[] = { 0, 1, 2, 2, 1, 3 };

			if (!createBuffer(D3D12_HEAP_TYPE_DEFAULT, sizeof(indices), D3D12_RESOURCE_STATE_COMMON, m_index_buffer.GetAddressOf()))
				return false;

			if (!createBuffer(D3D12_HEAP_TYPE_UPLOAD, sizeof(indices), D3D12_RESOURCE_STATE_GENERIC_READ, upload_ibuffer.GetAddressOf()))
				return false;

			{
				void* data = nullptr;
				ASSERT_IF_FAILED(upload_ibuffer->Map(0, nullptr, &data));
				std::memcpy(data, indices, sizeof(indices));
				upload_ibuffer->Unmap(0, nullptr);
			}

			copy_cmd_list->CopyResource(m_index_buffer.Get(), upload_ibuffer.Get());
		}
	}

	ComPtr<ID3D12Resource> upload_texture;
	{
		std::unique_ptr<uint8_t[]> dds;
        {
			std::ifstream ifs(UTF16To8(m_asset_dir) + filename, std::ios_base::binary);
			if (!ifs.is_open())
				return false;

            ifs.seekg(0, std::ios_base::beg);
			const auto beg = ifs.tellg();
			ifs.seekg(0, std::ios_base::end);
			const auto end = ifs.tellg();
			ifs.seekg(0, std::ios_base::beg);

            const auto size = end - beg;
			dds.reset(new uint8_t[size]);
			ifs.read(reinterpret_cast<char*>(dds.get()), size);

            if (ifs.bad())
				return false;
        }

		const uint32_t magic = *reinterpret_cast<const uint32_t*>(dds.get());
		if (magic != DDS_MAGIC)
			return false;

		size_t     offset = sizeof(uint32_t);
		const auto header = reinterpret_cast<const DDS_HEADER*>(dds.get() + offset);

		if (header->Size != sizeof(DDS_HEADER))
			return false;

		if (header->PixelFormat.Size != sizeof(DDS_PIXELFORMAT))
			return false;

		offset += sizeof(DDS_HEADER);

		bool enable_dxt10 = false;
		if (header->Flags & DDSD_PIXELFORMAT && header->PixelFormat.FourCC == _DDS_FOURCC('D', 'X', '1', '0'))
		{
			enable_dxt10 = true;
			offset += sizeof(DDS_HEADER_DXT10);
		}

		uint32_t width  = header->Width;
		uint32_t height = header->Height;

		uint32_t depth = 1;
		if (header->Flags & DDSD_DEPTH)
		{
			depth = header->Depth;
		}

		uint32_t mipmap_count = 1;
		if (header->Flags & DDSD_MIPMAPCOUNT || header->Caps & DDSCAPS_MIPMAP)
		{
			mipmap_count = header->MipMapCount;
		}

		bool     is_cube_map = false;
		uint32_t array_size  = 1;
		if (header->Caps2 & DDSCAPS2_CUBEMAP)
		{
			is_cube_map = true;
			array_size  = 6;
		}

		DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
		if (enable_dxt10)
		{
			// TODO:
		}
		else
		{
			format = GetDXGIFormatFromPixelFormat(header->PixelFormat);
			if (format == DXGI_FORMAT_UNKNOWN)
				return false;
		}

		const uint32_t                      num_subresources = array_size * mipmap_count;
		std::vector<D3D12_SUBRESOURCE_DATA> init_data(num_subresources);
		{
			uint8_t* pixel_data = dds.get() + offset;
			int      index      = 0;
			for (uint32_t i = 0; i < array_size; ++i)
			{
				uint32_t w = width;
				uint32_t h = height;
				uint32_t d = depth;
				for (uint32_t j = 0; j < mipmap_count; ++j)
				{
					uint64_t num_bytes = 0;
					uint64_t row_bytes = 0;
					if (!GetImageInfo(w, h, format, &num_bytes, &row_bytes, nullptr))
						return false;

					init_data[index].pData      = pixel_data;
					init_data[index].RowPitch   = row_bytes;
					init_data[index].SlicePitch = num_bytes;
					++index;

					pixel_data += num_bytes * d;

					w = std::max(1u, w >> 1u);
					h = std::max(1u, h >> 1u);
					d = std::max(1u, d >> 1u);
				}
			}
		}

		if (!createTexture2D(
		        D3D12_HEAP_TYPE_DEFAULT,
		        width,
		        height,
		        array_size,
		        mipmap_count,
		        format,
		        D3D12_RESOURCE_FLAG_NONE,
		        D3D12_RESOURCE_STATE_COMMON,
		        nullptr,
		        m_texture.GetAddressOf()))
			return false;

		UINT64                                          upload_size = 0;
		std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> layouts(num_subresources);
		std::vector<UINT>                               num_rows(num_subresources);
		std::vector<UINT64>                             row_size_in_bytes(num_subresources);
		const auto                                      texture_desc = m_texture->GetDesc();
		m_device->GetCopyableFootprints(
		    &texture_desc,
		    0,
		    num_subresources,
		    0,
		    &layouts[0],
		    &num_rows[0],
		    &row_size_in_bytes[0],
		    &upload_size);

		if (!createBuffer(
		        D3D12_HEAP_TYPE_UPLOAD,
		        upload_size,
		        D3D12_RESOURCE_STATE_GENERIC_READ,
		        upload_texture.GetAddressOf()))
			return false;

		{
			void* mapped_ptr = nullptr;
			ASSERT_IF_FAILED(upload_texture->Map(0, nullptr, &mapped_ptr));
			for (uint32_t i = 0; i < num_subresources; ++i)
			{
				D3D12_MEMCPY_DEST mem_dst = {
					.pData      = reinterpret_cast<uint8_t*>(mapped_ptr) + layouts[i].Offset,
					.RowPitch   = layouts[i].Footprint.RowPitch,
					.SlicePitch = layouts[i].Footprint.RowPitch * num_rows[i]
				};
				auto& mem_src = init_data[i];
				for (uint32_t z = 0; z < layouts[i].Footprint.Depth; ++z)
				{
					auto dst = reinterpret_cast<uint8_t*>(mem_dst.pData) + mem_dst.SlicePitch * z;
					auto src = reinterpret_cast<const uint8_t*>(mem_src.pData) + mem_src.SlicePitch * z;
					for (uint32_t y = 0; y < num_rows[i]; ++y)
					{
						std::memcpy(dst + mem_dst.RowPitch * y, src + mem_src.RowPitch * y, row_size_in_bytes[i]);
					}
				}
			}
			upload_texture->Unmap(0, nullptr);

			for (uint32_t i = 0; i < num_subresources; ++i)
			{
				D3D12_TEXTURE_COPY_LOCATION copy_dst = {};
				{
					copy_dst.pResource        = m_texture.Get();
					copy_dst.SubresourceIndex = i;
					copy_dst.Type             = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
				}
				D3D12_TEXTURE_COPY_LOCATION copy_src = {};
				{
					copy_src.pResource       = upload_texture.Get();
					copy_src.PlacedFootprint = layouts[i];
					copy_src.Type            = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
				}
				copy_cmd_list->CopyTextureRegion(&copy_dst, 0, 0, 0, &copy_src, nullptr);
			}
		}
	}

	copy_cmd_list->Close();

	ID3D12CommandList* cmd_lists[] = { copy_cmd_list.Get() };
	copy_cmd_queue->ExecuteCommandLists(1, cmd_lists);

	waitForGPU(copy_cmd_queue.Get());

	{
		if (!createDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, m_texture_heap.GetAddressOf()))
			return false;

		D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
		{
			srv_desc.Format                  = m_texture->GetDesc().Format;
			srv_desc.ViewDimension           = D3D12_SRV_DIMENSION_TEXTURE2D;
			srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srv_desc.Texture2D.MipLevels     = m_texture->GetDesc().MipLevels;
		}
		m_device->CreateShaderResourceView(m_texture.Get(), &srv_desc, m_texture_heap->GetCPUDescriptorHandleForHeapStart());

		if (!createDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, 1, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, m_sampler_heap.GetAddressOf()))
			return false;

		D3D12_SAMPLER_DESC sampler_desc = {};
		{
			sampler_desc.Filter         = D3D12_ENCODE_BASIC_FILTER(D3D12_FILTER_TYPE_LINEAR, D3D12_FILTER_TYPE_LINEAR, D3D12_FILTER_TYPE_LINEAR, D3D12_FILTER_REDUCTION_TYPE_STANDARD);
			sampler_desc.AddressU       = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			sampler_desc.AddressV       = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			sampler_desc.AddressW       = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			sampler_desc.MinLOD         = -FLT_MAX;
			sampler_desc.MaxLOD         = FLT_MAX;
			sampler_desc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		}
		m_device->CreateSampler(&sampler_desc, m_sampler_heap->GetCPUDescriptorHandleForHeapStart());
	}

	return true;
}

bool D3D12DDSTextureApp::createPipelineState()
{
	ComPtr<IDxcBlob> vs;
	if (!loadShader(L"PositionTexcoord_vs.hlsl", L"main", L"vs_6_0", vs.GetAddressOf()))
		return false;

	ComPtr<IDxcBlob> ps;
	if (!loadShader(L"PositionTexcoord_ps.hlsl", L"main", L"ps_6_0", ps.GetAddressOf()))
		return false;

	HRESULT hr = S_OK;

	D3D12_INPUT_ELEMENT_DESC input_layout[] = {
		{"POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	};

	{
		D3D12_DESCRIPTOR_RANGE tex_desc_range = {};
		{
			tex_desc_range.RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
			tex_desc_range.NumDescriptors                    = 1;
			tex_desc_range.BaseShaderRegister                = 0;
			tex_desc_range.RegisterSpace                     = 0;
			tex_desc_range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
		}
		D3D12_DESCRIPTOR_RANGE sampler_desc_range = {};
		{
			sampler_desc_range.RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
			sampler_desc_range.NumDescriptors                    = 1;
			sampler_desc_range.BaseShaderRegister                = 0;
			sampler_desc_range.RegisterSpace                     = 0;
			sampler_desc_range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
		}

		D3D12_ROOT_PARAMETER root_param[2] = {};
		{
			root_param[0].ParameterType                       = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			root_param[0].DescriptorTable.NumDescriptorRanges = 1;
			root_param[0].DescriptorTable.pDescriptorRanges   = &tex_desc_range;
			root_param[0].ShaderVisibility                    = D3D12_SHADER_VISIBILITY_PIXEL;
			root_param[1].ParameterType                       = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			root_param[1].DescriptorTable.NumDescriptorRanges = 1;
			root_param[1].DescriptorTable.pDescriptorRanges   = &sampler_desc_range;
			root_param[1].ShaderVisibility                    = D3D12_SHADER_VISIBILITY_PIXEL;
		}

		D3D12_ROOT_SIGNATURE_DESC root_sig_desc = {};
		{
			root_sig_desc.NumParameters = 2;
			root_sig_desc.pParameters   = root_param;
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
