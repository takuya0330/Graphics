#include "D3D12RayTracingTriangleApp.h"

#include "Win32/Debug.h"

#include <DirectXMath.h>
using namespace DirectX;

namespace {

struct Vertex
{
	XMFLOAT3 position;
};

} // namespace

D3D12RayTracingTriangleApp::D3D12RayTracingTriangleApp(LPCWSTR title, UINT width, UINT height)
    : D3D12App(title, width, height)
    , m_device5()
    , m_gfx_cmd_list4()
    , m_vertex_buffer()
    , m_blas()
    , m_tlas()
    , m_global_root_signature()
    , m_state_object()
    , m_dxr_output()
    , m_resource_heap()
    , m_resource_alloc_size(0)
    , m_shader_table()
    , m_dispatch_desc()
{
}

bool D3D12RayTracingTriangleApp::OnInitialize()
{
	ASSERT_RETURN(D3D12App::OnInitialize(), false);

	// DXR のサポートチェック
	if (!isSupportedRayTracing())
		return false;

	auto hr = m_device.As(&m_device5);
	RETURN_FALSE_IF_FAILED(hr);

	hr = Utilities::CreateCommandList(
	    m_device5.Get(),
	    D3D12_COMMAND_LIST_TYPE_DIRECT,
	    m_gfx_cmd_allocators.at(0).Get(),
	    m_gfx_cmd_list4.GetAddressOf());
	RETURN_FALSE_IF_FAILED(hr);

	// BLAS 作成
	{
		hr = Utilities::Reset(
		    m_gfx_cmd_allocators.at(m_back_buffer_index).Get(),
		    m_gfx_cmd_list4.Get());
		RETURN_FALSE_IF_FAILED(hr);

		// 頂点バッファ作成
		const Vertex vertices[] = {
			XMFLOAT3(+0.0f, +0.5f, +0),
			XMFLOAT3(+0.5f, -0.5f, +0),
			XMFLOAT3(-0.5f, -0.5f, +0),
		};
		if (!createBuffer(
		        D3D12_HEAP_TYPE_UPLOAD,
		        sizeof(vertices),
		        D3D12_RESOURCE_STATE_GENERIC_READ,
		        m_vertex_buffer.GetAddressOf()))
			return false;
		{
			void* ptr = nullptr;
			hr        = m_vertex_buffer->Map(0, nullptr, &ptr);
			RETURN_FALSE_IF_FAILED(hr);
			std::memcpy(ptr, vertices, sizeof(vertices));
			m_vertex_buffer->Unmap(0, nullptr);
		}

		D3D12_RAYTRACING_GEOMETRY_DESC geometry_desc = {};
		{
			geometry_desc.Type                                 = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
			geometry_desc.Flags                                = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
			geometry_desc.Triangles.VertexFormat               = DXGI_FORMAT_R32G32B32_FLOAT;
			geometry_desc.Triangles.VertexCount                = _countof(vertices);
			geometry_desc.Triangles.VertexBuffer.StartAddress  = m_vertex_buffer->GetGPUVirtualAddress();
			geometry_desc.Triangles.VertexBuffer.StrideInBytes = sizeof(Vertex);
		}

		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC build_desc = {};
		auto&                                              inputs     = build_desc.Inputs;
		{
			inputs.Type           = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
			inputs.Flags          = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
			inputs.NumDescs       = 1;
			inputs.DescsLayout    = D3D12_ELEMENTS_LAYOUT_ARRAY;
			inputs.pGeometryDescs = &geometry_desc;
		}

		AccelerationStructure blas;
		if (!createAccelerationStructure(
		        m_device5.Get(),
		        build_desc,
		        blas))
			return false;
		m_blas = blas.result;

		m_gfx_cmd_list4->BuildRaytracingAccelerationStructure(&build_desc, 0, nullptr);

		Utilities::SetResourceBarrierUAV(m_gfx_cmd_list4.Get(), m_blas.Get());
		Utilities::ExecuteCommandList(m_gfx_cmd_queue.Get(), m_gfx_cmd_list4.Get());

		waitForGPU(m_gfx_cmd_queue.Get());
	}

	// TLAS 作成
	{
		hr = Utilities::Reset(
		    m_gfx_cmd_allocators.at(m_back_buffer_index).Get(),
		    m_gfx_cmd_list4.Get());
		RETURN_FALSE_IF_FAILED(hr);

		D3D12_RAYTRACING_INSTANCE_DESC instance_desc = {};
		{
			XMStoreFloat3x4(reinterpret_cast<XMFLOAT3X4*>(&instance_desc.Transform), XMMatrixIdentity());
			instance_desc.InstanceID                          = 0;
			instance_desc.InstanceMask                        = 0xFF;
			instance_desc.InstanceContributionToHitGroupIndex = 0;
			instance_desc.Flags                               = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;
			instance_desc.AccelerationStructure               = m_blas->GetGPUVirtualAddress();
		}

		ComPtr<ID3D12Resource> instance;
		if (!createBuffer(
		        D3D12_HEAP_TYPE_UPLOAD,
		        sizeof(instance_desc),
		        D3D12_RESOURCE_STATE_GENERIC_READ,
		        instance.GetAddressOf()))
			return false;
		{
			void* ptr = nullptr;
			hr        = instance->Map(0, nullptr, &ptr);
			RETURN_FALSE_IF_FAILED(hr);
			std::memcpy(ptr, &instance_desc, sizeof(instance_desc));
			instance->Unmap(0, nullptr);
		}

		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC build_desc = {};
		auto&                                              inputs     = build_desc.Inputs;
		{
			inputs.Type          = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
			inputs.Flags         = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
			inputs.NumDescs      = 1;
			inputs.DescsLayout   = D3D12_ELEMENTS_LAYOUT_ARRAY;
			inputs.InstanceDescs = instance->GetGPUVirtualAddress();
		}

		AccelerationStructure tlas;
		if (!createAccelerationStructure(
		        m_device5.Get(),
		        build_desc,
		        tlas))
			return false;
		m_tlas = tlas.result;

		m_gfx_cmd_list4->BuildRaytracingAccelerationStructure(&build_desc, 0, nullptr);

		Utilities::SetResourceBarrierUAV(m_gfx_cmd_list4.Get(), m_tlas.Get());
		Utilities::ExecuteCommandList(m_gfx_cmd_queue.Get(), m_gfx_cmd_list4.Get());

		waitForGPU(m_gfx_cmd_queue.Get());
	}

	// グローバルルートシグネチャ作成
	{
		D3D12_DESCRIPTOR_RANGE range_tlas = {};
		{
			range_tlas.RangeType          = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
			range_tlas.NumDescriptors     = 1;
			range_tlas.BaseShaderRegister = 0;
		}
		D3D12_DESCRIPTOR_RANGE range_output = {};
		{
			range_output.RangeType          = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
			range_output.NumDescriptors     = 1;
			range_output.BaseShaderRegister = 0;
		}
		D3D12_ROOT_PARAMETER root_params[2] = {};
		{
			root_params[0].ParameterType                       = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			root_params[0].DescriptorTable.NumDescriptorRanges = 1;
			root_params[0].DescriptorTable.pDescriptorRanges   = &range_tlas;
			root_params[1].ParameterType                       = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			root_params[1].DescriptorTable.NumDescriptorRanges = 1;
			root_params[1].DescriptorTable.pDescriptorRanges   = &range_output;
		}

		D3D12_ROOT_SIGNATURE_DESC root_sig_desc = {};
		{
			root_sig_desc.NumParameters = _countof(root_params);
			root_sig_desc.pParameters   = root_params;
		}
		ComPtr<ID3DBlob> blob, error;
		hr = D3D12SerializeRootSignature(
		    &root_sig_desc,
		    D3D_ROOT_SIGNATURE_VERSION_1_0,
		    blob.ReleaseAndGetAddressOf(),
		    error.ReleaseAndGetAddressOf());
		RETURN_FALSE_IF_FAILED(hr);

		hr = m_device->CreateRootSignature(
		    0,
		    blob->GetBufferPointer(),
		    blob->GetBufferSize(),
		    IID_PPV_ARGS(m_global_root_signature.GetAddressOf()));
		RETURN_FALSE_IF_FAILED(hr);
	}

	// ステートオブジェクト作成
	{
		ComPtr<IDxcBlob> blob;
		if (!loadShader(L"DXRTriangle.hlsl", L"", L"lib_6_4", blob.GetAddressOf()))
			return false;

		std::vector<D3D12_STATE_SUBOBJECT> subobjects;

		D3D12_EXPORT_DESC exports[] = {
			{ L"MainRGS", nullptr, D3D12_EXPORT_FLAG_NONE },
			{ L"MainMS", nullptr, D3D12_EXPORT_FLAG_NONE },
			{ L"MainCHS", nullptr, D3D12_EXPORT_FLAG_NONE },
		};
		D3D12_DXIL_LIBRARY_DESC library = {};
		{
			library.DXILLibrary.pShaderBytecode = blob->GetBufferPointer();
			library.DXILLibrary.BytecodeLength  = blob->GetBufferSize();
			library.NumExports                  = _countof(exports);
			library.pExports                    = exports;
		}
		subobjects.emplace_back(D3D12_STATE_SUBOBJECT { D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY, &library });

		D3D12_HIT_GROUP_DESC hit_group = {};
		{
			hit_group.Type                   = D3D12_HIT_GROUP_TYPE_TRIANGLES;
			hit_group.ClosestHitShaderImport = L"MainCHS";
			hit_group.HitGroupExport         = L"DefaultHitGroup";
		}
		subobjects.emplace_back(D3D12_STATE_SUBOBJECT { D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP, &hit_group });

		D3D12_GLOBAL_ROOT_SIGNATURE global = {};
		{
			global.pGlobalRootSignature = m_global_root_signature.Get();
		}
		subobjects.emplace_back(D3D12_STATE_SUBOBJECT { D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE, &global });

		D3D12_RAYTRACING_SHADER_CONFIG shader = {};
		{
			shader.MaxPayloadSizeInBytes   = sizeof(XMFLOAT3);
			shader.MaxAttributeSizeInBytes = sizeof(XMFLOAT2);
		}
		subobjects.emplace_back(D3D12_STATE_SUBOBJECT { D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG, &shader });

		D3D12_RAYTRACING_PIPELINE_CONFIG pipeline = {};
		{
			pipeline.MaxTraceRecursionDepth = 1;
		}
		subobjects.emplace_back(D3D12_STATE_SUBOBJECT { D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG, &pipeline });

		D3D12_STATE_OBJECT_DESC state_obj_desc = {};
		{
			state_obj_desc.Type          = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;
			state_obj_desc.NumSubobjects = static_cast<UINT>(subobjects.size());
			state_obj_desc.pSubobjects   = subobjects.data();
		}
		hr = m_device5->CreateStateObject(&state_obj_desc, IID_PPV_ARGS(m_state_object.GetAddressOf()));
		RETURN_FALSE_IF_FAILED(hr);
	}

	// 出力先バッファ作成
	if (!createTexture2D(
	        D3D12_HEAP_TYPE_DEFAULT,
	        m_width,
	        m_height,
	        1,
	        1,
	        DXGI_FORMAT_R8G8B8A8_UNORM,
	        D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
	        D3D12_RESOURCE_STATE_COPY_SOURCE,
	        nullptr,
	        m_dxr_output.GetAddressOf()))
		return false;

	// ディスクリプタヒープ作成
	if (!createDescriptorHeap(
	        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
	        2,
	        D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
	        m_resource_heap.GetAddressOf()))
		return false;

	m_resource_alloc_size = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	auto cpu_handle       = m_resource_heap->GetCPUDescriptorHandleForHeapStart();

	D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
	{
		srv_desc.ViewDimension                            = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
		srv_desc.Shader4ComponentMapping                  = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srv_desc.RaytracingAccelerationStructure.Location = m_tlas->GetGPUVirtualAddress();
	}
	m_device->CreateShaderResourceView(nullptr, &srv_desc, cpu_handle);

	cpu_handle.ptr += m_resource_alloc_size;
	D3D12_UNORDERED_ACCESS_VIEW_DESC uav_desc = {};
	{
		uav_desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	}
	m_device->CreateUnorderedAccessView(m_dxr_output.Get(), nullptr, &uav_desc, cpu_handle);

	// シェーダーテーブル作成
	{
		const auto record_size    = calcAlignment(D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);
		const auto ray_gen_size   = 1 * record_size;
		const auto miss_size      = 1 * record_size;
		const auto hit_group_size = 1 * record_size;

		const auto ray_gen_aligned_size   = calcAlignment(ray_gen_size, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);
		const auto miss_aligned_size      = calcAlignment(miss_size, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);
		const auto hit_group_aligned_size = calcAlignment(hit_group_size, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);

		if (!createBuffer(
		        D3D12_HEAP_TYPE_UPLOAD,
		        ray_gen_aligned_size + miss_aligned_size + hit_group_aligned_size,
		        D3D12_RESOURCE_STATE_GENERIC_READ,
		        m_shader_table.GetAddressOf()))
			return false;

		ComPtr<ID3D12StateObjectProperties> props;
		m_state_object.As(&props);

		uint8_t* ptr = nullptr;
		hr           = m_shader_table->Map(0, nullptr, reinterpret_cast<void**>(&ptr));
		RETURN_FALSE_IF_FAILED(hr);

		auto ray_gen_id = props->GetShaderIdentifier(L"MainRGS");
		std::memcpy(ptr, ray_gen_id, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
		ptr += ray_gen_aligned_size;

		auto miss_id = props->GetShaderIdentifier(L"MainMS");
		std::memcpy(ptr, miss_id, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
		ptr += miss_aligned_size;

		auto hit_group_id = props->GetShaderIdentifier(L"DefaultHitGroup");
		std::memcpy(ptr, hit_group_id, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
		ptr += hit_group_aligned_size;

		m_shader_table->Unmap(0, nullptr);

		auto  address        = m_shader_table->GetGPUVirtualAddress();
		auto& ray_gen_record = m_dispatch_desc.RayGenerationShaderRecord;
		{
			ray_gen_record.StartAddress = address;
			ray_gen_record.SizeInBytes  = ray_gen_size;
			address += ray_gen_aligned_size;
		}
		auto& miss_record = m_dispatch_desc.MissShaderTable;
		{
			miss_record.StartAddress  = address;
			miss_record.SizeInBytes   = miss_size;
			miss_record.StrideInBytes = record_size;
			address += miss_aligned_size;
		}
		auto& hit_group_record = m_dispatch_desc.HitGroupTable;
		{
			hit_group_record.StartAddress  = address;
			hit_group_record.SizeInBytes   = hit_group_size;
			hit_group_record.StrideInBytes = record_size;
			address += hit_group_aligned_size;
		}
		m_dispatch_desc.Width  = m_width;
		m_dispatch_desc.Height = m_height;
		m_dispatch_desc.Depth  = 1;
	}

	return true;
}

void D3D12RayTracingTriangleApp::OnFinalize()
{
	D3D12App::OnFinalize();
}

void D3D12RayTracingTriangleApp::OnUpdate()
{
}

void D3D12RayTracingTriangleApp::OnRender()
{
	auto hr = Utilities::Reset(
	    m_gfx_cmd_allocators.at(m_back_buffer_index).Get(),
	    m_gfx_cmd_list4.Get());
	ASSERT_IF_FAILED(hr);

	Utilities::SetViewport(
	    m_gfx_cmd_list4.Get(),
	    static_cast<float>(m_width),
	    static_cast<float>(m_height));
	Utilities::SetScissorRect(
	    m_gfx_cmd_list4.Get(),
	    static_cast<LONG>(m_width),
	    static_cast<LONG>(m_height));

	m_gfx_cmd_list4->SetComputeRootSignature(m_global_root_signature.Get());
	m_gfx_cmd_list4->SetDescriptorHeaps(1, m_resource_heap.GetAddressOf());
	auto gpu_handle = m_resource_heap->GetGPUDescriptorHandleForHeapStart();
	m_gfx_cmd_list4->SetComputeRootDescriptorTable(0, gpu_handle);
	gpu_handle.ptr += m_resource_alloc_size;
	m_gfx_cmd_list4->SetComputeRootDescriptorTable(1, gpu_handle);

	Utilities::SetResourceBarrierTransition(
	    m_gfx_cmd_list4.Get(),
	    m_dxr_output.Get(),
	    D3D12_RESOURCE_STATE_COPY_SOURCE,
	    D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	m_gfx_cmd_list4->SetPipelineState1(m_state_object.Get());
	m_gfx_cmd_list4->DispatchRays(&m_dispatch_desc);

	D3D12_RESOURCE_BARRIER barriers[] = {
		Utilities::ResourceBarrierTrantision(m_dxr_output.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE),
		Utilities::ResourceBarrierTrantision(m_back_buffers.at(m_back_buffer_index).Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_COPY_DEST),
	};
	m_gfx_cmd_list4->ResourceBarrier(2, barriers);
	m_gfx_cmd_list4->CopyResource(m_back_buffers.at(m_back_buffer_index).Get(), m_dxr_output.Get());

	Utilities::SetResourceBarrierTransition(
	    m_gfx_cmd_list4.Get(),
	    m_back_buffers.at(m_back_buffer_index).Get(),
	    D3D12_RESOURCE_STATE_COPY_DEST,
	    D3D12_RESOURCE_STATE_PRESENT);
	Utilities::ExecuteCommandList(m_gfx_cmd_queue.Get(), m_gfx_cmd_list4.Get());

	present(1);
	waitPreviousFrame();
}

bool D3D12RayTracingTriangleApp::isSupportedRayTracing()
{
	HRESULT                           hr       = S_OK;
	D3D12_FEATURE_DATA_D3D12_OPTIONS5 options5 = {};

	hr = m_device->CheckFeatureSupport(
	    D3D12_FEATURE_D3D12_OPTIONS5,
	    &options5,
	    sizeof(options5));
	RETURN_FALSE_IF_FAILED(hr);

	if (options5.RaytracingTier == D3D12_RAYTRACING_TIER_NOT_SUPPORTED)
	{
		Debug::Log("DirectX Raytracing not supported.\n");
		return false;
	}

	return true;
}

bool D3D12RayTracingTriangleApp::createAccelerationStructure(
    ID3D12Device5*                                      dxr_device,
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC& acceleration_structure_desc,
    AccelerationStructure&                              acceleration_structure)
{
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuild_info = {};
	dxr_device->GetRaytracingAccelerationStructurePrebuildInfo(
	    &acceleration_structure_desc.Inputs,
	    &prebuild_info);
	if (prebuild_info.ResultDataMaxSizeInBytes == 0)
		return false;

	auto hr = Utilities::CreateBuffer(
	    dxr_device,
	    D3D12_HEAP_TYPE_DEFAULT,
	    prebuild_info.ScratchDataSizeInBytes,
	    D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
	    D3D12_RESOURCE_STATE_COMMON, // D3D12 WARNING: ID3D12Device::CreateCommittedResource: Ignoring InitialState D3D12_RESOURCE_STATE_UNORDERED_ACCESS. Buffers are effectively created in state D3D12_RESOURCE_STATE_COMMON. [ STATE_CREATION WARNING #1328: CREATERESOURCE_STATE_IGNORED]
	    acceleration_structure.scratch.GetAddressOf());
	RETURN_FALSE_IF_FAILED(hr);

	hr = Utilities::CreateBuffer(
	    dxr_device,
	    D3D12_HEAP_TYPE_DEFAULT,
	    prebuild_info.ResultDataMaxSizeInBytes,
	    D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
	    D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
	    acceleration_structure.result.GetAddressOf());
	RETURN_FALSE_IF_FAILED(hr);

	acceleration_structure_desc.ScratchAccelerationStructureData = acceleration_structure.scratch->GetGPUVirtualAddress();
	acceleration_structure_desc.DestAccelerationStructureData    = acceleration_structure.result->GetGPUVirtualAddress();

	return true;
}
