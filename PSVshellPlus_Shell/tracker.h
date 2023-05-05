#pragma once

#include <kernel.h>
#include <paf.h>

#include "profile.h"

namespace psvs
{
	namespace tracker
	{
		#define PSVS_APP_UPDATE_WINDOW_USEC  (100000)

		void Init();
		void Term();
		void Update(void *arg);
		paf::wstring *GetCurrentAppName();
		ScePID GetCurrentPID();
		void ReloadCurrentProfile();
		psvs::Profile *GetCurrentProfile();
		int32_t GetFpsCounterTarget();
		SceInt32 PowerCallback(SceUID notifyId, SceInt32 notifyCount, SceInt32 notifyArg, void *pCommon);
	}
}
