#pragma once

#include "Platform.h"

// https://learn.microsoft.com/ja-jp/windows/win32/direct3ddds/dds-header?source=recommendations
// https://learn.microsoft.com/ja-jp/windows/win32/direct3ddds/dx-graphics-dds-pguide
// https://learn.microsoft.com/ja-jp/windows/uwp/gaming/complete-code-for-ddstextureloader
// https://dench.flatlib.jp/ddsformat
// https://dench.flatlib.jp/ddsformatmemo.html
// https://dench.flatlib.jp/opengl/textures

#define _DDS_FOURCC(x, y, z, w) (((w) << 24) | ((z) << 16) | ((y) << 8) | (x))

enum DDS_CONST
{
	DDSD_CAPS        = 0x1,
	DDSD_HEIGHT      = 0x2,
	DDSD_WIDTH       = 0x4,
	DDSD_PITCH       = 0x8,
	DDSD_PIXELFORMAT = 0x1000,
	DDSD_MIPMAPCOUNT = 0x20000,
	DDSD_LINEARSIZE  = 0x80000,
	DDSD_DEPTH       = 0x800000,

	DDSCAPS_COMPLEX = 0x8,
	DDSCAPS_MIPMAP  = 0x400000,
	DDSCAPS_TEXTURE = 0x1000,

	DDSCAPS2_CUBEMAP           = 0x200,
	DDSCAPS2_CUBEMAP_POSITIVEX = 0x400,
	DDSCAPS2_CUBEMAP_NEGATIVEX = 0x800,
	DDSCAPS2_CUBEMAP_POSITIVEY = 0x1000,
	DDSCAPS2_CUBEMAP_NEGATIVEY = 0x2000,
	DDSCAPS2_CUBEMAP_POSITIVEZ = 0x4000,
	DDSCAPS2_CUBEMAP_NEGATIVEZ = 0x8000,
	DDSCAPS2_VOLUME            = 0x200000,

	DDPF_ALPHAPIXELS = 0x1,
	DDPF_ALPHA       = 0x2,
	DDPF_FOURCC      = 0x4,
	DDPF_RGB         = 0x40,
	DDPF_YUV         = 0x200,
	DDPF_LUMINANCE   = 0x20000,

	DDS_DIMENSION_TEXTURE1D = 2,
	DDS_DIMENSION_TEXTURE2D = 3,
	DDS_DIMENSION_TEXTURE3D = 4,

	DDS_RESOURCE_MISC_TEXTURECUBE = 0x4,

	DDS_ALPHA_MODE_UNKNOWN       = 0x0,
	DDS_ALPHA_MODE_STRAIGHT      = 0x1,
	DDS_ALPHA_MODE_PREMULTIPLIED = 0x2,
	DDS_ALPHA_MODE_OPAQUE        = 0x3,
	DDS_ALPHA_MODE_CUSTOM        = 0x4,

	DDS_MAGIC = 0x20534444,
};

struct DDS_PIXELFORMAT
{
	DWORD Size;
	DWORD Flags;
	DWORD FourCC;
	DWORD RGBBitCount;
	DWORD RBitMask;
	DWORD GBitMask;
	DWORD BBitMask;
	DWORD ABitMask;
};

struct DDS_HEADER
{
	DWORD           Size;
	DWORD           Flags;
	DWORD           Height;
	DWORD           Width;
	DWORD           PitchOrLinearSize;
	DWORD           Depth;
	DWORD           MipMapCount;
	DWORD           Reserved1[11];
	DDS_PIXELFORMAT PixelFormat;
	DWORD           Caps;
	DWORD           Caps2;
	DWORD           Caps3;
	DWORD           Caps4;
	DWORD           Reserved2;
};

struct DDS_HEADER_DXT10
{
	DXGI_FORMAT Format;
	UINT        Dimension;
	UINT        MiscFlag;
	UINT        ArraySize;
	UINT        MiscFlags2;
};

