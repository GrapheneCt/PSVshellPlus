#include <kernel.h>
#include <libdbg.h>
#include <paf.h>

#include "global.h"
#include "utils.h"
#include "psvs.h"
#include "hud.h"
#include "tracker.h"

using namespace paf;

namespace psvs
{
	static psvs::Hud *s_currentHud = NULL;
	static int32_t s_casShift = -1;

	Hud *Hud::GetCurrentHud()
	{
		return s_currentHud;
	}

	void Hud::SetCasShift(int32_t shift)
	{
		s_casShift = shift;
	}

	Hud::Hud(BaseType type)
	{
		Plugin::TemplateOpenParam tmpParam;
		Plugin::PageOpenParam pgiParam;

		pgiParam.overwrite_draw_priority = 6;
		coreRoot = g_corePlugin->PageOpen("psvs_page_hud", pgiParam);

		switch (type) {
		case BaseType_Mini:
			g_corePlugin->TemplateOpen(coreRoot, psvs_template_hud_bg_mini, tmpParam);
			this->root = coreRoot->GetChild(coreRoot->GetChildrenNum() - 1);
			this->root->Show(common::transition::Type_Fadein1);
			break;
		case BaseType_Full:
			g_corePlugin->TemplateOpen(coreRoot, psvs_template_hud_bg_full, tmpParam);
			this->root = coreRoot->GetChild(coreRoot->GetChildrenNum() - 1);
			this->root->Show(common::transition::Type_Fadein1);
			break;
		case BaseType_Dev:
			g_corePlugin->TemplateOpen(coreRoot, psvs_template_hud_bg_dev, tmpParam);
			this->root = coreRoot->GetChild(coreRoot->GetChildrenNum() - 1);
			this->root->Show(common::transition::Type_Fadein1);
			break;
		default:
			break;
		}

		psvsSetFpsCounterTarget(tracker::GetFpsCounterTarget());
	}

	Hud::~Hud()
	{
		Plugin::PageCloseParam pgcParam;

		psvsSetFpsCounterTarget(PSVS_FPS_COUNTER_TARGET_NONE);

		pgcParam.fade = true;
		g_corePlugin->PageClose("psvs_page_hud", pgcParam);

		s_currentHud = NULL;
	}

	HudMicro::HudMicro() : Hud::Hud(Hud::BaseType_None)
	{
		Plugin::TemplateOpenParam tmpParam;

		this->oldFps = 0;

		g_corePlugin->TemplateOpen(coreRoot, psvs_template_hud_micro, tmpParam);
		this->fps = (ui::Text *)coreRoot->FindChild(psvs_text_fps_micro);
		this->root = this->fps;
		common::MainThreadCallList::Register(Update, this);
		s_currentHud = this;
	}

	HudMicro::~HudMicro()
	{
		common::MainThreadCallList::Unregister(Update, this);
	}

	void HudMicro::SetPosition(Position pos)
	{
		math::v4 vec(0.0f);

		switch (pos) {
		case Position_UpLeft:
			vec.set_x(-920.0f);
			vec.set_y(-10.0f);
			break;
		case Position_UpRight:
			vec.set_x(-16.0f);
			vec.set_y(-10.0f);
			break;
		case Position_DownLeft:
			vec.set_x(-920.0f);
			vec.set_y(-520.0f);
			break;
		case Position_DownRight:
			vec.set_x(-16.0f);
			vec.set_y(-520.0f);
			break;
		}

		this->root->SetPos(vec);
	}

	void HudMicro::Update(void *arg)
	{
		wstring wtext;
		wchar_t wbuf[6];
		HudMicro *obj = (HudMicro *)arg;

		//Update FPS
		psvsSetFpsCounterTarget(tracker::GetFpsCounterTarget());
		int32_t fps = psvsGetFps();

		if (fps != obj->oldFps) {
			sce_paf_swprintf(wbuf, 6, L"%d", fps);
			wtext = wbuf;
			obj->fps->SetString(wtext);
		}

		obj->oldFps = fps;
	}

