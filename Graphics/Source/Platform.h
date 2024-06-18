#pragma once

#if defined(_WIN32) || defined(_WIN64)

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <Windows.h>

#if defined(_D3D11)

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include <d3d11.h>
#include <d3dcompiler.h>
#include <dxgi.h>
#include <wrl.h>

namespace MSWRL = Microsoft::WRL;

#elif defined(_D3D12)

#define DXC_LIB_PATH "../../Source/External/DirectXShaderCompiler/lib/x64/"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, DXC_LIB_PATH "dxcompiler.lib")

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

#include "External/DirectXShaderCompiler/inc/dxcapi.h"

namespace MSWRL = Microsoft::WRL;

#endif
#endif
