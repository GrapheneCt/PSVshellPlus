#pragma once

#include <kernel.h>
#include <paf.h>

#include "psvs.h"

#define PSVS_PROFILES_DIR "ur0:data/PSVshell/profiles/"
#define PSVS_VERSION_VER    "PSVS0300"

namespace psvs
{
	class Profile
	{
	public:

		static SceVoid Init();

		static Profile *Load();

		static SceVoid Delete();

		Profile();

		Profile(const char *path);

		~Profile();

		SceVoid Save();

		char ver[8];
		SceUInt32 lock;
		SceInt32 clock[PSVS_OC_DEVICE_MAX];

	private:
	};

}