	HudMini::HudMini(Hud::BaseType type) : Hud::Hud(type)
	{
		Plugin::TemplateOpenParam tmpParam;

		this->oldFps = 0;

		g_corePlugin->TemplateOpen(this->root, psvs_template_hud_mini, tmpParam);

		this->cpuAvg[0] = (ui::Text *)this->root->FindChild(psvs_text_cpu_0);
		this->cpuAvg[1] = (ui::Text *)this->root->FindChild(psvs_text_cpu_1);
		this->cpuAvg[2] = (ui::Text *)this->root->FindChild(psvs_text_cpu_2);
		this->cpuAvg[3] = (ui::Text *)this->root->FindChild(psvs_text_cpu_3);
		this->cpuPeak = (ui::Text *)this->root->FindChild(psvs_text_cpu_peak);
		this->fps = (ui::Text *)this->root->FindChild(psvs_text_fps);

		common::MainThreadCallList::Register(Update, this);

		s_currentHud = this;
	}

	HudMini::~HudMini()
	{
		common::MainThreadCallList::Unregister(Update, this);
	}

	void HudMini::SetPosition(Position pos)
	{
		math::v4 vec(0.0f);

		switch (pos) {
		case Position_UpLeft:
			vec.set_x(10.0f);
			vec.set_y(-10.0f);
			break;
		case Position_UpRight:
			vec.set_x(600.0f);
			vec.set_y(-10.0f);
			break;
		case Position_DownLeft:
			vec.set_x(10.0f);
			vec.set_y(-470.0f);
			break;
		case Position_DownRight:
			vec.set_x(600.0f);
			vec.set_y(-470.0f);
			break;
		}

		this->root->SetPos(vec);
	}

	void HudMini::Update(void *arg)
	{
		wstring wtext;
		wchar_t wbuf[16];
		PSVSCpu cpu;
		HudMini *obj = (HudMini *)arg;

		//Update FPS
		psvsSetFpsCounterTarget(tracker::GetFpsCounterTarget());
		int32_t fps = psvsGetFps();

		if (fps != obj->oldFps) {
			sce_paf_swprintf(wbuf, 16, L" %d FPS", fps);
			wtext = wbuf;
			obj->fps->SetString(wtext);
			//obj->fps->SetColor(ui::Text::ColorType_Text, 1, 2, &ScaleColor(30 - fps, 0, 30));
		}

		obj->oldFps = fps;

		//Update CPU
		psvsGetCpu(&cpu);

		for (int i = 0; i < SCE_KERNEL_MAX_CPU; i++) {
			sce_paf_swprintf(wbuf, 16, L" %d%%", cpu.avg[i]);
			wtext = wbuf;
			obj->cpuAvg[i]->SetString(wtext);
			//obj->cpuAvg[i]->SetColor(ui::Text::ColorType_Text, 1, LengthOfValue(cpu.avg[i]), &ScaleColor(cpu.avg[i], 0, 100));
		}

		sce_paf_swprintf(wbuf, 16, L" %d%%", cpu.peak);
		wtext = wbuf;
		obj->cpuPeak->SetString(wtext);
		//obj->cpuPeak->SetColor(ui::Text::ColorType_Text, 1, LengthOfValue(cpu.peak), &ScaleColor(cpu.peak, 0, 100));
	}

	HudFull::HudFull() : HudMini::HudMini(Hud::BaseType_Full)
	{
		Plugin::TemplateOpenParam tmpParam;

		this->oldMem.mainTotal = __INT_MAX__;
		this->oldMem.cdramTotal = __INT_MAX__;
		this->oldMem.phycontTotal = __INT_MAX__;
		this->oldMem.cdialogTotal = __INT_MAX__;
		this->memTick = 0;

		g_corePlugin->TemplateOpen(this->root, psvs_template_hud_full, tmpParam);

		this->ramUsed = (ui::Text *)this->root->FindChild(psvs_text_ram_used);
		this->cdramUsed = (ui::Text *)this->root->FindChild(psvs_text_cdram_used);
		this->phyUsed = (ui::Text *)this->root->FindChild(psvs_text_phy_used);
		this->cdlgUsed = (ui::Text *)this->root->FindChild(psvs_text_cdlg_used);
		this->ramTotal = (ui::Text *)this->root->FindChild(psvs_text_ram_total);
		this->cdramTotal = (ui::Text *)this->root->FindChild(psvs_text_cdram_total);
		this->phyTotal = (ui::Text *)this->root->FindChild(psvs_text_phy_total);
		this->cdlgTotal = (ui::Text *)this->root->FindChild(psvs_text_cdlg_total);
		this->app = (ui::Text *)this->root->FindChild(psvs_text_app);

		common::MainThreadCallList::Register(Update, this);

		s_currentHud = this;
	}

	HudFull::~HudFull()
	{
		common::MainThreadCallList::Unregister(Update, this);
	}

