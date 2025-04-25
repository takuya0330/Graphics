#pragma once

#include <cassert>
#include <iostream>
#include <winerror.h>

namespace Debug {

#if 0
inline void Print(const char* msg)
{
	printf("%s", msg);
}
inline void Print(const wchar_t* msg)
{
	wprintf(L"%ws", msg);
}
#else
inline void Print(const char* msg)
{
	OutputDebugStringA(msg);
}
inline void Print(const wchar_t* msg)
{
	OutputDebugStringW(msg);
}
#endif

inline void Log(const char* format, ...)
{
	char    buffer[256];
	va_list ap;
	va_start(ap, format);
	vsprintf_s(buffer, 256, format, ap);
	va_end(ap);
	Print(buffer);
}

inline void Log(const wchar_t* format, ...)
{
	wchar_t buffer[256];
	va_list ap;
	va_start(ap, format);
	vswprintf(buffer, 256, format, ap);
	va_end(ap);
	Print(buffer);
}

} // namespace Debug

#define ASSERT(expr) assert(expr)

#define ASSERT_RETURN(expr, ...) \
	do                           \
	{                            \
		if (!(expr))             \
		{                        \
			assert(expr);        \
			return __VA_ARGS__;  \
		}                        \
	} while (false)

#define ASSERT_IF_FAILED(hr)                              \
	do                                                    \
	{                                                     \
		if (FAILED(hr))                                   \
		{                                                 \
			Debug::Log(L"ERROR: HRESULT = 0x%08X\n", hr); \
			assert(SUCCEEDED(hr));                        \
		}                                                 \
	} while (false)

#define RETURN_HRESULT_IF_FAILED(hr)                      \
	do                                                    \
	{                                                     \
		if (FAILED(hr))                                   \
		{                                                 \
			Debug::Log(L"ERROR: HRESULT = 0x%08X\n", hr); \
			return hr;                                    \
		}                                                 \
	} while (false)

#define RETURN_FALSE_IF_FAILED(hr)                        \
	do                                                    \
	{                                                     \
		if (FAILED(hr))                                   \
		{                                                 \
			Debug::Log(L"ERROR: HRESULT = 0x%08X\n", hr); \
			return false;                                 \
		}                                                 \
	} while (false)

#define ENABLE_IMGUI_DEMO_WINDOW 1
