#include <kernel.h>
#include <display.h>
#include <sblaimgr.h>
#include <taihen.h>

#include "clock.h"
#include "info.h"
#include "common.h"

SceInt32 g_psvsKpls = 0;

static tai_hook_ref_t s_hookRef[20];
static SceUID s_hookId[20];
static SceUID s_injectId[1];

static SceInt32 s_fpsCounterTarget = -1;

static SceUID s_reqPid = -1;

static SceUInt32 *scePowerKPLSKey = NULL;

int(*scePervasiveArmSetClock)(int mul, int ndiv);

SceVoid psvsSetFpsCounterTarget(SceInt32 target)
{
	s_fpsCounterTarget = target;
}

SceVoid psvsSetClockingPid(SceUID pid)
{
	s_reqPid = pid;
}

SceUID psvsGetClockingPid()
{
	return s_reqPid;
}

//HOOKS
int sceDisplaySetFrameBufInternal_patched(SceDisplayHead head, SceDisplayFrameBufType fb_idx, const SceDisplayFrameBuf *pFrameBuf, SceInt32 iUpdateTimingMode)
{
	if (head != sceDisplayGetPrimaryHead() || !pFrameBuf || !pFrameBuf->base)
		goto DISPLAY_HOOK_RET;

	if (fb_idx == s_fpsCounterTarget)
		psvsCalcFps();

DISPLAY_HOOK_RET:

	return TAI_NEXT(sceDisplaySetFrameBufInternal_patched, s_hookRef[0], head, fb_idx, pFrameBuf, iUpdateTimingMode);
}

int scePowerSetArmClockFrequency_patched(int clock)
{
	SceInt32 ret = SCE_OK;
	SceUID pid = s_reqPid;

	if (!psvsClockFrequencyIsLockedProc(pid, PSVS_LOCK_DEVICE_CPU) || clock > PSVS_CLOCK_MAGIC) {
		if (clock > PSVS_CLOCK_MAGIC)
			clock -= PSVS_CLOCK_MAGIC;

		if (clock == 500) {
			//First set to 444 to disable auto clockdown 
			TAI_NEXT(scePowerSetArmClockFrequency_patched, s_hookRef[5], 444);

			//Write clock to KPLS
			ret = psvsClockFrequencyLockProc(pid, PSVS_500_MHZ_KEY);
			if (ret == SCE_OK) {
				// Apply mul:div (15:0)
				ret = scePervasiveArmSetClock(15, 16 - 0);
			}
			
			return ret;
		}
		else {
			//Write clock to KPLS
			psvsClockFrequencyUnlockProc(pid, PSVS_500_MHZ_KEY);
		}

		ret = TAI_NEXT(scePowerSetArmClockFrequency_patched, s_hookRef[5], clock);
	}
		
	return ret;
}
int scePowerSetBusClockFrequency_patched(int clock)
{
	if (!psvsClockFrequencyIsLockedProc(s_reqPid, PSVS_LOCK_DEVICE_BUS) || clock > PSVS_CLOCK_MAGIC) {
		if (clock > PSVS_CLOCK_MAGIC)
			clock -= PSVS_CLOCK_MAGIC;
		return TAI_NEXT(scePowerSetBusClockFrequency_patched, s_hookRef[6], clock);
	}

	return SCE_OK;
}

int scePowerSetGpuClockFrequencyInternal_patched(int corefreq, int mpfreq)
{
	if (!psvsClockFrequencyIsLockedProc(s_reqPid, PSVS_LOCK_DEVICE_GPU_ES4) || corefreq > PSVS_CLOCK_MAGIC) {
		if (corefreq > PSVS_CLOCK_MAGIC) {
			corefreq -= PSVS_CLOCK_MAGIC;
			mpfreq -= PSVS_CLOCK_MAGIC;
		}

		return TAI_NEXT(scePowerSetGpuClockFrequencyInternal_patched, s_hookRef[7], corefreq, mpfreq);
	}

	return SCE_OK;
}

int scePowerSetGpuXbarClockFrequency_patched(int clock)
{
	if (!psvsClockFrequencyIsLockedProc(s_reqPid, PSVS_LOCK_DEVICE_GPU_XBAR) || clock > PSVS_CLOCK_MAGIC) {
		if (clock > PSVS_CLOCK_MAGIC)
			clock -= PSVS_CLOCK_MAGIC;
		return TAI_NEXT(scePowerSetGpuXbarClockFrequency_patched, s_hookRef[8], clock);
	}

	return SCE_OK;
}

int __sce_aeabi_idiv0()
{
	return 1;
}

long long __sce_aeabi_ldiv0()
{
	return 1;
}