	void HudFull::SetPosition(Position pos)
	{
		math::v4 vec(0.0f);

		switch (pos) {
		case Position_UpLeft:
			vec.set_x(10.0f);
			vec.set_y(-10.0f);
			break;
		case Position_UpRight:
			vec.set_x(600.0f);
			vec.set_y(-10.0f);
			break;
		case Position_DownLeft:
			vec.set_x(10.0f);
			vec.set_y(-334.0f);
			break;
		case Position_DownRight:
			vec.set_x(600.0f);
			vec.set_y(-334.0f);
			break;
		}

		this->root->SetPos(vec);
	}

	void HudFull::Update(void *arg)
	{
		wstring wtext;
		PSVSMem mem;
		uint32_t tickNow = sceKernelGetProcessTimeLow();
		HudFull *obj = (HudFull *)arg;

		if (tickNow - obj->memTick > PSVS_FULL_UPDATE_WINDOW_USEC) {
			//Update memory
			psvsGetMem(s_casShift, &mem);

			if (obj->oldMem.mainTotal != mem.mainTotal || obj->oldMem.mainFree != mem.mainFree)
				SetMemLabel(mem.mainTotal - mem.mainFree, mem.mainTotal, obj->ramUsed, obj->ramTotal);
			if (obj->oldMem.cdramTotal != mem.cdramTotal || obj->oldMem.cdramFree != mem.cdramFree)
				SetMemLabel(mem.cdramTotal - mem.cdramFree, mem.cdramTotal, obj->cdramUsed, obj->cdramTotal);
			if (obj->oldMem.phycontTotal != mem.phycontTotal || obj->oldMem.phycontFree != mem.phycontFree)
				SetMemLabel(mem.phycontTotal - mem.phycontFree, mem.phycontTotal, obj->phyUsed, obj->phyTotal);
			if (obj->oldMem.cdialogTotal != mem.cdialogTotal || obj->oldMem.cdialogFree != mem.cdialogFree)
				SetMemLabel(mem.cdialogTotal - mem.cdialogFree, mem.cdialogTotal, obj->cdlgUsed, obj->cdlgTotal);

			//Update fg app
			obj->app->SetString(*tracker::GetCurrentAppName());

			obj->memTick = tickNow;
			obj->oldMem = mem;
		}
	}

	void HudFull::SetMemLabel(uint32_t used, uint32_t total, paf::ui::Text *usedText, paf::ui::Text *totalText)
	{
		wstring wtext;
		wchar_t wbuf[32];
		math::v4 green(0.0f, 1.0f, 0.0f, 1.0f);

		int32_t usedValue = ValueFromSize(used);
		int32_t totalValue = ValueFromSize(total);

		sce_paf_swprintf(wbuf, 32, L" %d %ls /", usedValue, UnitsFromSize(used));
		wtext = wbuf;
		usedText->SetString(wtext);
		//usedText->SetColor(ui::Text::ColorType_Text, 1, LengthOfValue(usedValue), &ScaleColor(used, total - (total / 10), total + (total / 10)));

		sce_paf_swprintf(wbuf, 32, L" %d %ls", totalValue, UnitsFromSize(total));
		wtext = wbuf;
		totalText->SetString(wtext);
		//totalText->SetColor(ui::Text::ColorType_Text, 1, LengthOfValue(totalValue), &green);
	}

