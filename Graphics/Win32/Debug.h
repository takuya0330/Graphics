#pragma once

#include <winerror.h>

#include <cassert>
#include <iostream>

#define ASSERT_RETURN(expr, ...) \
	if (!(expr))                 \
	{                            \
		assert(expr);            \
		return __VA_ARGS__;      \
	}

#define RETURN_HRESULT_IF_FAILED(hr)             \
	if (FAILED(hr))                              \
	{                                            \
		printf("ERROR: HRESULT = 0x%08X\n", hr); \
		return hr;                               \
	}

#define RETURN_FALSE_IF_FAILED(hr)               \
	if (FAILED(hr))                              \
	{                                            \
		printf("ERROR: HRESULT = 0x%08X\n", hr); \
		return false;                            \
	}

#define ASSERT_IF_FAILED(hr)                     \
	if (FAILED(hr))                              \
	{                                            \
		printf("ERROR: HRESULT = 0x%08X\n", hr); \
		assert(SUCCEEDED(hr));                   \
	}
