#pragma once

#define APP_WIN32 0
#define APP_D3D11 0
#define APP_D3D12 0

#if defined(_WIN32) || defined(_WIN64)

#ifdef APP_WIN32
#undef APP_WIN32
#endif
#define APP_WIN32 1

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <Windows.h>

#if defined(_D3D11)

#ifdef APP_D3D11
#undef APP_D3D11
#endif
#define APP_D3D11 1

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include <d3d11.h>
#include <d3dcompiler.h>
#include <dxgi.h>
#include <wrl.h>

namespace MSWRL = Microsoft::WRL;

#elif defined(_D3D12)

#ifdef APP_D3D12
#undef APP_D3D12
#endif
#define APP_D3D12 1

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
