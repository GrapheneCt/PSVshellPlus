#include <kernel.h>
#include <appmgr.h>
#include <power.h>
#include <libdbg.h>
#include <paf.h>

#include "global.h"
#include "utils.h"
#include "impose.h"
#include "hud.h"
#include "psvs.h"
#include "tracker.h"
#include "profile.h"

using namespace paf;

namespace psvs
{
namespace tracker
{
	static int32_t s_currAppId = -1;
	static ScePID s_currPid = -1;
	static wstring *s_currAppName = NULL;
	static uint32_t s_tickOld = 0;
	static int32_t s_fpsTarget = PSVS_FPS_COUNTER_TARGET_SHFB;
	static psvs::Profile *s_currProfile = NULL;

	void Update(void *arg)
	{
		char name[32];
		int32_t appId = -1;
		uint32_t tickNow = sceKernelGetProcessTimeLow();
		bool appChanged = false;

		if (tickNow - s_tickOld > PSVS_APP_UPDATE_WINDOW_USEC) {
			appId = sceAppMgrGetAppIdByAppId(SCE_APPMGR_APP_ID_ACTIVE);
			if (appId > 0) {
				if (s_currAppId != appId) {
					s_currPid = sceAppMgrGetProcessIdByAppIdForShell(appId);
					sceAppMgrGetNameById(sceAppMgrGetProcessIdByAppIdForShell(appId), name);
					common::Utf8ToUtf16(name, s_currAppName);
					if (sceAppMgrGetAppIdByAppId(SCE_APPMGR_APP_ID_GAME) != appId) {
						s_fpsTarget = PSVS_FPS_COUNTER_TARGET_SHFB;
						psvsSetRecommendedCasShift(name, sce_paf_strlen(name) + 1);
					}
					else {
						s_fpsTarget = PSVS_FPS_COUNTER_TARGET_GAME;
						psvsSetRecommendedCasShift("game", sizeof("game"));
					}
					s_currAppId = appId;
					appChanged = true;
				}
			}
			else {
				if (s_currAppId != -1) {
					s_currPid = sceKernelGetProcessId();
					*s_currAppName = L"main";
					psvsSetRecommendedCasShift("main", sizeof("main"));
					s_fpsTarget = PSVS_FPS_COUNTER_TARGET_SHFB;
					s_currAppId = -1;
					appChanged = true;
				}
			}

			if (appChanged) {
				psvsSetClockingPid(s_currPid);
				if (s_currProfile)
					delete s_currProfile;
				s_currProfile = psvs::Profile::Load();
				if (s_currProfile) {
					psvsSetArmClockFrequency(s_currProfile->clock[PSVS_OC_DEVICE_CPU]);
					psvsSetGpuClockFrequency(s_currProfile->clock[PSVS_OC_DEVICE_GPU_ES4]);
					//psvsSetBusClockFrequency(s_currProfile->clock[PSVS_OC_DEVICE_BUS]);
					psvsSetGpuXbarClockFrequency(s_currProfile->clock[PSVS_OC_DEVICE_GPU_XBAR]);
					psvsClockFrequencyLockProc(s_currPid, (PsvsLockDevice)s_currProfile->lock);
				}
				else {
					psvsClockFrequencyUnlockProc(s_currPid, PSVS_LOCK_DEVICE_ALL);
				}
			}

			s_tickOld = tickNow;
		}
	}

	wstring *GetCurrentAppName()
	{
		return s_currAppName;
	}

	ScePID GetCurrentPID()
	{
		return s_currPid;
	}

	void ReloadCurrentProfile()
	{
		s_currProfile = psvs::Profile::Load();
	}

	psvs::Profile *GetCurrentProfile()
	{
		return s_currProfile;
	}

	int32_t GetFpsCounterTarget()
	{
		return s_fpsTarget;
	}

	SceInt32 PowerCallback(SceUID notifyId, SceInt32 notifyCount, SceInt32 notifyArg, void *pCommon)
	{
		if ((notifyArg & SCE_POWER_CALLBACKARG_RESERVED_23) == SCE_POWER_CALLBACKARG_RESERVED_23) {
			// Set clocks after resume reset
			PSVSClockFrequency clocks;
			psvsGetClockFrequency(&clocks);
			psvsSetArmClockFrequency(clocks.cpu);
			psvsSetGpuClockFrequency(clocks.gpu);
			//psvsSetBusClockFrequency(clocks.bus);
			psvsSetGpuXbarClockFrequency(clocks.xbar);

			// Prevent HUD from blocking peel screen
			if (!SCE_PAF_IS_DOLCE && Impose::GetHudPosition() == Hud::Position_UpRight && Hud::GetCurrentHud() != SCE_NULL) {
				Impose::SetHudPosition(Hud::Position_UpLeft);
			}
		}

		return SCE_OK;
	}

	void Init()
	{
		s_currPid = sceKernelGetProcessId();
		s_currAppName = new wstring(L"main");
		psvsSetClockingPid(s_currPid);
		s_currProfile = psvs::Profile::Load();
		if (s_currProfile) {
			psvsSetArmClockFrequency(s_currProfile->clock[PSVS_OC_DEVICE_CPU]);
			psvsSetGpuClockFrequency(s_currProfile->clock[PSVS_OC_DEVICE_GPU_ES4]);
			//psvsSetBusClockFrequency(s_currProfile->clock[PSVS_OC_DEVICE_BUS]);
			psvsSetGpuXbarClockFrequency(s_currProfile->clock[PSVS_OC_DEVICE_GPU_XBAR]);
			psvsClockFrequencyLockProc(s_currPid, (PsvsLockDevice)s_currProfile->lock);
		}
		else {
			psvsClockFrequencyUnlockProc(s_currPid, PSVS_LOCK_DEVICE_ALL);
		}

		SceUID cbid = sceKernelCreateCallback("PSVshellPlus_PCB", 0, PowerCallback, NULL);
		scePowerRegisterCallback(cbid);

		common::MainThreadCallList::Register(Update, NULL);
	}

	void Term()
	{
		common::MainThreadCallList::Unregister(Update, NULL);
		delete s_currAppName;
	}
}
}