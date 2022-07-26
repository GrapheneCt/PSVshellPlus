#pragma once

#include <kernel.h>
#include <paf.h>

#include "profile.h"

namespace psvs
{
	namespace tracker
	{
		#define PSVS_APP_UPDATE_WINDOW_USEC  (100000)

		SceVoid Init();
		SceVoid Term();
		SceVoid Update(ScePVoid arg);
		paf::wstring *GetCurrentAppName();
		ScePID GetCurrentPID();
		SceVoid ReloadCurrentProfile();
		psvs::Profile *GetCurrentProfile();
		SceInt32 GetFpsCounterTarget();
		SceInt32 PowerCallback(SceUID notifyId, SceInt32 notifyCount, SceInt32 notifyArg, void *pCommon);
	}
}