//MAIN
int __module_start(SceSize args, const void * argp)
{
	SceInt32 ret = 0;

	g_psvsKpls = sceKernelRegisterKPLS("PSVS", sizeof(PSVSKPLS));
	if (g_psvsKpls < 0)
		return SCE_KERNEL_START_SUCCESS;

	tai_module_info_t tai_info;
	tai_info.size = sizeof(tai_module_info_t);
	taiGetModuleInfoForKernel(KERNEL_PID, "ScePower", &tai_info);

	module_get_offset(KERNEL_PID, tai_info.modid, 1, 0, (uintptr_t *)&scePowerKPLSKey);

	s_hookId[0] = taiHookFunctionExportForKernel(KERNEL_PID, &s_hookRef[0],
		"SceDisplay", 0x9FED47AC, 0x16466675, sceDisplaySetFrameBufInternal_patched);

	//Hook kernel functions
	s_hookId[5] = taiHookFunctionExportForKernel(KERNEL_PID, &s_hookRef[5],
		"ScePower", 0x1590166F, 0x74DB5AE5, scePowerSetArmClockFrequency_patched);
	s_hookId[6] = taiHookFunctionExportForKernel(KERNEL_PID, &s_hookRef[6],
		"ScePower", 0x1590166F, 0xB8D7B3FB, scePowerSetBusClockFrequency_patched);
	s_hookId[7] = taiHookFunctionExportForKernel(KERNEL_PID, &s_hookRef[7],
		"ScePower", 0x1590166F, 0x264C24FC, scePowerSetGpuClockFrequencyInternal_patched);
	s_hookId[8] = taiHookFunctionExportForKernel(KERNEL_PID, &s_hookRef[8],
		"ScePower", 0x1590166F, 0xA7739DBE, scePowerSetGpuXbarClockFrequency_patched);

	ret = module_get_export_func(KERNEL_PID,
		"SceSysmem", 0x63A519E5, 0x3650963F, (uintptr_t *)&sceKernelGetPhyMemPartInfoCore); // 3.60
	if (ret < 0) {
		module_get_export_func(KERNEL_PID,
			"SceSysmem", 0x02451F0F, 0xB9B69700, (uintptr_t *)&sceKernelGetPhyMemPartInfoCore); // 3.65
	}

	module_get_export_func(KERNEL_PID,
		"SceLowio", 0xE692C727, 0xE9D95643, (uintptr_t *)&scePervasiveArmSetClock);

	/*
	ret = module_get_export_func(KERNEL_PID,
		"SceSysmem", 0x63A519E5, 0x2F6F9C2C, (uintptr_t *)&sceKernelGetPhyMemPartAll); // 3.60
	if (ret < 0) {
		module_get_export_func(KERNEL_PID,
			"SceSysmem", 0x02451F0F, 0x021053DD, (uintptr_t *)&sceKernelGetPhyMemPartAll); // 3.65
	}

	ret = module_get_export_func(KERNEL_PID,
		"SceSysmem", 0x63A519E5, 0xF4FA0575, (uintptr_t *)&sceKernelGetPhyMemPartInfoByID); // 3.60
	if (ret < 0) {
		module_get_export_func(KERNEL_PID,
			"SceSysmem", 0x02451F0F, 0x029F5989, (uintptr_t *)&sceKernelGetPhyMemPartInfoByID); // 3.65
	}

	ret = module_get_export_func(KERNEL_PID,
		"SceSysmem", 0x63A519E5, 0x0FC24464, (uintptr_t *)&sceKernelGUIDGetObject); // 3.60
	if (ret < 0) {
		module_get_export_func(KERNEL_PID,
			"SceSysmem", 0x02451F0F, 0x0FC24464, (uintptr_t *)&sceKernelGUIDGetObject); // 3.65
	}

	ret = module_get_export_func(KERNEL_PID,
		"SceSysmem", 0x63A519E5, 0x4D809B47, (uintptr_t *)&sceKernelGetGamePhyMemPartGrowSize); // 3.60
	if (ret < 0) {
		module_get_export_func(KERNEL_PID,
			"SceSysmem", 0x02451F0F, 0xBC36755F, (uintptr_t *)&sceKernelGetGamePhyMemPartGrowSize); // 3.65
	}

	ret = module_get_export_func(KERNEL_PID,
		"SceSysmem", 0x63A519E5, 0x6B3F4102, (uintptr_t *)&sceKernelGrowPhyMemPart); // 3.60
	if (ret < 0) {
		module_get_export_func(KERNEL_PID,
			"SceSysmem", 0x02451F0F, 0x36FDA794, (uintptr_t *)&sceKernelGrowPhyMemPart); // 3.65
	}

	ret = module_get_export_func(KERNEL_PID,
		"SceSysmem", 0x63A519E5, 0xEEB85560, (uintptr_t *)&sceKernelShrinkPhyMemPart); // 3.60
	if (ret < 0) {
		module_get_export_func(KERNEL_PID,
			"SceSysmem", 0x02451F0F, 0x9B7E673F, (uintptr_t *)&sceKernelShrinkPhyMemPart); // 3.65
	}
	*/

	//Auto clockdown
	/*
	ret = module_get_export_func(KERNEL_PID,
		"SceSysmem", 0x2A5DBD38, 0xEEF091A7, (uintptr_t *)&sceKernelSysrootEnableAutoClockDown); // 3.60
	ret = module_get_export_func(KERNEL_PID,
		"SceSysmem", 0x2A5DBD38, 0xEE934615, (uintptr_t *)&sceKernelSysrootDisableAutoClockDown); // 3.60

	if (ret < 0) {
		module_get_export_func(KERNEL_PID,
			"SceSysmem", 0x2ED7F97A, 0xEEF091A7, (uintptr_t *)&sceKernelSysrootEnableAutoClockDown); // 3.65
		module_get_export_func(KERNEL_PID,
			"SceSysmem", 0x2ED7F97A, 0xEE934615, (uintptr_t *)&sceKernelSysrootDisableAutoClockDown); // 3.65
	}
	*/

	const SceUInt8 nop[] = { 0x00, 0xBF };
	s_injectId[0] = taiInjectAbsForKernel(KERNEL_PID,
		(void *)((uintptr_t)scePervasiveArmSetClock + 0x1D), &nop, 2);

	return SCE_KERNEL_START_SUCCESS;
}

int __module_stop(SceSize args, const void * argp)
{
	return SCE_KERNEL_STOP_SUCCESS;
}