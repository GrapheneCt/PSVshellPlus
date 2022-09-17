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
	static SceInt32 s_casShift = -1;

	Hud *Hud::GetCurrentHud()
	{
		return s_currentHud;
	}

	SceVoid Hud::SetCasShift(SceInt32 shift)
	{
		s_casShift = shift;
	}

	Hud::Hud(BaseType type)
	{
		rco::Element element;
		Plugin::TemplateInitParam tmpParam;

		switch (type) {
		case BaseType_Mini:
			element.hash = psvs_template_hud_bg_mini;
			g_corePlugin->TemplateOpen(g_coreRoot, &element, &tmpParam);
			this->root = g_coreRoot->GetChild(g_coreRoot->childNum - 1);
			this->root->PlayEffect(0.0f, effect::EffectType_Fadein1);
			break;
		case BaseType_Full:
			element.hash = psvs_template_hud_bg_full;
			g_corePlugin->TemplateOpen(g_coreRoot, &element, &tmpParam);
			this->root = g_coreRoot->GetChild(g_coreRoot->childNum - 1);
			this->root->PlayEffect(0.0f, effect::EffectType_Fadein1);
			break;
		default:
			break;
		}

		psvsSetFpsCounterTarget(tracker::GetFpsCounterTarget());
	}

	Hud::~Hud()
	{
		psvsSetFpsCounterTarget(PSVS_FPS_COUNTER_TARGET_NONE);
		effect::Play(0.0f, this->root, effect::EffectType_Fadein1, true, false);
		s_currentHud = NULL;
	}

	HudMicro::HudMicro() : Hud::Hud(Hud::BaseType_None)
	{
		rco::Element element;
		Plugin::TemplateInitParam tmpParam;

		this->oldFps = 0;

		element.hash = psvs_template_hud_micro;
		g_corePlugin->TemplateOpen(g_coreRoot, &element, &tmpParam);
		element.hash = psvs_text_fps_micro;
		this->fps = (ui::Text *)g_coreRoot->GetChild(&element, 0);
		this->root = this->fps;
		task::Register(Update, this);
		s_currentHud = this;
	}

	HudMicro::~HudMicro()
	{
		task::Unregister(Update, this);
	}

	SceVoid HudMicro::SetPosition(Position pos)
	{
		Vector4 vec;
		vec.z = 0.0f;
		vec.w = 0.0f;

		switch (pos) {
		case Position_UpLeft:
			vec.x = -920.0f;
			vec.y = -10.0f;
			break;
		case Position_UpRight:
			vec.x = -16.0f;
			vec.y = -10.0f;
			break;
		case Position_DownLeft:
			vec.x = -920.0f;
			vec.y = -520.0f;
			break;
		case Position_DownRight:
			vec.x = -16.0f;
			vec.y = -520.0f;
			break;
		}

		this->root->SetPosition(&vec);
	}

	SceVoid HudMicro::Update(ScePVoid arg)
	{
		wstring wtext;
		wchar_t wbuf[6];
		HudMicro *obj = (HudMicro *)arg;

		//Update FPS
		psvsSetFpsCounterTarget(tracker::GetFpsCounterTarget());
		SceInt32 fps = psvsGetFps();

		if (fps != obj->oldFps) {
			sce_paf_swprintf(wbuf, sizeof(wbuf), L"%d", fps);
			wtext = wbuf;
			obj->fps->SetLabel(&wtext);
		}

		obj->oldFps = fps;
	}

	HudMini::HudMini(Hud::BaseType type) : Hud::Hud(type)
	{
		rco::Element element;
		Plugin::TemplateInitParam tmpParam;

		this->oldFps = 0;

		element.hash = psvs_template_hud_mini;
		g_corePlugin->TemplateOpen(this->root, &element, &tmpParam);

		element.hash = psvs_text_cpu_0;
		this->cpuAvg[0] = (ui::Text *)this->root->GetChild(&element, 0);

		element.hash = psvs_text_cpu_1;
		this->cpuAvg[1] = (ui::Text *)this->root->GetChild(&element, 0);

		element.hash = psvs_text_cpu_2;
		this->cpuAvg[2] = (ui::Text *)this->root->GetChild(&element, 0);

		element.hash = psvs_text_cpu_3;
		this->cpuAvg[3] = (ui::Text *)this->root->GetChild(&element, 0);

		element.hash = psvs_text_cpu_peak;
		this->cpuPeak = (ui::Text *)this->root->GetChild(&element, 0);

		element.hash = psvs_text_fps;
		this->fps = (ui::Text *)this->root->GetChild(&element, 0);

		task::Register(Update, this);

		s_currentHud = this;
	}

	HudMini::~HudMini()
	{
		task::Unregister(Update, this);
	}

	SceVoid HudMini::SetPosition(Position pos)
	{
		Vector4 vec;
		vec.z = 0.0f;
		vec.w = 0.0f;

		switch (pos) {
		case Position_UpLeft:
			vec.x = 10.0f;
			vec.y = -10.0f;
			break;
		case Position_UpRight:
			vec.x = 600.0f;
			vec.y = -10.0f;
			break;
		case Position_DownLeft:
			vec.x = 10.0f;
			vec.y = -470.0f;
			break;
		case Position_DownRight:
			vec.x = 600.0f;
			vec.y = -470.0f;
			break;
		}

		this->root->SetPosition(&vec);
	}

	SceVoid HudMini::Update(ScePVoid arg)
	{
		wstring wtext;
		wchar_t wbuf[16];
		PSVSCpu cpu;
		HudMini *obj = (HudMini *)arg;

		//Update FPS
		psvsSetFpsCounterTarget(tracker::GetFpsCounterTarget());
		SceInt32 fps = psvsGetFps();

		if (fps != obj->oldFps) {
			sce_paf_swprintf(wbuf, sizeof(wbuf), L" %d FPS", fps);
			wtext = wbuf;
			obj->fps->SetLabel(&wtext);
			//obj->fps->SetColor(ui::Text::ColorType_Text, 1, 2, &ScaleColor(30 - fps, 0, 30));
		}

		obj->oldFps = fps;

		//Update CPU
		psvsGetCpu(&cpu);

		for (int i = 0; i < SCE_KERNEL_MAX_CPU; i++) {
			sce_paf_swprintf(wbuf, sizeof(wbuf), L" %d%%", cpu.avg[i]);
			wtext = wbuf;
			obj->cpuAvg[i]->SetLabel(&wtext);
			//obj->cpuAvg[i]->SetColor(ui::Text::ColorType_Text, 1, LengthOfValue(cpu.avg[i]), &ScaleColor(cpu.avg[i], 0, 100));
		}

		sce_paf_swprintf(wbuf, sizeof(wbuf), L" %d%%", cpu.peak);
		wtext = wbuf;
		obj->cpuPeak->SetLabel(&wtext);
		//obj->cpuPeak->SetColor(ui::Text::ColorType_Text, 1, LengthOfValue(cpu.peak), &ScaleColor(cpu.peak, 0, 100));
	}

	HudFull::HudFull() : HudMini::HudMini(Hud::BaseType_Full)
	{
		rco::Element element;
		Plugin::TemplateInitParam tmpParam;

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
		this->memTick = 0;

		element.hash = psvs_template_hud_vnz;
		g_corePlugin->TemplateOpen(this->root, &element, &tmpParam);

		element.hash = psvs_text_vnz_0;
		this->vnz[0] = (ui::Text *)this->root->GetChild(&element, 0);

		element.hash = psvs_text_vnz_1;
		this->vnz[1] = (ui::Text *)this->root->GetChild(&element, 0);

		element.hash = psvs_text_vnz_2;
		this->vnz[2] = (ui::Text *)this->root->GetChild(&element, 0);

		element.hash = psvs_text_vnz_3;
		this->vnz[3] = (ui::Text *)this->root->GetChild(&element, 0);

		element.hash = psvs_text_vnz_4;
		this->vnz[4] = (ui::Text *)this->root->GetChild(&element, 0);

		element.hash = psvs_text_vnz_5;
		this->vnz[5] = (ui::Text *)this->root->GetChild(&element, 0);

		element.hash = psvs_text_vnz_6;
		this->vnz[6] = (ui::Text *)this->root->GetChild(&element, 0);

		element.hash = psvs_text_vnz_7;
		this->vnz[7] = (ui::Text *)this->root->GetChild(&element, 0);

		element.hash = psvs_text_vnz_peak;
		this->vnzPeak = (ui::Text *)this->root->GetChild(&element, 0);

		element.hash = psvs_template_hud_full;
		g_corePlugin->TemplateOpen(this->root, &element, &tmpParam);

		element.hash = psvs_text_ram_used;
		this->ramUsed = (ui::Text *)this->root->GetChild(&element, 0);

		element.hash = psvs_text_cdram_used;
		this->cdramUsed = (ui::Text *)this->root->GetChild(&element, 0);

		element.hash = psvs_text_phy_used;
		this->phyUsed = (ui::Text *)this->root->GetChild(&element, 0);

		element.hash = psvs_text_cdlg_used;
		this->cdlgUsed = (ui::Text *)this->root->GetChild(&element, 0);

		element.hash = psvs_text_ram_total;
		this->ramTotal = (ui::Text *)this->root->GetChild(&element, 0);

		element.hash = psvs_text_cdram_total;
		this->cdramTotal = (ui::Text *)this->root->GetChild(&element, 0);

		element.hash = psvs_text_phy_total;
		this->phyTotal = (ui::Text *)this->root->GetChild(&element, 0);

		element.hash = psvs_text_cdlg_total;
		this->cdlgTotal = (ui::Text *)this->root->GetChild(&element, 0);

		element.hash = psvs_text_app;
		this->app = (ui::Text *)this->root->GetChild(&element, 0);

		task::Register(Update, this);

		s_currentHud = this;
	}

	HudFull::~HudFull()
	{
		task::Unregister(Update, this);
	}

	SceVoid HudFull::SetPosition(Position pos)
	{
		Vector4 vec;
		vec.z = 0.0f;
		vec.w = 0.0f;

		switch (pos) {
		case Position_UpLeft:
			vec.x = 10.0f;
			vec.y = -10.0f;
			break;
		case Position_UpRight:
			vec.x = 600.0f;
			vec.y = -10.0f;
			break;
		case Position_DownLeft:
			vec.x = 10.0f;
			vec.y = -288.0f;
			break;
		case Position_DownRight:
			vec.x = 600.0f;
			vec.y = -288.0f;
			break;
		}

		this->root->SetPosition(&vec);
	}

	SceVoid HudFull::Update(ScePVoid arg)
	{
		wstring wtext;
		wchar_t wbuf[8];
		PSVSMem mem;
		PSVSVenezia vnz;
		SceUInt32 tickNow = sceKernelGetProcessTimeLow();
		SceUInt32 used = 0;
		SceUInt32 total = 0;
		SceBool vnzUpdateFrame = SCE_FALSE;
		HudFull *obj = (HudFull *)arg;

		if (tickNow - obj->memTick > PSVS_VNZ_UPDATE_WINDOW_USEC) {
			//Update Venezia
			sceCodecEnginePmonStop();
			psvsGetVeneziaInfo(&vnz);
			sceCodecEnginePmonReset();
			sceCodecEnginePmonStart();
			vnzUpdateFrame = SCE_TRUE;
		}

		if (tickNow - obj->memTick > PSVS_FULL_UPDATE_WINDOW_USEC) {

			if (vnzUpdateFrame) {
				if (obj->oldVnz.core0 != vnz.core0) {
					sce_paf_swprintf(wbuf, sizeof(wbuf), L" %d%%", vnz.core0);
					wtext = wbuf;
					obj->vnz[0]->SetLabel(&wtext);
				}
				if (obj->oldVnz.core1 != vnz.core1) {
					sce_paf_swprintf(wbuf, sizeof(wbuf), L" %d%%", vnz.core1);
					wtext = wbuf;
					obj->vnz[1]->SetLabel(&wtext);
				}
				if (obj->oldVnz.core2 != vnz.core2) {
					sce_paf_swprintf(wbuf, sizeof(wbuf), L" %d%%", vnz.core2);
					wtext = wbuf;
					obj->vnz[2]->SetLabel(&wtext);
				}
				if (obj->oldVnz.core3 != vnz.core3) {
					sce_paf_swprintf(wbuf, sizeof(wbuf), L" %d%%", vnz.core3);
					wtext = wbuf;
					obj->vnz[3]->SetLabel(&wtext);
				}
				if (obj->oldVnz.core4 != vnz.core4) {
					sce_paf_swprintf(wbuf, sizeof(wbuf), L" %d%%", vnz.core4);
					wtext = wbuf;
					obj->vnz[4]->SetLabel(&wtext);
				}
				if (obj->oldVnz.core5 != vnz.core5) {
					sce_paf_swprintf(wbuf, sizeof(wbuf), L" %d%%", vnz.core5);
					wtext = wbuf;
					obj->vnz[5]->SetLabel(&wtext);
				}
				if (obj->oldVnz.core6 != vnz.core6) {
					sce_paf_swprintf(wbuf, sizeof(wbuf), L" %d%%", vnz.core6);
					wtext = wbuf;
					obj->vnz[6]->SetLabel(&wtext);
				}
				if (obj->oldVnz.core7 != vnz.core7) {
					sce_paf_swprintf(wbuf, sizeof(wbuf), L" %d%%", vnz.core7);
					wtext = wbuf;
					obj->vnz[7]->SetLabel(&wtext);
				}
				if (obj->oldVnz.peak != vnz.peak) {
					sce_paf_swprintf(wbuf, sizeof(wbuf), L" %d%%", vnz.peak);
					wtext = wbuf;
					obj->vnzPeak->SetLabel(&wtext);
				}
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
			obj->app->SetLabel(tracker::GetCurrentAppName());

			obj->memTick = tickNow;
			obj->oldMem = mem;
			obj->oldVnz = vnz;
		}
	}

	SceVoid HudFull::SetMemLabel(SceUInt32 used, SceUInt32 total, paf::ui::Text *usedText, paf::ui::Text *totalText)
	{
		wstring wtext;
		wchar_t wbuf[16];
		Rgba green(0.0f, 1.0f, 0.0f, 1.0f);

		SceInt32 usedValue = ValueFromSize(used);
		SceInt32 totalValue = ValueFromSize(total);

		sce_paf_swprintf(wbuf, sizeof(wbuf), L" %d %ls /", usedValue, UnitsFromSize(used));
		wtext = wbuf;
		usedText->SetLabel(&wtext);
		//usedText->SetColor(ui::Text::ColorType_Text, 1, LengthOfValue(usedValue), &ScaleColor(used, total - (total / 10), total + (total / 10)));

		sce_paf_swprintf(wbuf, sizeof(wbuf), L" %d %ls", totalValue, UnitsFromSize(total));
		wtext = wbuf;
		totalText->SetLabel(&wtext);
		//totalText->SetColor(ui::Text::ColorType_Text, 1, LengthOfValue(totalValue), &green);
	}
}