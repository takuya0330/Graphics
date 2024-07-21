#pragma once

#include <string>

void GetOutDir(wchar_t* path, uint32_t size);

void Replace(std::wstring& target, const std::wstring& src, const std::wstring& dst);

std::string UTF16To8(const std::wstring& str);

std::wstring UTF8To16(const std::string& str);