inline uint32_t GetBitsPerPixel(DXGI_FORMAT format)
{
	switch (format)
	{
	case DXGI_FORMAT_R32G32B32A32_TYPELESS:
	case DXGI_FORMAT_R32G32B32A32_FLOAT:
	case DXGI_FORMAT_R32G32B32A32_UINT:
	case DXGI_FORMAT_R32G32B32A32_SINT:
		return 128;

	case DXGI_FORMAT_R32G32B32_TYPELESS:
	case DXGI_FORMAT_R32G32B32_FLOAT:
	case DXGI_FORMAT_R32G32B32_UINT:
	case DXGI_FORMAT_R32G32B32_SINT:
		return 96;

	case DXGI_FORMAT_R16G16B16A16_TYPELESS:
	case DXGI_FORMAT_R16G16B16A16_FLOAT:
	case DXGI_FORMAT_R16G16B16A16_UNORM:
	case DXGI_FORMAT_R16G16B16A16_UINT:
	case DXGI_FORMAT_R16G16B16A16_SNORM:
	case DXGI_FORMAT_R16G16B16A16_SINT:
	case DXGI_FORMAT_R32G32_TYPELESS:
	case DXGI_FORMAT_R32G32_FLOAT:
	case DXGI_FORMAT_R32G32_UINT:
	case DXGI_FORMAT_R32G32_SINT:
	case DXGI_FORMAT_R32G8X24_TYPELESS:
	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
	case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
	case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
		return 64;

	case DXGI_FORMAT_R10G10B10A2_TYPELESS:
	case DXGI_FORMAT_R10G10B10A2_UNORM:
	case DXGI_FORMAT_R10G10B10A2_UINT:
	case DXGI_FORMAT_R11G11B10_FLOAT:
	case DXGI_FORMAT_R8G8B8A8_TYPELESS:
	case DXGI_FORMAT_R8G8B8A8_UNORM:
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
	case DXGI_FORMAT_R8G8B8A8_UINT:
	case DXGI_FORMAT_R8G8B8A8_SNORM:
	case DXGI_FORMAT_R8G8B8A8_SINT:
	case DXGI_FORMAT_R16G16_TYPELESS:
	case DXGI_FORMAT_R16G16_FLOAT:
	case DXGI_FORMAT_R16G16_UNORM:
	case DXGI_FORMAT_R16G16_UINT:
	case DXGI_FORMAT_R16G16_SNORM:
	case DXGI_FORMAT_R16G16_SINT:
	case DXGI_FORMAT_R32_TYPELESS:
	case DXGI_FORMAT_D32_FLOAT:
	case DXGI_FORMAT_R32_FLOAT:
	case DXGI_FORMAT_R32_UINT:
	case DXGI_FORMAT_R32_SINT:
	case DXGI_FORMAT_R24G8_TYPELESS:
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
	case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
	case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
	case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
	case DXGI_FORMAT_R8G8_B8G8_UNORM:
	case DXGI_FORMAT_G8R8_G8B8_UNORM:
	case DXGI_FORMAT_B8G8R8A8_UNORM:
	case DXGI_FORMAT_B8G8R8X8_UNORM:
	case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
	case DXGI_FORMAT_B8G8R8A8_TYPELESS:
	case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
	case DXGI_FORMAT_B8G8R8X8_TYPELESS:
	case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
		return 32;

	case DXGI_FORMAT_R8G8_TYPELESS:
	case DXGI_FORMAT_R8G8_UNORM:
	case DXGI_FORMAT_R8G8_UINT:
	case DXGI_FORMAT_R8G8_SNORM:
	case DXGI_FORMAT_R8G8_SINT:
	case DXGI_FORMAT_R16_TYPELESS:
	case DXGI_FORMAT_R16_FLOAT:
	case DXGI_FORMAT_D16_UNORM:
	case DXGI_FORMAT_R16_UNORM:
	case DXGI_FORMAT_R16_UINT:
	case DXGI_FORMAT_R16_SNORM:
	case DXGI_FORMAT_R16_SINT:
	case DXGI_FORMAT_B5G6R5_UNORM:
	case DXGI_FORMAT_B5G5R5A1_UNORM:
	case DXGI_FORMAT_B4G4R4A4_UNORM:
		return 16;

	case DXGI_FORMAT_R8_TYPELESS:
	case DXGI_FORMAT_R8_UNORM:
	case DXGI_FORMAT_R8_UINT:
	case DXGI_FORMAT_R8_SNORM:
	case DXGI_FORMAT_R8_SINT:
	case DXGI_FORMAT_A8_UNORM:
		return 8;

	case DXGI_FORMAT_R1_UNORM:
		return 1;

	case DXGI_FORMAT_BC1_TYPELESS:
	case DXGI_FORMAT_BC1_UNORM:
	case DXGI_FORMAT_BC1_UNORM_SRGB:
	case DXGI_FORMAT_BC4_TYPELESS:
	case DXGI_FORMAT_BC4_UNORM:
	case DXGI_FORMAT_BC4_SNORM:
		return 4;

	case DXGI_FORMAT_BC2_TYPELESS:
	case DXGI_FORMAT_BC2_UNORM:
	case DXGI_FORMAT_BC2_UNORM_SRGB:
	case DXGI_FORMAT_BC3_TYPELESS:
	case DXGI_FORMAT_BC3_UNORM:
	case DXGI_FORMAT_BC3_UNORM_SRGB:
	case DXGI_FORMAT_BC5_TYPELESS:
	case DXGI_FORMAT_BC5_UNORM:
	case DXGI_FORMAT_BC5_SNORM:
	case DXGI_FORMAT_BC6H_TYPELESS:
	case DXGI_FORMAT_BC6H_UF16:
	case DXGI_FORMAT_BC6H_SF16:
	case DXGI_FORMAT_BC7_TYPELESS:
	case DXGI_FORMAT_BC7_UNORM:
	case DXGI_FORMAT_BC7_UNORM_SRGB:
		return 8;

	default:
		return 0;
	}
	return 0;
}

