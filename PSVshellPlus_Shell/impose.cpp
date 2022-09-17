#include <kernel.h>
#include <libdbg.h>
#include <paf.h>

#include "global.h"
#include "utils.h"
#include "psvs.h"
#include "impose.h"
#include "tracker.h"
#include "hud.h"
#include "profile.h"

using namespace paf;

namespace psvs
{
	static SceInt32 s_hudMode = Impose::HudMode_Off;
	static SceInt32 s_hudPos = Hud::Position_UpLeft;
	static SceInt32 s_cas = -0x1000;

	const SceInt32 k_cpuClock[] = { 41, 83, 111, 166, 222, 333, 444, 500 };
	const SceInt32 k_gpuClock[] = { 41, 55, 83, 111, 166, 222 };
	const SceInt32 k_busClock[] = { 55, 83, 111, 166, 222 };
	const SceInt32 k_xbarClock[] = { 83, 111, 166 };

	SceVoid Impose::ControlButtonCB::ControlButtonCBFun(SceInt32 eventId, ui::Widget *self, SceInt32 a3, ScePVoid pUserData)
	{
		wstring wstr;
		wchar_t casVal[2];
		SceInt32 arraySize = 0;
		const SceInt32 *arrayPtr = SCE_NULL;
		SceInt32 direction = 0;
		SceInt32 currentClock = 0;
		Impose *obj = (Impose *)pUserData;
		Hud *currHud = SCE_NULL;
		SceInt32(*clockSet)(int clock) = SCE_NULL;
		PSVSClockFrequency clock;
		psvsGetClockFrequency(&clock);
			 
		switch (self->elem.hash) {
		case psvs_button_impose_cpu_down:
			currentClock = clock.cpu;
			clockSet = psvsSetArmClockFrequency;
			direction = -1;
			arraySize = sizeof(k_cpuClock) / sizeof(SceInt32);
			arrayPtr = k_cpuClock;
			break;
		case psvs_button_impose_cpu_up:
			currentClock = clock.cpu;
			clockSet = psvsSetArmClockFrequency;
			direction = 1;
			arraySize = sizeof(k_cpuClock) / sizeof(SceInt32);
			arrayPtr = k_cpuClock;
			break;
		case psvs_button_impose_gpu_down:
			currentClock = clock.gpu;
			clockSet = psvsSetGpuClockFrequency;
			direction = -1;
			arraySize = sizeof(k_gpuClock) / sizeof(SceInt32);
			arrayPtr = k_gpuClock;
			break;
		case psvs_button_impose_gpu_up:
			currentClock = clock.gpu;
			clockSet = psvsSetGpuClockFrequency;
			direction = 1;
			arraySize = sizeof(k_gpuClock) / sizeof(SceInt32);
			arrayPtr = k_gpuClock;
			break;
			/*
		case psvs_button_impose_bus_down:
			currentClock = clock.bus;
			clockSet = psvsSetBusClockFrequency;
			direction = -1;
			arraySize = sizeof(k_busClock) / sizeof(SceInt32);
			arrayPtr = k_busClock;
			break;
		case psvs_button_impose_bus_up:
			currentClock = clock.bus;
			clockSet = psvsSetBusClockFrequency;
			direction = 1;
			arraySize = sizeof(k_busClock) / sizeof(SceInt32);
			arrayPtr = k_busClock;
			break;
			*/
		case psvs_button_impose_xbar_down:
			currentClock = clock.xbar;
			clockSet = psvsSetGpuXbarClockFrequency;
			direction = -1;
			arraySize = sizeof(k_xbarClock) / sizeof(SceInt32);
			arrayPtr = k_xbarClock;
			break;
		case psvs_button_impose_xbar_up:
			currentClock = clock.xbar;
			clockSet = psvsSetGpuXbarClockFrequency;
			direction = 1;
			arraySize = sizeof(k_xbarClock) / sizeof(SceInt32);
			arrayPtr = k_xbarClock;
			break;
		case psvs_button_impose_hud_down:
			if (s_hudMode != HudMode_Off) {
				s_hudMode--;
			}
			else {
				break;
			}
			thread::s_mainThreadMutex.Lock();
			currHud = Hud::GetCurrentHud();
			if (currHud != SCE_NULL) {
				delete currHud;
			}
			switch (s_hudMode) {
			case HudMode_Off:
				wstr = GetString("psvs_msg_hud_off");
				break;
			case HudMode_Micro:
				wstr = GetString("psvs_msg_hud_micro");
				currHud = new HudMicro();
				currHud->SetPosition((Hud::Position)s_hudPos);
				break;
			case HudMode_Mini:
				wstr = GetString("psvs_msg_hud_mini");
				currHud = new HudMini();
				currHud->SetPosition((Hud::Position)s_hudPos);
				break;
			case HudMode_Full:
				wstr = GetString("psvs_msg_hud_full");
				currHud = new HudFull();
				currHud->SetPosition((Hud::Position)s_hudPos);
				break;
			}
			obj->hud->SetLabel(&wstr);
			thread::s_mainThreadMutex.Unlock();
			break;
		case psvs_button_impose_hud_up:
			if (s_hudMode != HudMode_Full) {
				s_hudMode++;
			}
			else {
				break;
			}
			thread::s_mainThreadMutex.Lock();
			currHud = Hud::GetCurrentHud();
			if (currHud != SCE_NULL) {
				delete currHud;
			}
			switch (s_hudMode) {
			case HudMode_Off:
				wstr = GetString("psvs_msg_hud_off");
				break;
			case HudMode_Micro:
				wstr = GetString("psvs_msg_hud_micro");
				currHud = new HudMicro();
				currHud->SetPosition((Hud::Position)s_hudPos);
				break;
			case HudMode_Mini:
				wstr = GetString("psvs_msg_hud_mini");
				currHud = new HudMini();
				currHud->SetPosition((Hud::Position)s_hudPos);
				break;
			case HudMode_Full:
				wstr = GetString("psvs_msg_hud_full");
				currHud = new HudFull();
				currHud->SetPosition((Hud::Position)s_hudPos);
				break;
			}
			obj->hud->SetLabel(&wstr);
			thread::s_mainThreadMutex.Unlock();
			break;
		case psvs_button_impose_hud_pos_down:
			currHud = Hud::GetCurrentHud();
			if (s_hudPos != Hud::Position_UpLeft) {
				s_hudPos--;
			}
			else {
				break;
			}
			thread::s_mainThreadMutex.Lock();
			switch (s_hudPos) {
			case Hud::Position_UpLeft:
				wstr = GetString("psvs_msg_hud_pos_ul");
				break;
			case Hud::Position_UpRight:
				wstr = GetString("psvs_msg_hud_pos_ur");
				break;
			case Hud::Position_DownLeft:
				wstr = GetString("psvs_msg_hud_pos_dl");
				break;
			case Hud::Position_DownRight:
				wstr = GetString("psvs_msg_hud_pos_dr");
				break;
			}
			obj->hudpos->SetLabel(&wstr);
			if (currHud != SCE_NULL)
				currHud->SetPosition((Hud::Position)s_hudPos);
			thread::s_mainThreadMutex.Unlock();
			break;
		case psvs_button_impose_hud_pos_up:
			currHud = Hud::GetCurrentHud();
			if (s_hudPos != Hud::Position_DownRight) {
				s_hudPos++;
			}
			else {
				break;
			}
			thread::s_mainThreadMutex.Lock();
			switch (s_hudPos) {
			case Hud::Position_UpLeft:
				wstr = GetString("psvs_msg_hud_pos_ul");
				break;
			case Hud::Position_UpRight:
				wstr = GetString("psvs_msg_hud_pos_ur");
				break;
			case Hud::Position_DownLeft:
				wstr = GetString("psvs_msg_hud_pos_dl");
				break;
			case Hud::Position_DownRight:
				wstr = GetString("psvs_msg_hud_pos_dr");
				break;
			}
			obj->hudpos->SetLabel(&wstr);
			if (currHud != SCE_NULL)
				currHud->SetPosition((Hud::Position)s_hudPos);
			thread::s_mainThreadMutex.Unlock();
			break;
		case psvs_button_impose_cas_down:
			if (s_cas >= 0) {
				s_cas -= 0x1000;
			}
			else {
				break;
			}
			thread::s_mainThreadMutex.Lock();
			if (s_cas < 0) {
				wstr = GetString("psvs_msg_auto");
			}
			else {
				sce_paf_swprintf(casVal, sizeof(casVal), L"%d", s_cas / 0x1000);
				wstr = casVal;
			}
			obj->cas->SetLabel(&wstr);
			Hud::SetCasShift(s_cas);
			thread::s_mainThreadMutex.Unlock();
			break;
		case psvs_button_impose_cas_up:
			if (s_cas != 0x7000) {
				s_cas += 0x1000;
			}
			else {
				break;
			}
			thread::s_mainThreadMutex.Lock();
			sce_paf_swprintf(casVal, sizeof(casVal) / sizeof(wchar_t), L"%d", s_cas / 0x1000);
			wstr = casVal;
			obj->cas->SetLabel(&wstr);
			Hud::SetCasShift(s_cas);
			thread::s_mainThreadMutex.Unlock();
			break;
		}

		if (direction == -1) {
			for (int i = arraySize - 1; i > -1; i--) {
				if (arrayPtr[i] < currentClock) {
					clockSet(arrayPtr[i]);
					break;
				}
			}
		}
		else if (direction == 1) {
			for (int i = 0; i < arraySize; i++) {
				if (arrayPtr[i] > currentClock) {
					clockSet(arrayPtr[i]);
					break;
				}
			}
		}
	}

