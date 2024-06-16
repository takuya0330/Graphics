#include "Image.h"

#define STB_IMAGE_IMPLEMENTATION
#include "External/stb/stb_image.h"

Image::Image(const char* filename)
    : m_buffer()
    , m_width(0)
    , m_height(0)
{
	int  bpp   = 0;
	auto image = stbi_load(filename, &m_width, &m_height, &bpp, 4);

	m_buffer.resize(static_cast<size_t>(GetStride()) * m_height);
	std::memcpy(m_buffer.data(), image, GetSize());

	stbi_image_free(image);
}
