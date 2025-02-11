#pragma once

#include "Platform.h"

struct RefreshRate
{
	uint32_t numerator;
	uint32_t denominator;
};

struct DisplayMode
{
	uint32_t width;
	uint32_t height;
	RefreshRate refresh_rate;
};
