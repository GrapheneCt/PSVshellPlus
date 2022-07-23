#pragma once

#include <kernel.h>
#include <paf.h>

namespace psvs
{
	SceUInt32 GetHash(const char *name);
	wchar_t *GetString(const char *name);
	paf::Rgba ScaleColor(SceInt32 value, SceInt32 min, SceInt32 max);
	const wchar_t *UnitsFromSize(SceInt32 bytes);
	SceInt32 ValueFromSize(SceInt32 bytes);
	SceInt32 LengthOfValue(SceInt32 num);
}