	SceVoid Impose::ProfileButtonCB::ProfileButtonCBFun(SceInt32 eventId, ui::Widget *self, SceInt32 a3, ScePVoid pUserData)
	{
		ui::Widget *bt = (ui::Widget *)pUserData;

		if (self->elem.hash == psvs_button_impose_save) {
			psvs::Profile *profile = new psvs::Profile();
			profile->Save();
			delete profile;

			tracker::ReloadCurrentProfile();
			bt->Enable(true);
		}
		else if (self->elem.hash == psvs_button_impose_delete) {
			psvs::Profile::Delete();
			tracker::ReloadCurrentProfile();
			bt->Disable(true);
		}
	}

	SceVoid Impose::LockCB::LockCBFun(SceInt32 eventId, ui::Widget *self, SceInt32 a3, ScePVoid pUserData)
	{
		ui::CheckBox *obj = (ui::CheckBox *)self;
		ScePID pid = psvs::tracker::GetCurrentPID();
		PsvsLockDevice target = PSVS_LOCK_DEVICE_NONE;

		switch (self->elem.hash) {
		case psvs_check_box_impose_cpu_lock:
			target = PSVS_LOCK_DEVICE_CPU;
			break;
		case psvs_check_box_impose_gpu_lock:
			target = PSVS_LOCK_DEVICE_GPU_ES4;
			break;
		case psvs_check_box_impose_bus_lock:
			target = PSVS_LOCK_DEVICE_BUS;
			break;
		case psvs_check_box_impose_xbar_lock:
			target = PSVS_LOCK_DEVICE_GPU_XBAR;
			break;
		}

		if (obj->state == 1)
			psvsClockFrequencyLockProc(pid, target);
		else
			psvsClockFrequencyUnlockProc(pid, target);
	}

