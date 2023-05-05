#pragma once

#include <kernel.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PSVS_FPS_COUNTER_TARGET_NONE	-1
#define PSVS_FPS_COUNTER_TARGET_GAME	0
#define PSVS_FPS_COUNTER_TARGET_SHFB	1

typedef enum {
	PSVS_OC_DEVICE_CPU,
	PSVS_OC_DEVICE_GPU_ES4,
	PSVS_OC_DEVICE_BUS,
	PSVS_OC_DEVICE_GPU_XBAR,
	/*PSVS_OC_DEVICE_VENEZIA,
	PSVS_OC_DEVICE_DMAC5,
	PSVS_OC_DEVICE_COMPAT,
	PSVS_OC_DEVICE_VIP,
	PSVS_OC_DEVICE_SYS,*/
	PSVS_OC_DEVICE_MAX
} PsvsOcDevice;

typedef enum {
	PSVS_LOCK_DEVICE_NONE = 0,
	PSVS_LOCK_DEVICE_CPU = 1,
	PSVS_LOCK_DEVICE_GPU_ES4 = 2,
	PSVS_LOCK_DEVICE_BUS = 4,
	PSVS_LOCK_DEVICE_GPU_XBAR = 8,
	/*PSVS_LOCK_DEVICE_VENEZIA = 16,
	PSVS_LOCK_DEVICE_DMAC5 = 32,
	PSVS_LOCK_DEVICE_COMPAT = 64,
	PSVS_LOCK_DEVICE_VIP = 128,
	PSVS_LOCK_DEVICE_SYS = 256,*/
	PSVS_LOCK_DEVICE_ALL = PSVS_LOCK_DEVICE_CPU | PSVS_LOCK_DEVICE_GPU_ES4 | PSVS_LOCK_DEVICE_BUS | PSVS_LOCK_DEVICE_GPU_XBAR
} PsvsLockDevice;

typedef struct PSVSClockFrequency {
	SceInt32 cpu;
	SceInt32 gpu;
	SceInt32 xbar;
	SceInt32 bus;
} PSVSClockFrequency;

typedef struct PSVSMem {
	SceUInt32 mainFree;
	SceUInt32 mainTotal;
	SceUInt32 cdramFree;
	SceUInt32 cdramTotal;
	SceUInt32 phycontFree;
	SceUInt32 phycontTotal;
	SceUInt32 cdialogFree;
	SceUInt32 cdialogTotal;
} PSVSMem;

typedef struct PSVSVenezia {
	SceInt32 core0;
	SceInt32 core1;
	SceInt32 core2;
	SceInt32 core3;
	SceInt32 core4;
	SceInt32 core5;
	SceInt32 core6;
	SceInt32 core7;
	SceInt32 average;
	SceInt32 peak;
} PSVSVenezia;

typedef struct PSVSBattery {
	SceInt32 current;
} PSVSBattery;

typedef struct PSVSCpu {
	SceInt32 avg[SCE_KERNEL_MAX_CPU];
	SceInt32 peak;
} PSVSCpu;

SceInt32 psvsSetArmClockFrequency(SceInt32 clock);
SceInt32 psvsSetGpuClockFrequency(SceInt32 clock);
SceInt32 psvsSetGpuXbarClockFrequency(SceInt32 clock);
SceInt32 psvsSetBusClockFrequency(SceInt32 clock);
SceInt32 psvsGetClockFrequency(PSVSClockFrequency *clocks);
SceInt32 psvsGetFps();
SceInt32 psvsGetMem(SceUInt32 casShift, PSVSMem *mem);
SceVoid psvsSetFpsCounterTarget(SceInt32 target);
SceInt32 psvsClockFrequencyLockProc(ScePID pid, PsvsLockDevice type);
SceInt32 psvsClockFrequencyUnlockProc(ScePID pid, PsvsLockDevice type);
SceBool psvsClockFrequencyIsLockedProc(ScePID pid, PsvsLockDevice type);
SceInt32 psvsSetRecommendedCasShift(char *name, SceInt32 namelen);
SceVoid psvsSetClockingPid(SceUID pid);
SceInt32 psvsGetVeneziaInfo(PSVSVenezia *data);
SceInt32 psvsGetBatteryInfo(PSVSBattery *data);

SceVoid psvsCalcCpu();
SceInt32 psvsGetCpu(PSVSCpu *cpu);

int _sceCodecEnginePmonStop();
int _sceCodecEnginePmonReset();
int _sceCodecEnginePmonStart();
#define sceCodecEnginePmonStop _sceCodecEnginePmonStop
#define sceCodecEnginePmonReset _sceCodecEnginePmonReset
#define sceCodecEnginePmonStart _sceCodecEnginePmonStart

#ifdef __cplusplus
}
#endif