inline DXGI_FORMAT GetDXGIFormatFromPixelFormat(const DDS_PIXELFORMAT& ddspf)
{
	auto isBitMask = [ddspf](DWORD r, DWORD g, DWORD b, DWORD a) {
		return ddspf.RBitMask == r && ddspf.GBitMask == g && ddspf.BBitMask == b && ddspf.ABitMask == a;
	};

	DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;

	if (ddspf.Flags & DDPF_RGB)
	{
		switch (ddspf.RGBBitCount)
		{
		case 32:
			if (isBitMask(0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000))
			{
				format = DXGI_FORMAT_R8G8B8A8_UNORM;
			}
			else if (isBitMask(0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000))
			{
				format = DXGI_FORMAT_B8G8R8A8_UNORM;
			}
			else if (isBitMask(0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000))
			{
				format = DXGI_FORMAT_B8G8R8X8_UNORM;
			}
			else if (isBitMask(0x3ff00000, 0x000ffc00, 0x000003ff, 0xc0000000))
			{
				format = DXGI_FORMAT_R10G10B10A2_UNORM;
			}
			else if (isBitMask(0x0000ffff, 0xffff0000, 0x00000000, 0x00000000))
			{
				format = DXGI_FORMAT_R16G16_UNORM;
			}
			else if (isBitMask(0xffffffff, 0x00000000, 0x00000000, 0x00000000))
			{
				format = DXGI_FORMAT_R32_FLOAT;
			}
			break;

		case 24:
			break;

		case 16:
			if (isBitMask(0x7c00, 0x03e0, 0x001f, 0x8000))
			{
				format = DXGI_FORMAT_B5G5R5A1_UNORM;
			}
			else if (isBitMask(0xf800, 0x07e0, 0x001f, 0x0000))
			{
				format = DXGI_FORMAT_B5G6R5_UNORM;
			}
			else if (isBitMask(0x0f00, 0x00f0, 0x000f, 0xf000))
			{
				format = DXGI_FORMAT_B4G4R4A4_UNORM;
			}
			break;

		default:
			break;
		}
	}
	else if (ddspf.Flags & DDPF_LUMINANCE)
	{
		switch (ddspf.RGBBitCount)
		{
		case 16:
			if (isBitMask(0x0000ffff, 0x00000000, 0x00000000, 0x00000000))
			{
				format = DXGI_FORMAT_R16_UNORM;
			}
			else if (isBitMask(0x000000ff, 0x00000000, 0x00000000, 0x0000ff00))
			{
				format = DXGI_FORMAT_R8G8_UNORM;
			}
			break;

		case 8:
			if (isBitMask(0x000000ff, 0x00000000, 0x00000000, 0x00000000))
			{
				format = DXGI_FORMAT_R8_UNORM;
			}
			break;

		default:
			break;
		}
	}
	else if (ddspf.Flags & DDPF_ALPHA)
	{
		if (ddspf.RGBBitCount == 8)
		{
			format = DXGI_FORMAT_A8_UNORM;
		}
	}
	else if (ddspf.Flags & DDPF_FOURCC)
	{
		switch (ddspf.FourCC)
		{
		case _DDS_FOURCC('D', 'X', 'T', '1'):
			format = DXGI_FORMAT_BC1_UNORM;
			break;

		case _DDS_FOURCC('D', 'X', 'T', '3'):
			format = DXGI_FORMAT_BC2_UNORM;
			break;

		case _DDS_FOURCC('D', 'X', 'T', '5'):
			format = DXGI_FORMAT_BC3_UNORM;
			break;

		case _DDS_FOURCC('D', 'X', 'T', '2'):
			format = DXGI_FORMAT_BC2_UNORM;
			break;

		case _DDS_FOURCC('D', 'X', 'T', '4'):
			format = DXGI_FORMAT_BC3_UNORM;
			break;

		case _DDS_FOURCC('A', 'T', 'I', '1'):
			format = DXGI_FORMAT_BC4_UNORM;
			break;

		case _DDS_FOURCC('B', 'C', '4', 'U'):
			format = DXGI_FORMAT_BC4_UNORM;
			break;

		case _DDS_FOURCC('B', 'C', '4', 'S'):
			format = DXGI_FORMAT_BC4_SNORM;
			break;

		case _DDS_FOURCC('A', 'T', 'I', '2'):
			format = DXGI_FORMAT_BC5_UNORM;
			break;

		case _DDS_FOURCC('B', 'C', '5', 'U'):
			format = DXGI_FORMAT_BC5_UNORM;
			break;

		case _DDS_FOURCC('B', 'C', '5', 'S'):
			format = DXGI_FORMAT_BC5_SNORM;
			break;

		case _DDS_FOURCC('R', 'G', 'B', 'G'):
			format = DXGI_FORMAT_R8G8_B8G8_UNORM;
			break;

		case _DDS_FOURCC('G', 'R', 'G', 'B'):
			format = DXGI_FORMAT_G8R8_G8B8_UNORM;
			break;

		case 36: // D3DFMT_A16B16G16R16
			format = DXGI_FORMAT_R16G16B16A16_UNORM;
			break;

		case 110: // D3DFMT_Q16W16V16U16
			format = DXGI_FORMAT_R16G16B16A16_SNORM;
			break;

		case 111: // D3DFMT_R16F
			format = DXGI_FORMAT_R16_FLOAT;
			break;

		case 112: // D3DFMT_G16R16F
			format = DXGI_FORMAT_R16G16_FLOAT;
			break;

		case 113: // D3DFMT_A16B16G16R16F
			format = DXGI_FORMAT_R16G16B16A16_FLOAT;
			break;

		case 114: // D3DFMT_R32F
			format = DXGI_FORMAT_R32_FLOAT;
			break;

		case 115: // D3DFMT_G32R32F
			format = DXGI_FORMAT_R32G32_FLOAT;
			break;

		case 116: // D3DFMT_A32B32G32R32F
			format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			break;

		default:
			break;
		}
	}

	return format;
}