	HudDev::HudDev() : HudMini::HudMini(Hud::BaseType_Dev)
	{
		Plugin::TemplateOpenParam tmpParam;

		this->oldMem.mainTotal = __INT_MAX__;
		this->oldMem.cdramTotal = __INT_MAX__;
		this->oldMem.phycontTotal = __INT_MAX__;
		this->oldMem.cdialogTotal = __INT_MAX__;
		this->oldVnz.core0 = __INT_MAX__;
		this->oldVnz.core1 = __INT_MAX__;
		this->oldVnz.core2 = __INT_MAX__;
		this->oldVnz.core3 = __INT_MAX__;
		this->oldVnz.core4 = __INT_MAX__;
		this->oldVnz.core5 = __INT_MAX__;
		this->oldVnz.core6 = __INT_MAX__;
		this->oldVnz.core7 = __INT_MAX__;
		this->oldVnz.average = __INT_MAX__;
		this->oldVnz.peak = __INT_MAX__;
		this->vnzNeedUpdate = false;
		this->memTickCommon = 0;
		this->memTickVnzUpd = 0;
		this->memTickBat = 0;

		g_corePlugin->TemplateOpen(this->root, psvs_template_hud_dev, tmpParam);

		this->vnz[0] = (ui::Text *)this->root->FindChild(psvs_text_vnz_0);
		this->vnz[1] = (ui::Text *)this->root->FindChild(psvs_text_vnz_1);
		this->vnz[2] = (ui::Text *)this->root->FindChild(psvs_text_vnz_2);
		this->vnz[3] = (ui::Text *)this->root->FindChild(psvs_text_vnz_3);
		this->vnz[4] = (ui::Text *)this->root->FindChild(psvs_text_vnz_4);
		this->vnz[5] = (ui::Text *)this->root->FindChild(psvs_text_vnz_5);
		this->vnz[6] = (ui::Text *)this->root->FindChild(psvs_text_vnz_6);
		this->vnz[7] = (ui::Text *)this->root->FindChild(psvs_text_vnz_7);
		this->vnzPeak = (ui::Text *)this->root->FindChild(psvs_text_vnz_peak);
		this->batCur = (ui::Text *)this->root->FindChild(psvs_text_bat_cur);

		if (SCE_PAF_IS_DOLCE) {
			wstring wtext = L"DOLCE";
			this->batCur->SetString(wtext);
		}

		this->ramUsed = (ui::Text *)this->root->FindChild(HudFull::psvs_text_ram_used);
		this->cdramUsed = (ui::Text *)this->root->FindChild(HudFull::psvs_text_cdram_used);
		this->phyUsed = (ui::Text *)this->root->FindChild(HudFull::psvs_text_phy_used);
		this->cdlgUsed = (ui::Text *)this->root->FindChild(HudFull::psvs_text_cdlg_used);
		this->ramTotal = (ui::Text *)this->root->FindChild(HudFull::psvs_text_ram_total);
		this->cdramTotal = (ui::Text *)this->root->FindChild(HudFull::psvs_text_cdram_total);
		this->phyTotal = (ui::Text *)this->root->FindChild(HudFull::psvs_text_phy_total);
		this->cdlgTotal = (ui::Text *)this->root->FindChild(HudFull::psvs_text_cdlg_total);
		this->app = (ui::Text *)this->root->FindChild(HudFull::psvs_text_app);

		common::MainThreadCallList::Register(Update, this);

		s_currentHud = this;
	}

	HudDev::~HudDev()
	{
		common::MainThreadCallList::Unregister(Update, this);
	}

	void HudDev::SetPosition(Position pos)
	{
		math::v4 vec(0.0f);

		switch (pos) {
		case Position_UpLeft:
			vec.set_x(10.0f);
			vec.set_y(-10.0f);
			break;
		case Position_UpRight:
			vec.set_x(600.0f);
			vec.set_y(-10.0f);
			break;
		case Position_DownLeft:
			vec.set_x(10.0f);
			vec.set_y(-256.0f);
			break;
		case Position_DownRight:
			vec.set_x(600.0f);
			vec.set_y(-256.0f);
			break;
		}

		this->root->SetPos(vec);
	}

