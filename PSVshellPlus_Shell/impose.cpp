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
	static int32_t s_hudMode = Impose::HudMode_Off;
	static int32_t s_hudPos = Hud::Position_UpLeft;
	static int32_t s_cas = -0x1000;

	const int32_t k_cpuClock[] = { 41, 83, 111, 166, 222, 333, 444, 500 };
	const int32_t k_gpuClock[] = { 41, 55, 83, 111, 166, 222 };
	const int32_t k_busClock[] = { 55, 83, 111, 166, 222 };
	const int32_t k_xbarClock[] = { 83, 111, 166 };

	void Impose::ControlButtonCBFun(int32_t type, ui::Handler *self, ui::Event *e, void *userdata)
	{
		wstring wstr;
		wchar_t casVal[2];
		int32_t arraySize = 0;
		const int32_t *arrayPtr = NULL;
		int32_t direction = 0;
		int32_t currentClock = 0;
		Impose *obj = (Impose *)userdata;
		ui::Widget *wdg = (ui::Widget *)self;
		Hud *currHud = NULL;
		int32_t(*clockSet)(int clock) = NULL;
		PSVSClockFrequency clock;
		psvsGetClockFrequency(&clock);
			 
		switch (wdg->GetName().GetIDHash()) {
		case psvs_button_impose_cpu_down:
			currentClock = clock.cpu;
			clockSet = psvsSetArmClockFrequency;
			direction = -1;
			arraySize = sizeof(k_cpuClock) / sizeof(int32_t);
			arrayPtr = k_cpuClock;
			break;
		case psvs_button_impose_cpu_up:
			currentClock = clock.cpu;
			clockSet = psvsSetArmClockFrequency;
			direction = 1;
			arraySize = sizeof(k_cpuClock) / sizeof(int32_t);
			arrayPtr = k_cpuClock;
			break;
		case psvs_button_impose_gpu_down:
			currentClock = clock.gpu;
			clockSet = psvsSetGpuClockFrequency;
			direction = -1;
			arraySize = sizeof(k_gpuClock) / sizeof(int32_t);
			arrayPtr = k_gpuClock;
			break;
		case psvs_button_impose_gpu_up:
			currentClock = clock.gpu;
			clockSet = psvsSetGpuClockFrequency;
			direction = 1;
			arraySize = sizeof(k_gpuClock) / sizeof(int32_t);
			arrayPtr = k_gpuClock;
			break;
			/*
		case psvs_button_impose_bus_down:
			currentClock = clock.bus;
			clockSet = psvsSetBusClockFrequency;
			direction = -1;
			arraySize = sizeof(k_busClock) / sizeof(int32_t);
			arrayPtr = k_busClock;
			break;
		case psvs_button_impose_bus_up:
			currentClock = clock.bus;
			clockSet = psvsSetBusClockFrequency;
			direction = 1;
			arraySize = sizeof(k_busClock) / sizeof(int32_t);
			arrayPtr = k_busClock;
			break;
			*/
		case psvs_button_impose_xbar_down:
			currentClock = clock.xbar;
			clockSet = psvsSetGpuXbarClockFrequency;
			direction = -1;
			arraySize = sizeof(k_xbarClock) / sizeof(int32_t);
			arrayPtr = k_xbarClock;
			break;
		case psvs_button_impose_xbar_up:
			currentClock = clock.xbar;
			clockSet = psvsSetGpuXbarClockFrequency;
			direction = 1;
			arraySize = sizeof(k_xbarClock) / sizeof(int32_t);
			arrayPtr = k_xbarClock;
			break;
		case psvs_button_impose_hud_down:
			if (s_hudMode != HudMode_Off) {
				s_hudMode--;
			}
			else {
				break;
			}
			thread::RMutex::main_thread_mutex.Lock();
			currHud = Hud::GetCurrentHud();
			if (currHud != NULL) {
				delete currHud;
			}
			switch (s_hudMode) {
			case HudMode_Off:
				wstr = g_corePlugin->GetString("psvs_msg_hud_off");
				break;
			case HudMode_Micro:
				wstr = g_corePlugin->GetString("psvs_msg_hud_micro");
				currHud = new HudMicro();
				currHud->SetPosition((Hud::Position)s_hudPos);
				break;
			case HudMode_Mini:
				wstr = g_corePlugin->GetString("psvs_msg_hud_mini");
				currHud = new HudMini();
				currHud->SetPosition((Hud::Position)s_hudPos);
				break;
			case HudMode_Full:
				wstr = g_corePlugin->GetString("psvs_msg_hud_full");
				currHud = new HudFull();
				currHud->SetPosition((Hud::Position)s_hudPos);
				break;
			case HudMode_Dev:
				wstr = g_corePlugin->GetString("psvs_msg_hud_dev");
				currHud = new HudDev();
				currHud->SetPosition((Hud::Position)s_hudPos);
				break;
			}
			obj->hud->SetString(wstr);
			thread::RMutex::main_thread_mutex.Unlock();
			break;
		case psvs_button_impose_hud_up:
			if (s_hudMode != HudMode_Dev) {
				s_hudMode++;
			}
			else {
				break;
			}
			thread::RMutex::main_thread_mutex.Lock();
			currHud = Hud::GetCurrentHud();
			if (currHud != NULL) {
				delete currHud;
			}
			switch (s_hudMode) {
			case HudMode_Off:
				wstr = g_corePlugin->GetString("psvs_msg_hud_off");
				break;
			case HudMode_Micro:
				wstr = g_corePlugin->GetString("psvs_msg_hud_micro");
				currHud = new HudMicro();
				currHud->SetPosition((Hud::Position)s_hudPos);
				break;
			case HudMode_Mini:
				wstr = g_corePlugin->GetString("psvs_msg_hud_mini");
				currHud = new HudMini();
				currHud->SetPosition((Hud::Position)s_hudPos);
				break;
			case HudMode_Full:
				wstr = g_corePlugin->GetString("psvs_msg_hud_full");
				currHud = new HudFull();
				currHud->SetPosition((Hud::Position)s_hudPos);
				break;
			case HudMode_Dev:
				wstr = g_corePlugin->GetString("psvs_msg_hud_dev");
				currHud = new HudDev();
				currHud->SetPosition((Hud::Position)s_hudPos);
				break;
			}
			obj->hud->SetString(wstr);
			thread::RMutex::main_thread_mutex.Unlock();
			break;
		case psvs_button_impose_hud_pos_down:
			currHud = Hud::GetCurrentHud();
			if (s_hudPos != Hud::Position_UpLeft) {
				s_hudPos--;
			}
			else {
				break;
			}
			thread::RMutex::main_thread_mutex.Lock();
			switch (s_hudPos) {
			case Hud::Position_UpLeft:
				wstr = g_corePlugin->GetString("psvs_msg_hud_pos_ul");
				break;
			case Hud::Position_UpRight:
				wstr = g_corePlugin->GetString("psvs_msg_hud_pos_ur");
				break;
			case Hud::Position_DownLeft:
				wstr = g_corePlugin->GetString("psvs_msg_hud_pos_dl");
				break;
			case Hud::Position_DownRight:
				wstr = g_corePlugin->GetString("psvs_msg_hud_pos_dr");
				break;
			}
			obj->hudpos->SetString(wstr);
			if (currHud != NULL)
				currHud->SetPosition((Hud::Position)s_hudPos);
			thread::RMutex::main_thread_mutex.Unlock();
			break;
		case psvs_button_impose_hud_pos_up:
			currHud = Hud::GetCurrentHud();
			if (s_hudPos != Hud::Position_DownRight) {
				s_hudPos++;
			}
			else {
				break;
			}
			thread::RMutex::main_thread_mutex.Lock();
			switch (s_hudPos) {
			case Hud::Position_UpLeft:
				wstr = g_corePlugin->GetString("psvs_msg_hud_pos_ul");
				break;
			case Hud::Position_UpRight:
				wstr = g_corePlugin->GetString("psvs_msg_hud_pos_ur");
				break;
			case Hud::Position_DownLeft:
				wstr = g_corePlugin->GetString("psvs_msg_hud_pos_dl");
				break;
			case Hud::Position_DownRight:
				wstr = g_corePlugin->GetString("psvs_msg_hud_pos_dr");
				break;
			}
			obj->hudpos->SetString(wstr);
			if (currHud != NULL)
				currHud->SetPosition((Hud::Position)s_hudPos);
			thread::RMutex::main_thread_mutex.Unlock();
			break;
		case psvs_button_impose_cas_down:
			if (s_cas >= 0) {
				s_cas -= 0x1000;
			}
			else {
				break;
			}
			thread::RMutex::main_thread_mutex.Lock();
			if (s_cas < 0) {
				wstr = g_corePlugin->GetString("psvs_msg_auto");
			}
			else {
				sce_paf_swprintf(casVal, 2, L"%d", s_cas / 0x1000);
				wstr = casVal;
			}
			obj->cas->SetString(wstr);
			Hud::SetCasShift(s_cas);
			thread::RMutex::main_thread_mutex.Unlock();
			break;
		case psvs_button_impose_cas_up:
			if (s_cas != 0x7000) {
				s_cas += 0x1000;
			}
			else {
				break;
			}
			thread::RMutex::main_thread_mutex.Lock();
			sce_paf_swprintf(casVal, 2, L"%d", s_cas / 0x1000);
			wstr = casVal;
			obj->cas->SetString(wstr);
			Hud::SetCasShift(s_cas);
			thread::RMutex::main_thread_mutex.Unlock();
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

	void Impose::ProfileButtonCBFun(int32_t type, ui::Handler *self, ui::Event *e, void *userdata)
	{
		ui::Button *wdg = (ui::Button *)self;
		uint32_t hash = wdg->GetName().GetIDHash();

		if (hash == psvs_button_impose_save) {
			psvs::Profile *profile = new psvs::Profile();
			profile->Save();
			delete profile;

			tracker::ReloadCurrentProfile();
			wdg->Enable(true);
		}
		else if (hash == psvs_button_impose_delete) {
			psvs::Profile::Delete();
			tracker::ReloadCurrentProfile();
			wdg->Disable(true);
		}
	}

	void Impose::LockCBFun(int32_t type, ui::Handler *self, ui::Event *e, void *userdata)
	{
		ui::CheckBox *wdg = (ui::CheckBox *)self;
		ScePID pid = psvs::tracker::GetCurrentPID();
		PsvsLockDevice target = PSVS_LOCK_DEVICE_NONE;

		switch (wdg->GetName().GetIDHash()) {
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

		if (wdg->IsChecked())
			psvsClockFrequencyLockProc(pid, target);
		else
			psvsClockFrequencyUnlockProc(pid, target);
	}

	void Impose::Update(void *arg)
	{
		PSVSClockFrequency clock;
		wstring wtext;
		wchar_t wbuf[16];
		uint32_t tickNow = sceKernelGetProcessTimeLow();
		Impose *obj = (Impose *)arg;

		if (tickNow - obj->oldTick > PSVS_IMPOSE_UPDATE_WINDOW_USEC) {
			if (!g_imposeRoot->FindChild(0x0EE0C8AF)) {
				delete obj;
				return;
			}

			//Update clock
			psvsGetClockFrequency(&clock);

			if (obj->oldClock.cpu != clock.cpu) {
				sce_paf_swprintf(wbuf, 16, L"%d MHz", clock.cpu);
				wtext = wbuf;
				obj->cpu->SetString(wtext);
			}

			if (obj->oldClock.gpu != clock.gpu) {
				sce_paf_swprintf(wbuf, 16, L"%d MHz", clock.gpu);
				wtext = wbuf;
				obj->gpu->SetString(wtext);
			}

			/*
			if (obj->oldClock.bus != clock.bus) {
				sce_paf_swprintf(wbuf, sizeof(wbuf), L"%d MHz", clock.bus);
				wtext = wbuf;
				obj->bus->SetString(wtext);
			}
			*/

			if (obj->oldClock.xbar != clock.xbar) {
				sce_paf_swprintf(wbuf, 16, L"%d MHz", clock.xbar);
				wtext = wbuf;
				obj->xbar->SetString(wtext);
			}

			obj->oldTick = tickNow;
			obj->oldClock = clock;
		}
	}

	Hud::Position Impose::GetHudPosition()
	{
		return (Hud::Position)s_hudPos;
	}

	void Impose::SetHudPosition(Hud::Position pos)
	{
		s_hudPos = pos;
		Hud *currHud = Hud::GetCurrentHud();
		if (currHud != NULL) {
			currHud->SetPosition(pos);
		}
	}

	Impose::Impose(Plugin *plugin, ui::Box *root)
	{
		wstring wstr;
		wchar_t casVal[2];
		math::v4 sz;
		Plugin::TemplateOpenParam tmpParam;
		ScePID pid = psvs::tracker::GetCurrentPID();

		this->oldTick = 0;
		this->plugin = plugin;
		this->root = root;
		this->imposePlugin = Plugin::Find("impose_plugin");
		sce_paf_memset(&this->oldClock, 0, sizeof(PSVSClockFrequency));

		ui::Widget *sep0 = this->imposePlugin->CreateWidget(this->root, "plane", 0x7ac075f7, 0x26c7781e);
		sz.set_x(835.0f);
		sz.set_y(20.0f);
		sep0->SetSize(sz);

		this->plugin->TemplateOpen(this->root, psvs_template_impose, tmpParam);

		ui::Widget *sep1 = this->imposePlugin->CreateWidget(this->root, "plane", 0x7ac075f7, 0x26c7781e);
		sz.set_x(835.0f);
		sz.set_y(20.0f);
		sep1->SetSize(sz);

		this->plugin->TemplateOpen(this->root, psvs_template_impose_2, tmpParam);
		this->cpu = (ui::Text *)this->root->FindChild(psvs_text_impose_cpu_value);
		this->gpu = (ui::Text *)this->root->FindChild(psvs_text_impose_gpu_value);
		/*
		this->bus = (ui::Text *)this->root->FindChild(psvs_text_impose_bus_value);
		*/
		this->xbar = (ui::Text *)this->root->FindChild(psvs_text_impose_xbar_value);
		this->hud = (ui::Text *)this->root->FindChild(psvs_text_impose_hud_value);

		switch (s_hudMode) {
		case HudMode_Off:
			wstr = g_corePlugin->GetString("psvs_msg_hud_off");
			break;
		case HudMode_Micro:
			wstr = g_corePlugin->GetString("psvs_msg_hud_micro");
			break;
		case HudMode_Mini:
			wstr = g_corePlugin->GetString("psvs_msg_hud_mini");
			break;
		case HudMode_Full:
			wstr = g_corePlugin->GetString("psvs_msg_hud_full");
			break;
		case HudMode_Dev:
			wstr = g_corePlugin->GetString("psvs_msg_hud_dev");
			break;
		}
		this->hud->SetString(wstr);

		this->hudpos = (ui::Text *)this->root->FindChild(psvs_text_impose_hud_pos_value);

		switch (s_hudPos) {
		case Hud::Position_UpLeft:
			wstr = g_corePlugin->GetString("psvs_msg_hud_pos_ul");
			break;
		case Hud::Position_UpRight:
			wstr = g_corePlugin->GetString("psvs_msg_hud_pos_ur");
			break;
		case Hud::Position_DownLeft:
			wstr = g_corePlugin->GetString("psvs_msg_hud_pos_dl");
			break;
		case Hud::Position_DownRight:
			wstr = g_corePlugin->GetString("psvs_msg_hud_pos_dr");
			break;
		}
		this->hudpos->SetString(wstr);

		this->cas = (ui::Text *)this->root->FindChild(psvs_text_impose_cas_value);

		if (s_cas < 0) {
			wstr = g_corePlugin->GetString("psvs_msg_auto");
		}
		else {
			sce_paf_swprintf(casVal, 2, L"%d", s_cas / 0x1000);
			wstr = casVal;
		}
		this->cas->SetString(wstr);

		ui::Widget *widget = NULL;
		ui::Button *bt = NULL;
		ui::CheckBox *box = NULL;

		widget = this->root->FindChild(psvs_button_impose_cpu_down);
		widget->AddEventCallback(ui::Button::CB_BTN_DECIDE, ControlButtonCBFun, this);
		widget = this->root->FindChild(psvs_button_impose_cpu_up);
		widget->AddEventCallback(ui::Button::CB_BTN_DECIDE, ControlButtonCBFun, this);
		widget = this->root->FindChild(psvs_button_impose_gpu_down);
		widget->AddEventCallback(ui::Button::CB_BTN_DECIDE, ControlButtonCBFun, this);
		widget = this->root->FindChild(psvs_button_impose_gpu_up);
		widget->AddEventCallback(ui::Button::CB_BTN_DECIDE, ControlButtonCBFun, this);
		/*
		widget = this->root->FindChild(psvs_button_impose_bus_down);
		widget->AddEventCallback(ui::Button::CB_BTN_DECIDE, ControlButtonCBFun, this);
		widget = this->root->FindChild(psvs_button_impose_bus_up);
		widget->AddEventCallback(ui::Button::CB_BTN_DECIDE, ControlButtonCBFun, this);
		*/
		widget = this->root->FindChild(psvs_button_impose_xbar_down);
		widget->AddEventCallback(ui::Button::CB_BTN_DECIDE, ControlButtonCBFun, this);
		widget = this->root->FindChild(psvs_button_impose_xbar_up);
		widget->AddEventCallback(ui::Button::CB_BTN_DECIDE, ControlButtonCBFun, this);
		widget = this->root->FindChild(psvs_button_impose_hud_up);
		widget->AddEventCallback(ui::Button::CB_BTN_DECIDE, ControlButtonCBFun, this);
		widget = this->root->FindChild(psvs_button_impose_hud_down);
		widget->AddEventCallback(ui::Button::CB_BTN_DECIDE, ControlButtonCBFun, this);
		widget = this->root->FindChild(psvs_button_impose_hud_pos_up);
		widget->AddEventCallback(ui::Button::CB_BTN_DECIDE, ControlButtonCBFun, this);
		widget = this->root->FindChild(psvs_button_impose_hud_pos_down);
		widget->AddEventCallback(ui::Button::CB_BTN_DECIDE, ControlButtonCBFun, this);
		widget = this->root->FindChild(psvs_button_impose_cas_up);
		widget->AddEventCallback(ui::Button::CB_BTN_DECIDE, ControlButtonCBFun, this);
		widget = this->root->FindChild(psvs_button_impose_cas_down);
		widget->AddEventCallback(ui::Button::CB_BTN_DECIDE, ControlButtonCBFun, this);
		widget = this->root->FindChild(psvs_button_impose_delete);
		widget->AddEventCallback(ui::Button::CB_BTN_DECIDE, ProfileButtonCBFun, this);
		if (psvs::tracker::GetCurrentProfile()) {
			bt = (ui::Button *)widget;
			bt->Enable(true);
		}
		widget = this->root->FindChild(psvs_button_impose_save);
		widget->AddEventCallback(ui::Button::CB_BTN_DECIDE, ProfileButtonCBFun, this);
		widget = this->root->FindChild(psvs_check_box_impose_cpu_lock);
		widget->AddEventCallback(ui::Button::CB_BTN_DECIDE, LockCBFun, this);
		if (psvsClockFrequencyIsLockedProc(pid, PSVS_LOCK_DEVICE_CPU)) {
			box = (ui::CheckBox *)widget;
			box->SetCheck(true);
		}
		widget = this->root->FindChild(psvs_check_box_impose_gpu_lock);
		widget->AddEventCallback(ui::Button::CB_BTN_DECIDE, LockCBFun, this);
		if (psvsClockFrequencyIsLockedProc(pid, PSVS_LOCK_DEVICE_GPU_ES4)) {
			box = (ui::CheckBox *)widget;
			box->SetCheck(true);
		}
		/*
		widget = this->root->FindChild(psvs_check_box_impose_bus_lock);
		widget->AddEventCallback(ui::Button::CB_BTN_DECIDE, LockCBFun, this);
		if (psvsClockFrequencyIsLockedProc(pid, PSVS_LOCK_DEVICE_BUS)) {
			box = (ui::CheckBox *)widget;
			box->SetCheck(true);
		}
		*/
		widget = this->root->FindChild(psvs_check_box_impose_xbar_lock);
		widget->AddEventCallback(ui::Button::CB_BTN_DECIDE, LockCBFun, this);
		if (psvsClockFrequencyIsLockedProc(pid, PSVS_LOCK_DEVICE_GPU_XBAR)) {
			box = (ui::CheckBox *)widget;
			box->SetCheck(true);
		}

		common::MainThreadCallList::Register(Update, this);
	}

	Impose::~Impose()
	{
		common::MainThreadCallList::Unregister(Update, this);
	}
}