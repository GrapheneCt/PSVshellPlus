#include <kernel.h>

#include "psvs.h"

#define PSVS_PERF_CPU_SAMPLERATE 500 * 1000
#define PSVS_PERF_PEAK_SAMPLES 10

static SceInt32 g_perfPeakUsageSamples[PSVS_PERF_PEAK_SAMPLES] = { 0 };
static SceInt32 g_perfPeakUsageRotation = 0;
static SceInt32 g_perfUsage[4] = { 0, 0, 0, 0 };

static SceUInt32 g_perfTickLast = 0; // AVG CPU load
static SceUInt32 g_perfTickQLast = 0; // Peak CPU load

static SceUInt64 g_perfIdleClockLast[4] = { 0, 0, 0, 0 };
static SceUInt64 g_perfIdleClockQLast[4] = { 0, 0, 0, 0 };

SceVoid psvsCalcCpu()
{
	SceUInt32 tickNow = sceKernelGetProcessTimeLow();
	SceUInt32 tickDiff = tickNow - g_perfTickLast;
	SceUInt32 tickQDiff = tickNow - g_perfTickQLast;

	SceKernelSystemInfo info;
	info.size = sizeof(SceKernelSystemInfo);
	sceKernelGetSystemInfo(&info);

	// Calculate AVG CPU usage
	if (tickDiff >= PSVS_PERF_CPU_SAMPLERATE) {
		for (int i = 0; i < 4; i++) {
			g_perfUsage[i] = (int)(100.0f - ((info.cpuInfo[i].idleClock.quad - g_perfIdleClockLast[i]) / (float)tickDiff) * 100);
			if (g_perfUsage[i] < 0)
				g_perfUsage[i] = 0;
			if (g_perfUsage[i] > 100)
				g_perfUsage[i] = 100;
			g_perfIdleClockLast[i] = info.cpuInfo[i].idleClock.quad;
		}

		g_perfTickLast = tickNow;
	}

	// Calculate peak ST CPU usage
	SceInt32 maxUsage = 0;
	for (int i = 0; i < 4; i++) {
		int usage = (int)(100.0f - ((info.cpuInfo[i].idleClock.quad - g_perfIdleClockQLast[i]) / (float)tickQDiff) * 100);
		if (usage > maxUsage)
			maxUsage = usage;
		g_perfIdleClockQLast[i] = info.cpuInfo[i].idleClock.quad;
	}
	if (maxUsage < 0)
		maxUsage = 0;
	if (maxUsage > 100)
		maxUsage = 100;
	g_perfPeakUsageSamples[g_perfPeakUsageRotation] = maxUsage;
	g_perfPeakUsageRotation++;
	if (g_perfPeakUsageRotation >= PSVS_PERF_PEAK_SAMPLES)
		g_perfPeakUsageRotation = 0; // flip
	g_perfTickQLast = tickNow;
}

SceInt32 psvsGetCpu(PSVSCpu *cpu)
{
	if (!cpu)
		return SCE_KERNEL_ERROR_INVALID_ARGUMENT;

	psvsCalcCpu();

	SceInt32 peakTotal = 0;
	for (int i = 0; i < PSVS_PERF_PEAK_SAMPLES; i++)
		peakTotal += g_perfPeakUsageSamples[i];
	cpu->peak = peakTotal / PSVS_PERF_PEAK_SAMPLES;

	for (int i = 0; i < SCE_KERNEL_MAX_CPU; i++)
		cpu->avg[i] = g_perfUsage[i];
}