	void HudDev::Update(void *arg)
	{
		wstring wtext;
		wchar_t wbuf[16];
		PSVSMem mem;
		PSVSVenezia vnz;
		PSVSBattery bat;
		uint32_t tickNow = sceKernelGetProcessTimeLow();
		HudDev *obj = (HudDev *)arg;

		if (tickNow - obj->memTickVnzUpd > PSVS_VNZ_UPDATE_WINDOW_USEC) {
			//Update Venezia
			sceCodecEnginePmonStop();
			psvsGetVeneziaInfo(&vnz);
			sceCodecEnginePmonReset();
			sceCodecEnginePmonStart();
			obj->vnzNeedUpdate = true;
			obj->memTickVnzUpd = tickNow;
		}

		if (!SCE_PAF_IS_DOLCE && (tickNow - obj->memTickBat > PSVS_BAT_UPDATE_WINDOW_USEC)) {
			//Update battery
			psvsGetBatteryInfo(&bat);

			if (obj->oldBat.current != bat.current) {
				sce_paf_swprintf(wbuf, 16, L" %d mA", bat.current);
				wtext = wbuf;
				obj->batCur->SetString(wtext);
			}

			obj->memTickBat = tickNow;
			obj->oldBat = bat;
		}

		if (tickNow - obj->memTickCommon > PSVS_FULL_UPDATE_WINDOW_USEC) {

			if (obj->vnzNeedUpdate) {
				if (obj->oldVnz.core0 != vnz.core0) {
					sce_paf_swprintf(wbuf, 16, L" %d%%", vnz.core0);
					wtext = wbuf;
					obj->vnz[0]->SetString(wtext);
				}
				if (obj->oldVnz.core1 != vnz.core1) {
					sce_paf_swprintf(wbuf, 16, L" %d%%", vnz.core1);
					wtext = wbuf;
					obj->vnz[1]->SetString(wtext);
				}
				if (obj->oldVnz.core2 != vnz.core2) {
					sce_paf_swprintf(wbuf, 16, L" %d%%", vnz.core2);
					wtext = wbuf;
					obj->vnz[2]->SetString(wtext);
				}
				if (obj->oldVnz.core3 != vnz.core3) {
					sce_paf_swprintf(wbuf, 16, L" %d%%", vnz.core3);
					wtext = wbuf;
					obj->vnz[3]->SetString(wtext);
				}
				if (obj->oldVnz.core4 != vnz.core4) {
					sce_paf_swprintf(wbuf, 16, L" %d%%", vnz.core4);
					wtext = wbuf;
					obj->vnz[4]->SetString(wtext);
				}
				if (obj->oldVnz.core5 != vnz.core5) {
					sce_paf_swprintf(wbuf, 16, L" %d%%", vnz.core5);
					wtext = wbuf;
					obj->vnz[5]->SetString(wtext);
				}
				if (obj->oldVnz.core6 != vnz.core6) {
					sce_paf_swprintf(wbuf, 16, L" %d%%", vnz.core6);
					wtext = wbuf;
					obj->vnz[6]->SetString(wtext);
				}
				if (obj->oldVnz.core7 != vnz.core7) {
					sce_paf_swprintf(wbuf, 16, L" %d%%", vnz.core7);
					wtext = wbuf;
					obj->vnz[7]->SetString(wtext);
				}
				if (obj->oldVnz.peak != vnz.peak) {
					sce_paf_swprintf(wbuf, 16, L" %d%%", vnz.peak);
					wtext = wbuf;
					obj->vnzPeak->SetString(wtext);
				}
				obj->vnzNeedUpdate = SCE_FALSE;
			}

			//Update memory
			psvsGetMem(s_casShift, &mem);

			if (obj->oldMem.mainTotal != mem.mainTotal || obj->oldMem.mainFree != mem.mainFree)
				SetMemLabel(mem.mainTotal - mem.mainFree, mem.mainTotal, obj->ramUsed, obj->ramTotal);
			if (obj->oldMem.cdramTotal != mem.cdramTotal || obj->oldMem.cdramFree != mem.cdramFree)
				SetMemLabel(mem.cdramTotal - mem.cdramFree, mem.cdramTotal, obj->cdramUsed, obj->cdramTotal);
			if (obj->oldMem.phycontTotal != mem.phycontTotal || obj->oldMem.phycontFree != mem.phycontFree)
				SetMemLabel(mem.phycontTotal - mem.phycontFree, mem.phycontTotal, obj->phyUsed, obj->phyTotal);
			if (obj->oldMem.cdialogTotal != mem.cdialogTotal || obj->oldMem.cdialogFree != mem.cdialogFree)
				SetMemLabel(mem.cdialogTotal - mem.cdialogFree, mem.cdialogTotal, obj->cdlgUsed, obj->cdlgTotal);

			//Update fg app
			obj->app->SetString(*tracker::GetCurrentAppName());

			obj->memTickCommon = tickNow;
			obj->oldMem = mem;
			obj->oldVnz = vnz;
		}
	}

	void HudDev::SetMemLabel(uint32_t used, uint32_t total, paf::ui::Text *usedText, paf::ui::Text *totalText)
	{
		wstring wtext;
		wchar_t wbuf[32];
		math::v4 green(0.0f, 1.0f, 0.0f, 1.0f);

		int32_t usedValue = ValueFromSize(used);
		int32_t totalValue = ValueFromSize(total);

		sce_paf_swprintf(wbuf, 32, L" %d %ls /", usedValue, UnitsFromSize(used));
		wtext = wbuf;
		usedText->SetString(wtext);
		//usedText->SetColor(ui::Text::ColorType_Text, 1, LengthOfValue(usedValue), &ScaleColor(used, total - (total / 10), total + (total / 10)));

		sce_paf_swprintf(wbuf, 32, L" %d %ls", totalValue, UnitsFromSize(total));
		wtext = wbuf;
		totalText->SetString(wtext);
		//totalText->SetColor(ui::Text::ColorType_Text, 1, LengthOfValue(totalValue), &green);
	}
}