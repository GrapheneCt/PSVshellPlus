#include <kernel.h>
#include <taihen.h>

#include "clock.h"
#include "common.h"

#define PSVS_LOCKED_PID_MAX_COUNT	16

int scePowerSetArmClockFrequency(int clock);
int scePowerSetGpuClockFrequencyInternal(int corefreq, int mpfreq);
int scePowerSetGpuXbarClockFrequency(int clock);
int scePowerSetBusClockFrequency(int clock);

int scePowerGetArmClockFrequencyProc(ScePID pid);
int scePowerGetGpuClockFrequencyProc(ScePID pid);
int scePowerGetGpuXbarClockFrequencyProc(ScePID pid);
int scePowerGetBusClockFrequencyProc(ScePID pid);

int scePowerGetArmClockFrequency(void);
int scePowerGetGpuClockFrequencyInternal(SceInt32 *corefreq, SceInt32 *mpfreq);
int scePowerGetBusClockFrequency(void);
int scePowerGetGpuXbarClockFrequency(void);

SceInt32 psvsClockFrequencyLockProc(ScePID pid, PsvsLockDevice type)
{
	PSVSKPLS *kpls = NULL;
	SceInt32 ret = sceKernelGetRemoteKPLS(pid, g_psvsKpls, &kpls, SCE_TRUE);
	if (ret == SCE_OK && kpls != NULL) {
		kpls->lock |= type;
	}

	return ret;
}

SceInt32 psvsClockFrequencyUnlockProc(ScePID pid, PsvsLockDevice type)
{
	PSVSKPLS *kpls = NULL;
	SceInt32 ret = sceKernelGetRemoteKPLS(pid, g_psvsKpls, &kpls, SCE_TRUE);
	if (ret == SCE_OK && kpls != NULL) {
		kpls->lock &= ~type;
	}

	return ret;
}

SceBool psvsClockFrequencyIsLockedProc(ScePID pid, PsvsLockDevice type)
{
	PSVSKPLS *kpls = NULL;
	SceInt32 ret = sceKernelGetRemoteKPLS(pid, g_psvsKpls, &kpls, SCE_TRUE);
	if (ret == SCE_OK && kpls != NULL) {
		return kpls->lock & type;
	}

	return SCE_FALSE;
}

SceInt32 psvsSetArmClockFrequency(SceInt32 clock)
{
	SYSCALL_ENTER
	clock += PSVS_CLOCK_MAGIC;
	SceInt32 ret = scePowerSetArmClockFrequency(clock);
	SYSCALL_EXIT
	return ret;
}

SceInt32 psvsSetGpuClockFrequency(SceInt32 clock)
{
	SYSCALL_ENTER
	clock += PSVS_CLOCK_MAGIC;
	SceInt32 corefreq = clock;
	SceInt32 mpfreq = clock;
	SceInt32 ret = scePowerSetGpuClockFrequencyInternal(corefreq, mpfreq);
	SYSCALL_EXIT
	return ret;
}

SceInt32 psvsSetGpuXbarClockFrequency(SceInt32 clock)
{
	SYSCALL_ENTER
	clock += PSVS_CLOCK_MAGIC;
	SceInt32 ret = scePowerSetGpuXbarClockFrequency(clock);
	SYSCALL_EXIT
	return ret;
}

SceInt32 psvsSetBusClockFrequency(SceInt32 clock)
{
	SYSCALL_ENTER
	clock += PSVS_CLOCK_MAGIC;
	SceInt32 ret = scePowerSetBusClockFrequency(clock);
	SYSCALL_EXIT
	return ret;
}

SceInt32 psvsGetClockFrequency(PSVSClockFrequency *clocks)
{
	SceInt32 ret = 0;
	SceInt32 unused;
	if (!clocks)
		return SCE_KERNEL_ERROR_INVALID_ARGUMENT;

	SYSCALL_ENTER

	PSVSClockFrequency kclocks;
	kclocks.cpu = scePowerGetArmClockFrequency();
	if (psvsClockFrequencyIsLockedProc(psvsGetClockingPid(), PSVS_500_MHZ_KEY))
		kclocks.cpu = 500;
	scePowerGetGpuClockFrequencyInternal(&kclocks.gpu, &unused);
	kclocks.xbar = scePowerGetGpuXbarClockFrequency();
	kclocks.bus = scePowerGetBusClockFrequency();

	ret = sceKernelCopyToUser(clocks, &kclocks, sizeof(PSVSClockFrequency));

	SYSCALL_EXIT
	return ret;
}