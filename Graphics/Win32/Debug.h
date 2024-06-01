#pragma once

#include <winerror.h>

#include <cassert>
#include <iostream>

#define ASSERT_RETURN(expr, ...) \
	do                           \
	{                            \
		if (!(expr))             \
		{                        \
			assert(expr);        \
			return __VA_ARGS__;  \
		}                        \
	} while (false)

#define ASSERT_IF_FAILED(hr)                         \
	do                                               \
	{                                                \
		if (FAILED(hr))                              \
		{                                            \
			printf("ERROR: HRESULT = 0x%08X\n", hr); \
			assert(SUCCEEDED(hr));                   \
		}                                            \
	} while (false)

#define RETURN_HRESULT_IF_FAILED(hr)                 \
	do                                               \
	{                                                \
		if (FAILED(hr))                              \
		{                                            \
			printf("ERROR: HRESULT = 0x%08X\n", hr); \
			return hr;                               \
		}                                            \
	} while (false)

#define RETURN_FALSE_IF_FAILED(hr)                   \
	do                                               \
	{                                                \
		if (FAILED(hr))                              \
		{                                            \
			printf("ERROR: HRESULT = 0x%08X\n", hr); \
			return false;                            \
		}                                            \
	} while (false)

#define ENABLE_IMGUI_DEMO_WINDOW 0
