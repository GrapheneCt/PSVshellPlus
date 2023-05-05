#include <kernel.h>
#include <libdbg.h>
#include <paf.h>

#include "global.h"
#include "utils.h"
#include "psvs.h"
#include "tracker.h"
#include "profile.h"

using namespace paf;

void psvs::Profile::Init()
{
	Dir::Create(PSVS_PROFILES_DIR);
}

psvs::Profile::Profile()
{
	PSVSClockFrequency clock;
	ScePID pid = psvs::tracker::GetCurrentPID();

	psvsGetClockFrequency(&clock);

	sce_paf_memcpy(this->ver, PSVS_VERSION_VER, sizeof(this->ver));

	this->clock[PSVS_OC_DEVICE_CPU] = clock.cpu;
	this->clock[PSVS_OC_DEVICE_GPU_ES4] = clock.gpu;
	this->clock[PSVS_OC_DEVICE_BUS] = clock.bus;
	this->clock[PSVS_OC_DEVICE_GPU_XBAR] = clock.xbar;
	this->lock = PSVS_LOCK_DEVICE_NONE;

	if (psvsClockFrequencyIsLockedProc(pid, PSVS_LOCK_DEVICE_CPU))
		this->lock |= PSVS_LOCK_DEVICE_CPU;
	if (psvsClockFrequencyIsLockedProc(pid, PSVS_LOCK_DEVICE_GPU_ES4))
		this->lock |= PSVS_LOCK_DEVICE_GPU_ES4;
	if (psvsClockFrequencyIsLockedProc(pid, PSVS_LOCK_DEVICE_BUS))
		this->lock |= PSVS_LOCK_DEVICE_BUS;
	if (psvsClockFrequencyIsLockedProc(pid, PSVS_LOCK_DEVICE_GPU_XBAR))
		this->lock |= PSVS_LOCK_DEVICE_GPU_XBAR;
}

psvs::Profile::Profile(const char *path)
{
	LocalFile file;
	LocalFile::OpenArg oarg;

	oarg.filename = path;
	file.Open(&oarg);
	file.Read(this, sizeof(psvs::Profile));
	file.Close();
}

psvs::Profile::~Profile()
{

}

psvs::Profile *psvs::Profile::Load()
{
	string path = PSVS_PROFILES_DIR;
	string path2;
	Profile *prof = NULL;

	common::Utf16ToUtf8(*psvs::tracker::GetCurrentAppName(), &path2);

	path += path2;

	if (LocalFile::Exists(path.c_str()) && LocalFile::GetFileSize(path.c_str()) == sizeof(psvs::Profile)) {
		prof = new Profile(path.c_str());
		if (!sce_paf_strncmp(prof->ver, PSVS_VERSION_VER, sizeof(prof->ver)))
			return prof;
	}

	if (prof)
		delete prof;

	return NULL;
}

void psvs::Profile::Delete()
{
	string path = PSVS_PROFILES_DIR;
	string path2;

	common::Utf16ToUtf8(*psvs::tracker::GetCurrentAppName(), &path2);

	path += path2;

	LocalFile::RemoveFile(path.c_str());
}

void psvs::Profile::Save()
{
	LocalFile file;
	LocalFile::OpenArg oarg;
	string path = PSVS_PROFILES_DIR;
	string path2;

	common::Utf16ToUtf8(*psvs::tracker::GetCurrentAppName(), &path2);

	path += path2;

	oarg.flag = SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC;
	oarg.mode = 0666;
	oarg.filename = path.c_str();
	file.Open(&oarg);
	file.Write(this, sizeof(psvs::Profile));
	file.Close();

}