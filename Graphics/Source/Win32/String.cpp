#include "String.h"

#include "Platform.h"

void GetOutDir(wchar_t* path, uint32_t size)
{
	if (!path)
		return;

	auto ret = GetModuleFileName(nullptr, path, size);
	if (ret == 0 || ret == size)
		return;

	wchar_t* end = wcsrchr(path, L'\\');
	if (end)
		*(end + 1) = L'\0';
}

void Replace(std::wstring& target, const std::wstring& src, const std::wstring& dst)
{
	size_t pos    = 0;
	size_t offset = 0;
	size_t length = src.length();
	while ((pos = target.find(src, offset)) != std::wstring::npos)
	{
		target.replace(pos, length, dst);
		offset = pos + dst.length();
	}
}

std::string UTF16To8(const std::wstring& str)
{
	const auto  size = WideCharToMultiByte(CP_UTF8, 0, str.c_str(), -1, nullptr, 0, nullptr, nullptr);
	std::string utf8(size - 1, '\0');
	WideCharToMultiByte(CP_UTF8, 0, str.c_str(), -1, &utf8[0], size - 1, nullptr, nullptr);
	return utf8;
}

std::wstring UTF8To16(const std::string& str)
{
	const auto   size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
	std::wstring utf16(size - 1, L'\0');
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &utf16[0], size - 1);
	return utf16;
}