inline bool GetImageInfo(uint32_t width, uint32_t height, DXGI_FORMAT format, uint64_t* out_num_bytes, uint64_t* out_row_bytes, uint64_t* out_num_rows)
{
	uint64_t num_bytes = 0;
	uint64_t row_bytes = 0;
	uint64_t num_rows  = 0;

	uint32_t bpe       = 0;
	bool     is_bc     = false;
	bool     is_packed = false;
	bool     is_planar = false;

	switch (format)
	{
	case DXGI_FORMAT_BC1_TYPELESS:
	case DXGI_FORMAT_BC1_UNORM:
	case DXGI_FORMAT_BC1_UNORM_SRGB:
	case DXGI_FORMAT_BC4_TYPELESS:
	case DXGI_FORMAT_BC4_UNORM:
	case DXGI_FORMAT_BC4_SNORM:
		is_bc = true;
		bpe   = 8;
		break;

	case DXGI_FORMAT_BC2_TYPELESS:
	case DXGI_FORMAT_BC2_UNORM:
	case DXGI_FORMAT_BC2_UNORM_SRGB:
	case DXGI_FORMAT_BC3_TYPELESS:
	case DXGI_FORMAT_BC3_UNORM:
	case DXGI_FORMAT_BC3_UNORM_SRGB:
	case DXGI_FORMAT_BC5_TYPELESS:
	case DXGI_FORMAT_BC5_UNORM:
	case DXGI_FORMAT_BC5_SNORM:
	case DXGI_FORMAT_BC6H_TYPELESS:
	case DXGI_FORMAT_BC6H_UF16:
	case DXGI_FORMAT_BC6H_SF16:
	case DXGI_FORMAT_BC7_TYPELESS:
	case DXGI_FORMAT_BC7_UNORM:
	case DXGI_FORMAT_BC7_UNORM_SRGB:
		is_bc = true;
		bpe   = 16;
		break;

	case DXGI_FORMAT_R8G8_B8G8_UNORM:
	case DXGI_FORMAT_G8R8_G8B8_UNORM:
	case DXGI_FORMAT_YUY2:
		is_packed = true;
		bpe       = 4;
		break;

	case DXGI_FORMAT_Y210:
	case DXGI_FORMAT_Y216:
		is_packed = true;
		bpe       = 8;
		break;

	case DXGI_FORMAT_NV12:
	case DXGI_FORMAT_420_OPAQUE:
		if ((height % 2) != 0)
			return false;
		is_planar = true;
		bpe       = 2;
		break;

	case DXGI_FORMAT_P208:
		is_planar = true;
		bpe       = 2;
		break;

	case DXGI_FORMAT_P010:
	case DXGI_FORMAT_P016:
		if ((height % 2) != 0)
			return false;
		is_planar = true;
		bpe       = 4;
		break;

	default:
		break;
	}

	if (is_bc)
	{
		uint64_t pitch = std::max(1ull, (width + 3ull) / 4ull);
		uint64_t slice = std::max(1ull, (height + 3ull) / 4ull);

		row_bytes = pitch * bpe;
		num_rows  = slice;
		num_bytes = row_bytes * num_rows;
	}
	else if (is_packed)
	{
		row_bytes = ((width + 1ull) >> 1ull) * bpe;
		num_rows  = height;
		num_bytes = row_bytes * height;
	}
	else if (format == DXGI_FORMAT_NV11)
	{
		row_bytes = ((width + 3ull) >> 2ull) * 4ull;
		num_rows  = height * 2ull;
		num_bytes = row_bytes * num_rows;
	}
	else if (is_planar)
	{
		row_bytes = ((width + 1ull) >> 1ull) * bpe;
		num_rows  = height + ((height + 1ull) >> 1ull);
		num_bytes = (row_bytes * height) + ((row_bytes * height + 1ull) >> 1ull);
	}
	else
	{
		const auto bpp = GetBitsPerPixel(format);
		if (bpp == 0)
			return false;

		row_bytes = (width * bpp + 7ull) * 8ull;
		num_rows  = height;
		num_bytes = row_bytes * height;
	}

	if (out_num_bytes)
		*out_num_bytes = num_bytes;
	if (out_row_bytes)
		*out_row_bytes = row_bytes;
	if (out_num_rows)
		*out_num_rows = num_rows;

	return true;
}
