#include <kernel.h>
#include <libdbg.h>
#include <taihen.h>
#include <paf.h>

#include "utils.h"
#include "psvs.h"
#include "hud.h"
#include "tracker.h"
#include "impose.h"
#include "profile.h"

using namespace paf;

Plugin *g_corePlugin = NULL;
ui::ScrollView *g_imposeRoot = NULL;

static bool s_imposeOpened = false;

extern "C"
{
	SceUID _vshKernelSearchModuleByName(const char *, int *);
}

void corePluginStartCb(Plugin *plugin)
{
	if (!plugin) {
		SCE_DBG_LOG_ERROR("g_corePlugin is NULL\n");
	}

	g_corePlugin = plugin;
}

void coreSetImposeTask(void *arg)
{
	ui::Box *box = (ui::Box *)g_imposeRoot->FindChild(0x0EE0C8AF, 0);
	if (!box)
		return;

	new psvs::Impose(g_corePlugin, box);

	s_imposeOpened = false;

	common::MainThreadCallList::Unregister(coreSetImposeTask, NULL);
}

void coreSetImpose(bool enable)
{
	if (enable && !s_imposeOpened) {
		common::MainThreadCallList::Register(coreSetImposeTask, NULL);
		s_imposeOpened = true;
	}
}

void coreInitImpose()
{
	SCE_DBG_LOG_INFO("coreInitImpose\n");

	//Get power manage plugin object
	Plugin *powerManagePlugin = Plugin::Find("power_manage_plugin");
	if (!powerManagePlugin)
		return;

	//Power manage plugin -> power manage interface
	void *powerIf = powerManagePlugin->GetInterface(1);
	if (!powerIf)
		return;

	//Power manage interface -> impose root
	ui::ScrollView *(*getImposeRoot)();
	getImposeRoot = (ui::ScrollView *(*)()) *(int *)((int)powerIf + 0x54);
	if (!getImposeRoot)
		return;

	//Power manage root -> impose root (some virtual function)
	g_imposeRoot = getImposeRoot();
	if (!g_imposeRoot)
		return;

	SCE_DBG_LOG_INFO("coreInitImpose OK\n");
}

void coreInitPlugin()
{
	SCE_DBG_LOG_INFO("coreInitPlugin\n");

	Plugin::InitParam pluginParam;

	pluginParam.name = "psvshell_plugin";
	pluginParam.resource_file = "ur0:data/PSVshell/psvshell_plugin.rco";
	pluginParam.caller_name = "__main__";
	pluginParam.start_func = corePluginStartCb;

	Plugin::LoadSync(pluginParam);

	psvs::Profile::Init();

	//task::Register(leakTestTask, NULL);
	psvs::tracker::Init();

	SCE_DBG_LOG_INFO("coreInitPlugin OK\n");
}

extern "C" {

	static tai_hook_ref_t s_hookRef[2];
	static SceUID s_hookId[2];

	bool paf_system_SupportsWiredEthernet_patched()
	{
		coreSetImpose(true);
		return TAI_NEXT(paf_system_SupportsWiredEthernet_patched, s_hookRef[1]);
	}

	SceInt32 sceAVConfigRegisterCallback_patched(SceUID cbid, SceInt32 a2)
	{
		coreInitPlugin();
		coreInitImpose();

		s_hookId[1] = taiHookFunctionImport(
			&s_hookRef[1],
			"SceShell",
			0x3D643CE8,
			0xB7CFFF5C,
			paf_system_SupportsWiredEthernet_patched);

		taiHookRelease(s_hookId[0], s_hookRef[0]);

		return TAI_NEXT(sceAVConfigRegisterCallback_patched, s_hookRef[0], cbid, a2);
	}

	int __module_start(SceSize args, const void * argp)
	{
		SceInt32 opt[2];

		SceUID fd = sceIoOpen("ur0:data/PSVshell/psvshell_plugin.rco", SCE_O_RDONLY, 0);
		if (fd <= 0)
			return SCE_KERNEL_START_NO_RESIDENT;

		sceIoClose(fd);

		if (_vshKernelSearchModuleByName("PSVshellPlus_Kernel", opt) <= 0) {
			return SCE_KERNEL_START_NO_RESIDENT;
		}

		s_hookId[0] = taiHookFunctionImport(
			&s_hookRef[0],
			"SceShell",
			0x79E0F03F,
			0xFB5E3E74,
			sceAVConfigRegisterCallback_patched);

		return SCE_KERNEL_START_SUCCESS;
	}

	int __module_stop(SceSize args, const void * argp)
	{
		return SCE_KERNEL_STOP_SUCCESS;
	}

	void __module_exit(void)
	{

	}

}
