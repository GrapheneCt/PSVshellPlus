#include <kernel.h>
#include <string.h>
#include <taihen.h>
#include <sce_atomic.h>

#include "info.h"
#include "common.h"

#define PSVS_CHECK_ASSIGN(struct, field, new_value) \
    struct.field = (new_value)

#define PSVS_PERF_FPS_SAMPLES 5
#define SECOND 1000000

static SceUInt32 s_perfTickFpsLast = 0;
static SceUInt32 g_perfFrametimeSum = 0;
static SceUInt8 g_perfFrametimeN = 0;
static SceInt32 s_perfFps = 0;
static SceInt32 s_casShift = 0;

int(*_sceVeneziaGetProcessorLoad)(PSVSVenezia *data) = NULL;

SceInt32 psvsGetFps()
{
	return sceAtomicLoad32AcqRel(&s_perfFps);
}

SceInt32 psvsSetRecommendedCasShift(char *name, SceInt32 namelen)
{
	char appName[10];
	SceKernelPhyMemPart *memPart = NULL;

	if (namelen > sizeof(appName) || name == NULL)
		return SCE_KERNEL_ERROR_INVALID_ARGUMENT;

	sceKernelStrncpyFromUser(appName, name, namelen);

	SceUInt32 sysrootCas = sceKernelSysrootGetCurrentAddressSpaceCB();
	SceUInt32 sysrootCasSearch = sysrootCas;
	SceInt32 shift = 0;
	SceUInt32 searchType = -1;
	SceInt32 iAppName = *(SceInt32 *)appName;

	if (iAppName == 0x6E69616D) { // "main"
		searchType = SCE_KERNEL_PARTITION_MEMTYPE_MAIN | SCE_KERNEL_PARTITION_OWNER_SYSTEM;
	}
	else if (iAppName == 0x656D6167) { // "game"
		searchType = SCE_KERNEL_PARTITION_MEMTYPE_MAIN | SCE_KERNEL_PARTITION_OWNER_GAME;
	}

	for (int i = 0; i < 7; i++)
	{
		sysrootCasSearch = sysrootCas;
		shift = 0x1000 * i;
		sysrootCasSearch += shift;

		if (*(SceKernelPhyMemPart **)(sysrootCasSearch + 328) < 0x1000000 && *(SceKernelPhyMemPart **)(sysrootCasSearch + 328) > 0x1000) {
			memPart = *(SceKernelPhyMemPart **)(sysrootCasSearch + 328);
			if (searchType != -1) {
				if (memPart->type == searchType)
					break;
			}
			else {
				if (strstr(memPart->name, appName))
					break;
			}
		}
	}

	s_casShift = shift;

	return SCE_OK;
}

