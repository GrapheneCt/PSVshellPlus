#pragma once

#include <kernel.h>
#include <paf.h>

#include "psvs.h"
#include "hud.h"

namespace psvs
{
	class Impose
	{
	public:

		#define PSVS_IMPOSE_UPDATE_WINDOW_USEC	(100000)

		enum HudMode
		{
			HudMode_Off,
			HudMode_Micro,
			HudMode_Mini,
			HudMode_Full,
			HudMode_Dev
		};

		enum Hash
		{
			psvs_template_impose = 0x1986fda5,
			psvs_template_impose_2 = 0x6825067c,
			psvs_text_impose_cpu_value = 0x480b4ef2,
			psvs_text_impose_gpu_value = 0x0b6cea96,
			psvs_text_impose_bus_value = 0xe0749e46,
			psvs_text_impose_xbar_value = 0xa3affcc2,
			psvs_button_impose_cpu_down = 0x6059f1bb,
			psvs_button_impose_cpu_up = 0x90fb2648,
			psvs_button_impose_gpu_down = 0xe503c992,
			psvs_button_impose_gpu_up = 0x81c4d74b,
			psvs_button_impose_bus_down = 0xa8d809ea,
			psvs_button_impose_bus_up = 0x402fe552,
			psvs_button_impose_xbar_down = 0xe1dcd852,
			psvs_button_impose_xbar_up = 0x63977f99,
			psvs_check_box_impose_cpu_lock = 0x44908086,
			psvs_check_box_impose_gpu_lock = 0xd39ce0ad,
			psvs_check_box_impose_bus_lock = 0xd522f917,
			psvs_check_box_impose_xbar_lock = 0xf5974793,
			psvs_button_impose_save = 0xf36ed842,
			psvs_button_impose_delete = 0x11f27c8a,
			psvs_button_impose_hud_down = 0x0071ecee,
			psvs_button_impose_hud_up = 0x88aa0668,
			psvs_button_impose_hud_pos_up = 0xe0d3e2ae,
			psvs_button_impose_hud_pos_down = 0xa5be81ad,
			psvs_text_impose_hud_pos_value = 0x3a6de20e,
			psvs_button_impose_cas_down = 0xed48e0d3,
			psvs_button_impose_cas_up = 0x0eea6bfa,
			psvs_text_impose_hud_value = 0xc7be30c1,
			psvs_text_impose_cas_value = 0xd0bd439b
		};

		static void ControlButtonCBFun(int32_t type, paf::ui::Handler *self, paf::ui::Event *e, void *userdata);
		static void ProfileButtonCBFun(int32_t type, paf::ui::Handler *self, paf::ui::Event *e, void *userdata);
		static void LockCBFun(int32_t type, paf::ui::Handler *self, paf::ui::Event *e, void *userdata);

		static void Update(void *arg);
		static Hud::Position GetHudPosition();
		static void SetHudPosition(Hud::Position pos);

		Impose(paf::Plugin *plugin, paf::ui::Box *root);

		~Impose();

	protected:

		paf::Plugin *plugin;
		paf::Plugin *imposePlugin;
		paf::ui::Box *root;

		paf::ui::Text *cpu;
		paf::ui::Text *gpu;
		paf::ui::Text *bus;
		paf::ui::Text *xbar;
		paf::ui::Text *hud;
		paf::ui::Text *hudpos;
		paf::ui::Text *cas;

		PSVSClockFrequency oldClock;
		uint32_t oldTick;
	};
}