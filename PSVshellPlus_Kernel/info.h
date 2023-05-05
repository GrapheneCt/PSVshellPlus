#pragma once

#include <kernel.h>
#include <scetypes.h>

#include "common.h"

int(*sceKernelGetPhyMemPartInfoCore)(SceKernelPhyMemPart *pPhyMemPart, ScePhyMemPartInfoCore *pInfo);

int(*sceSysconGetBatteryCurrent)(SceInt32 *res);

SceVoid psvsCalcFps();