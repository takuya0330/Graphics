#pragma once

#include <type_traits>
#include <vector>

class Image
{
public:
	Image(const char* filename);

	const uint8_t* GetBuffer() const
	{
		return m_buffer.data();
	}

	uint8_t* GetBuffer()
	{
		return const_cast<uint8_t*>(std::as_const(*this).GetBuffer());
	}

	size_t GetSize() const noexcept
	{
		return m_buffer.size();
	}

	int GetWidth() const noexcept
	{
		return m_width;
	}

	int GetHeight() const noexcept
	{
		return m_height;
	}

	int GetStride() const noexcept
	{
		return m_width * 4;
	}

private:
	std::vector<uint8_t> m_buffer;
	int                  m_width;
	int                  m_height;
};
