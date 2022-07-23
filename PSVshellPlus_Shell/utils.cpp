#include <kernel.h>
#include <paf.h>

#include "global.h"

using namespace paf;

namespace psvs
{
	SceUInt32 GetHash(const char *name)
	{
		string searchRequest;
		rco::Element searchResult;

		searchRequest = name;
		searchResult.hash = searchResult.GetHash(&searchRequest);

		return searchResult.hash;
	}

	wchar_t *GetString(const char *name)
	{
		rco::Element searchRequest;

		searchRequest.hash = GetHash(name);
		wchar_t *res = (wchar_t *)g_corePlugin->GetWString(&searchRequest);

		return res;
	}

	Rgba ScaleColor(SceInt32 value, SceInt32 min, SceInt32 max)
	{
		if (value < min)
			value = min;
		if (value > max)
			value = max;

		SceInt32 v = ((SceFloat32)(value - min) / (max - min)) * 255; // 0-255

		SceInt32 r = (v <= 127) ? v * 2 : 255;
		SceInt32 g = (v > 127) ? 255 - ((v - 128) * 2) : 255;

		return Rgba((SceFloat32)r / 255.0f, (SceFloat32)g / 255.0f, 0.0f, 1.0f);
	}

	const wchar_t *UnitsFromSize(SceInt32 bytes)
	{
		if (bytes >= 1024 * 1024)
			return L"MB";
		else if (bytes >= 1024)
			return L"kB";
		else
			return L" B";
	}

	SceInt32 ValueFromSize(SceInt32 bytes)
	{
		if (bytes >= 1024 * 1024)
			return bytes / 1024 / 1024;
		else if (bytes >= 1024)
			return bytes / 1024;
		else
			return bytes;
	}

	SceInt32 LengthOfValue(SceInt32 num)
	{
		SceInt32 len = 3;

		if (num < 10)
			len = 1;
		else if (num < 100)
			len = 2;

		return len;
	}
}