#include <kernel.h>
#include <paf.h>

#include "global.h"
#include "utils.h"

using namespace paf;

namespace psvs
{
	math::v4 ScaleColor(int32_t value, int32_t min, int32_t max)
	{
		if (value < min)
			value = min;
		if (value > max)
			value = max;

		int32_t v = ((float)(value - min) / (max - min)) * 255; // 0-255

		int32_t r = (v <= 127) ? v * 2 : 255;
		int32_t g = (v > 127) ? 255 - ((v - 128) * 2) : 255;

		return math::v4((float)r / 255.0f, (float)g / 255.0f, 0.0f, 1.0f);
	}

	const wchar_t *UnitsFromSize(int32_t bytes)
	{
		if (bytes >= 1024 * 1024)
			return L"MB";
		else if (bytes >= 1024)
			return L"kB";
		else
			return L" B";
	}

	int32_t ValueFromSize(int32_t bytes)
	{
		if (bytes >= 1024 * 1024)
			return bytes / 1024 / 1024;
		else if (bytes >= 1024)
			return bytes / 1024;
		else
			return bytes;
	}

	int32_t LengthOfValue(int32_t num)
	{
		int32_t len = 3;

		if (num < 10)
			len = 1;
		else if (num < 100)
			len = 2;

		return len;
	}
}