	SceVoid Impose::Update(ScePVoid arg)
	{
		rco::Element element;
		PSVSClockFrequency clock;
		wstring wtext;
		wchar_t wbuf[16];
		SceUInt32 tickNow = sceKernelGetProcessTimeLow();
		Impose *obj = (Impose *)arg;

		if (tickNow - obj->oldTick > PSVS_IMPOSE_UPDATE_WINDOW_USEC) {
			element.hash = 0x0EE0C8AF;
			if (!g_imposeRoot->GetChild(&element, 0)) {
				delete obj;
				return;
			}

			//Update clock
			psvsGetClockFrequency(&clock);

			if (obj->oldClock.cpu != clock.cpu) {
				sce_paf_swprintf(wbuf, sizeof(wbuf), L"%d MHz", clock.cpu);
				wtext = wbuf;
				obj->cpu->SetLabel(&wtext);
			}

			if (obj->oldClock.gpu != clock.gpu) {
				sce_paf_swprintf(wbuf, sizeof(wbuf), L"%d MHz", clock.gpu);
				wtext = wbuf;
				obj->gpu->SetLabel(&wtext);
			}

			/*
			if (obj->oldClock.bus != clock.bus) {
				sce_paf_swprintf(wbuf, sizeof(wbuf), L"%d MHz", clock.bus);
				wtext = wbuf;
				obj->bus->SetLabel(&wtext);
			}
			*/

			if (obj->oldClock.xbar != clock.xbar) {
				sce_paf_swprintf(wbuf, sizeof(wbuf), L"%d MHz", clock.xbar);
				wtext = wbuf;
				obj->xbar->SetLabel(&wtext);
			}

			obj->oldTick = tickNow;
			obj->oldClock = clock;
		}
	}

	Hud::Position Impose::GetHudPosition()
	{
		return (Hud::Position)s_hudPos;
	}

	SceVoid Impose::SetHudPosition(Hud::Position pos)
	{
		s_hudPos = pos;
		Hud *currHud = Hud::GetCurrentHud();
		if (currHud != SCE_NULL) {
			currHud->SetPosition(pos);
		}
	}

