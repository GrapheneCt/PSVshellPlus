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

Plugin *g_corePlugin = SCE_NULL;
ui::ScrollView *g_imposeRoot = SCE_NULL;
ui::Scene *g_coreRoot = SCE_NULL;

static SceBool s_imposeOpened = SCE_FALSE;

extern "C"
{
	SceUID _vshKernelSearchModuleByName(const char *, int *);
}

SceVoid corePluginStartCb(Plugin *plugin)
{
	if (!plugin) {
		SCE_DBG_LOG_ERROR("g_corePlugin is NULL\n");
	}

	g_corePlugin = plugin;
}

SceVoid coreSetImposeTask(ScePVoid arg)
{
	rco::Element element;
	element.hash = 0x0EE0C8AF;
	ui::Box *box = (ui::Box *)g_imposeRoot->GetChild(&element, 0);
	if (!box)
		return;

	new psvs::Impose(g_corePlugin, box);

	s_imposeOpened = SCE_FALSE;

	task::Unregister(coreSetImposeTask, SCE_NULL);
}

SceVoid coreSetImpose(SceBool enable)
{
	if (enable && !s_imposeOpened) {
		task::Register(coreSetImposeTask, SCE_NULL);
		s_imposeOpened = SCE_TRUE;
	}
}

SceVoid coreInitImpose()
{
	SCE_DBG_LOG_INFO("coreInitImpose\n");

	//Get power manage plugin object
	Plugin *powerManagePlugin = Plugin::Find("power_manage_plugin");
	if (!powerManagePlugin)
		return;

	//Power manage plugin -> power manage interface
	ScePVoid powerIf = powerManagePlugin->GetInterface(1);
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

SceVoid coreInitPlugin()
{
	SCE_DBG_LOG_INFO("coreInitPlugin\n");

	rco::Element element;
	Plugin::PageInitParam pgiParam;
	Plugin::InitParam pluginParam;

	pluginParam.pluginName = "psvshell_plugin";
	pluginParam.resourcePath = "ur0:data/PSVshell/psvshell_plugin.rco";
	pluginParam.scopeName = "__main__";
	pluginParam.pluginStartCB = corePluginStartCb;

	s_frameworkInstance->LoadPlugin(&pluginParam, SCE_NULL, SCE_NULL);

	psvs::Profile::Init();

	//task::Register(leakTestTask, SCE_NULL);
	psvs::tracker::Init();

	pgiParam.priority = 6;
	element.hash = psvs::GetHash("psvs_page_hud");
	g_coreRoot = g_corePlugin->PageOpen(&element, &pgiParam);

	SCE_DBG_LOG_INFO("coreInitPlugin OK\n");
}

extern "C" {

	static tai_hook_ref_t s_hookRef[2];
	static SceUID s_hookId[2];

	bool paf_system_SupportsWiredEthernet_patched()
	{
		coreSetImpose(SCE_TRUE);
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
