#pragma once

#include <kernel.h>
#include <scetypes.h>

#include "common.h"

int(*sceKernelGetPhyMemPartInfoCore)(SceKernelPhyMemPart *pPhyMemPart, ScePhyMemPartInfoCore *pInfo);

SceVoid psvsCalcFps();