SceInt32 psvsGetMem(SceInt32 casShift, PSVSMem *mem)
{
	if (!mem)
		return SCE_KERNEL_ERROR_INVALID_ARGUMENT;

	PSVSMem kmemUsage;
	ScePhyMemPartInfoCore info;
	SceUInt32 sysrootCas = sceKernelSysrootGetCurrentAddressSpaceCB();

	if (casShift < 0) {
		sysrootCas += s_casShift;
	}
	else {
		sysrootCas += casShift;
	}

	if (*(SceKernelPhyMemPart **)(sysrootCas + 328) < 0x1000000) {

		if (*(SceKernelPhyMemPart **)(sysrootCas + 328) > 0x1000) {
			sceKernelGetPhyMemPartInfoCore(*(SceKernelPhyMemPart **)(sysrootCas + 328), &info);
			PSVS_CHECK_ASSIGN(kmemUsage, mainFree, info.freeSize);
			PSVS_CHECK_ASSIGN(kmemUsage, mainTotal, info.totalSize);
		}
		else {
			PSVS_CHECK_ASSIGN(kmemUsage, mainFree, 0);
			PSVS_CHECK_ASSIGN(kmemUsage, mainTotal, 0);
		}

		if (*(SceKernelPhyMemPart **)(sysrootCas + 332) > 0x1000) {
			sceKernelGetPhyMemPartInfoCore(*(SceKernelPhyMemPart **)(sysrootCas + 332), &info);
			PSVS_CHECK_ASSIGN(kmemUsage, cdramFree, info.freeSize);
			PSVS_CHECK_ASSIGN(kmemUsage, cdramTotal, info.totalSize);
		}
		else {
			PSVS_CHECK_ASSIGN(kmemUsage, cdramFree, 0);
			PSVS_CHECK_ASSIGN(kmemUsage, cdramTotal, 0);
		}

		if (*(SceKernelPhyMemPart **)(sysrootCas + 316) > 0x1000) {
			sceKernelGetPhyMemPartInfoCore(*(SceKernelPhyMemPart **)(sysrootCas + 316), &info);
			PSVS_CHECK_ASSIGN(kmemUsage, phycontFree, info.freeSize);
			PSVS_CHECK_ASSIGN(kmemUsage, phycontTotal, info.totalSize);
		}
		else {
			PSVS_CHECK_ASSIGN(kmemUsage, phycontFree, 0);
			PSVS_CHECK_ASSIGN(kmemUsage, phycontTotal, 0);
		}

		if (*(SceKernelPhyMemPart **)(sysrootCas + 324) > 0x1000) {
			sceKernelGetPhyMemPartInfoCore(*(SceKernelPhyMemPart **)(sysrootCas + 324), &info);
			PSVS_CHECK_ASSIGN(kmemUsage, cdialogFree, info.freeSize);
			PSVS_CHECK_ASSIGN(kmemUsage, cdialogTotal, info.totalSize);
		}
		else {
			PSVS_CHECK_ASSIGN(kmemUsage, cdialogFree, 0);
			PSVS_CHECK_ASSIGN(kmemUsage, cdialogTotal, 0);
		}
	}
	else {
		PSVS_CHECK_ASSIGN(kmemUsage, mainFree, 0);
		PSVS_CHECK_ASSIGN(kmemUsage, mainTotal, 0);
		PSVS_CHECK_ASSIGN(kmemUsage, cdramFree, 0);
		PSVS_CHECK_ASSIGN(kmemUsage, cdramTotal, 0);
		PSVS_CHECK_ASSIGN(kmemUsage, phycontFree, 0);
		PSVS_CHECK_ASSIGN(kmemUsage, phycontTotal, 0);
		PSVS_CHECK_ASSIGN(kmemUsage, cdialogFree, 0);
		PSVS_CHECK_ASSIGN(kmemUsage, cdialogTotal, 0);
	}

	return sceKernelCopyToUser(mem, &kmemUsage, sizeof(PSVSMem));
}

SceVoid psvsCalcFps()
{
	SceUInt32 tickNow = sceKernelGetProcessTimeLow();
	SceUInt32 frametime = tickNow - s_perfTickFpsLast;

	// Update FPS when enough samples are collected
	if (g_perfFrametimeN > PSVS_PERF_FPS_SAMPLES) {
		SceUInt32 frametimeAvg = g_perfFrametimeSum / g_perfFrametimeN;
		sceAtomicStore32AcqRel(&s_perfFps, (SECOND + (frametimeAvg / 2) + 1) / frametimeAvg);
		g_perfFrametimeN = 0;
		g_perfFrametimeSum = 0;
	}

	g_perfFrametimeN++;
	g_perfFrametimeSum += frametime;
	s_perfTickFpsLast = tickNow;
}

SceInt32 psvsGetVeneziaInfo(PSVSVenezia *data)
{
	PSVSVenezia kdata;
	SceInt32 ret = SCE_KERNEL_ERROR_UNSUP;

	if (!data)
		return SCE_KERNEL_ERROR_INVALID_ARGUMENT;

	if (!_sceVeneziaGetProcessorLoad) {
		tai_module_info_t info;
		memset(&info, 0, sizeof(tai_module_info_t));
		info.size = sizeof(tai_module_info_t);
		taiGetModuleInfoForKernel(KERNEL_PID, "SceCodecEngineWrapper", &info);
		module_get_offset(KERNEL_PID, info.modid, 0, 0x36EC | 1, (uintptr_t *)&_sceVeneziaGetProcessorLoad);
	}

	if (_sceVeneziaGetProcessorLoad)
		ret = _sceVeneziaGetProcessorLoad(&kdata);

	sceKernelCopyToUser(data, &kdata, sizeof(PSVSVenezia));

	return ret;
}