#pragma once

#include <kernel.h>
#include <paf.h>

namespace psvs
{
	paf::math::v4 ScaleColor(int32_t value, int32_t min, int32_t max);
	const wchar_t *UnitsFromSize(int32_t bytes);
	int32_t ValueFromSize(int32_t bytes);
	int32_t LengthOfValue(int32_t num);
}