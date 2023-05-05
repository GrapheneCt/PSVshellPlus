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

		static void Init();

		static Profile *Load();

		static void Delete();

		Profile();

		Profile(const char *path);

		~Profile();

		void Save();

		char ver[8];
		uint32_t lock;
		int32_t clock[PSVS_OC_DEVICE_MAX];

	private:
	};

}