	Impose::Impose(Plugin *plugin, ui::Box *root)
	{
		rco::Element welement;
		rco::Element selement;
		wstring wstr;
		wchar_t casVal[2];
		Vector4 sz;
		Plugin::TemplateInitParam tmpParam;
		ScePID pid = psvs::tracker::GetCurrentPID();

		this->oldTick = 0;
		this->plugin = plugin;
		this->root = root;
		this->imposePlugin = Plugin::Find("impose_plugin");
		sce_paf_memset(&this->oldClock, 0, sizeof(PSVSClockFrequency));

		welement.hash = 0x7ac075f7;
		selement.hash = 0x26c7781e;
		ui::Widget *sep0 = this->imposePlugin->CreateWidgetWithStyle(this->root, "plane", &welement, &selement);
		sz.x = 835.0f;
		sz.y = 20.0f;
		sep0->SetSize(&sz);

		welement.hash = psvs_template_impose;
		this->plugin->TemplateOpen(this->root, &welement, &tmpParam);

		welement.hash = 0x7ac075f7;
		ui::Widget *sep1 = this->imposePlugin->CreateWidgetWithStyle(this->root, "plane", &welement, &selement);
		sz.x = 835.0f;
		sz.y = 20.0f;
		sep1->SetSize(&sz);

		welement.hash = psvs_template_impose_2;
		this->plugin->TemplateOpen(this->root, &welement, &tmpParam);

		welement.hash = psvs_text_impose_cpu_value;
		this->cpu = (ui::Text *)this->root->GetChild(&welement, 0);

		welement.hash = psvs_text_impose_gpu_value;
		this->gpu = (ui::Text *)this->root->GetChild(&welement, 0);

		/*
		welement.hash = psvs_text_impose_bus_value;
		this->bus = (ui::Text *)this->root->GetChild(&welement, 0);
		*/

		welement.hash = psvs_text_impose_xbar_value;
		this->xbar = (ui::Text *)this->root->GetChild(&welement, 0);

		welement.hash = psvs_text_impose_hud_value;
		this->hud = (ui::Text *)this->root->GetChild(&welement, 0);
		switch (s_hudMode) {
		case HudMode_Off:
			wstr = GetString("psvs_msg_hud_off");
			break;
		case HudMode_Micro:
			wstr = GetString("psvs_msg_hud_micro");
			break;
		case HudMode_Mini:
			wstr = GetString("psvs_msg_hud_mini");
			break;
		case HudMode_Full:
			wstr = GetString("psvs_msg_hud_full");
			break;
		}
		this->hud->SetLabel(&wstr);

		welement.hash = psvs_text_impose_hud_pos_value;
		this->hudpos = (ui::Text *)this->root->GetChild(&welement, 0);
		switch (s_hudPos) {
		case Hud::Position_UpLeft:
			wstr = GetString("psvs_msg_hud_pos_ul");
			break;
		case Hud::Position_UpRight:
			wstr = GetString("psvs_msg_hud_pos_ur");
			break;
		case Hud::Position_DownLeft:
			wstr = GetString("psvs_msg_hud_pos_dl");
			break;
		case Hud::Position_DownRight:
			wstr = GetString("psvs_msg_hud_pos_dr");
			break;
		}
		this->hudpos->SetLabel(&wstr);

		welement.hash = psvs_text_impose_cas_value;
		this->cas = (ui::Text *)this->root->GetChild(&welement, 0);
		if (s_cas < 0) {
			wstr = GetString("psvs_msg_auto");
		}
		else {
			sce_paf_swprintf(casVal, sizeof(casVal) / sizeof(wchar_t), L"%d", s_cas / 0x1000);
			wstr = casVal;
		}
		this->cas->SetLabel(&wstr);

		ui::Widget *widget = SCE_NULL;
		ui::CheckBox *box = SCE_NULL;

		welement.hash = psvs_button_impose_cpu_down;
		widget = this->root->GetChild(&welement, 0);
		widget->RegisterEventCallback(ui::EventMain_Decide, new ControlButtonCB(this), 0);

		welement.hash = psvs_button_impose_cpu_up;
		widget = this->root->GetChild(&welement, 0);
		widget->RegisterEventCallback(ui::EventMain_Decide, new ControlButtonCB(this), 0);

		welement.hash = psvs_button_impose_gpu_down;
		widget = this->root->GetChild(&welement, 0);
		widget->RegisterEventCallback(ui::EventMain_Decide, new ControlButtonCB(this), 0);

		welement.hash = psvs_button_impose_gpu_up;
		widget = this->root->GetChild(&welement, 0);
		widget->RegisterEventCallback(ui::EventMain_Decide, new ControlButtonCB(this), 0);

		/*
		welement.hash = psvs_button_impose_bus_down;
		widget = this->root->GetChild(&welement, 0);
		widget->RegisterEventCallback(ui::EventMain_Decide, new ControlButtonCB(this), 0);

		welement.hash = psvs_button_impose_bus_up;
		widget = this->root->GetChild(&welement, 0);
		widget->RegisterEventCallback(ui::EventMain_Decide, new ControlButtonCB(this), 0);
		*/

		welement.hash = psvs_button_impose_xbar_down;
		widget = this->root->GetChild(&welement, 0);
		widget->RegisterEventCallback(ui::EventMain_Decide, new ControlButtonCB(this), 0);

		welement.hash = psvs_button_impose_xbar_up;
		widget = this->root->GetChild(&welement, 0);
		widget->RegisterEventCallback(ui::EventMain_Decide, new ControlButtonCB(this), 0);

		welement.hash = psvs_button_impose_hud_up;
		widget = this->root->GetChild(&welement, 0);
		widget->RegisterEventCallback(ui::EventMain_Decide, new ControlButtonCB(this), 0);

		welement.hash = psvs_button_impose_hud_down;
		widget = this->root->GetChild(&welement, 0);
		widget->RegisterEventCallback(ui::EventMain_Decide, new ControlButtonCB(this), 0);

		welement.hash = psvs_button_impose_hud_pos_up;
		widget = this->root->GetChild(&welement, 0);
		widget->RegisterEventCallback(ui::EventMain_Decide, new ControlButtonCB(this), 0);

		welement.hash = psvs_button_impose_hud_pos_down;
		widget = this->root->GetChild(&welement, 0);
		widget->RegisterEventCallback(ui::EventMain_Decide, new ControlButtonCB(this), 0);

		welement.hash = psvs_button_impose_cas_up;
		widget = this->root->GetChild(&welement, 0);
		widget->RegisterEventCallback(ui::EventMain_Decide, new ControlButtonCB(this), 0);

		welement.hash = psvs_button_impose_cas_down;
		widget = this->root->GetChild(&welement, 0);
		widget->RegisterEventCallback(ui::EventMain_Decide, new ControlButtonCB(this), 0);

		welement.hash = psvs_button_impose_delete;
		widget = this->root->GetChild(&welement, 0);
		widget->RegisterEventCallback(ui::EventMain_Decide, new ProfileButtonCB(widget), 0);
		if (psvs::tracker::GetCurrentProfile()) {
			widget->Enable(true);
		}

		ProfileButtonCB *cb = new ProfileButtonCB(widget);
		welement.hash = psvs_button_impose_save;
		widget = this->root->GetChild(&welement, 0);
		widget->RegisterEventCallback(ui::EventMain_Decide, cb, 0);

		welement.hash = psvs_check_box_impose_cpu_lock;
		widget = this->root->GetChild(&welement, 0);
		widget->RegisterEventCallback(ui::EventMain_Decide, new LockCB(), 0);
		if (psvsClockFrequencyIsLockedProc(pid, PSVS_LOCK_DEVICE_CPU)) {
			box = (ui::CheckBox *)widget;
			box->SetChecked(0.0f, true, 0);
		}

		welement.hash = psvs_check_box_impose_gpu_lock;
		widget = this->root->GetChild(&welement, 0);
		widget->RegisterEventCallback(ui::EventMain_Decide, new LockCB(), 0);
		if (psvsClockFrequencyIsLockedProc(pid, PSVS_LOCK_DEVICE_GPU_ES4)) {
			box = (ui::CheckBox *)widget;
			box->SetChecked(0.0f, true, 0);
		}

		/*
		welement.hash = psvs_check_box_impose_bus_lock;
		widget = this->root->GetChild(&welement, 0);
		widget->RegisterEventCallback(ui::EventMain_Decide, new LockCB(), 0);
		if (psvsClockFrequencyIsLockedProc(pid, PSVS_LOCK_DEVICE_BUS)) {
			box = (ui::CheckBox *)widget;
			box->SetChecked(0.0f, true, 0);
		}
		*/

		welement.hash = psvs_check_box_impose_xbar_lock;
		widget = this->root->GetChild(&welement, 0);
		widget->RegisterEventCallback(ui::EventMain_Decide, new LockCB(), 0);
		if (psvsClockFrequencyIsLockedProc(pid, PSVS_LOCK_DEVICE_GPU_XBAR)) {
			box = (ui::CheckBox *)widget;
			box->SetChecked(0.0f, true, 0);
		}

		task::Register(Update, this);
	}

	Impose::~Impose()
	{
		task::Unregister(Update, this);
	}
}