#pragma once

#include <kernel.h>
#include <paf.h>

namespace psvs
{

	class Hud
	{
	public:

		enum BaseType
		{
			BaseType_None,
			BaseType_Mini,
			BaseType_Full,
		};

		enum Position
		{
			Position_UpLeft,
			Position_UpRight,
			Position_DownLeft,
			Position_DownRight
		};

		enum Hash
		{
			psvs_page_hud = 0x6c5a295a,
			psvs_template_hud_bg_mini = 0x6147e689,
			psvs_template_hud_bg_full = 0x34b71af6,
		};

		static Hud *GetCurrentHud();
		static SceVoid SetCasShift(SceInt32 shift);

		Hud(BaseType type);

		virtual ~Hud();
		virtual SceVoid SetPosition(Position pos) = 0;

	protected:

		paf::ui::Widget *root;

	};

	class HudMicro : public Hud
	{
	public:

		enum Hash
		{
			psvs_template_hud_micro = 0xcd140cb5,
			psvs_text_fps_micro = 0x8d7c94a7,
		};

		static SceVoid Update(ScePVoid arg);

		HudMicro();

		virtual ~HudMicro();
		virtual SceVoid SetPosition(Position pos);

	protected:

		paf::ui::Text *fps;

		SceInt32 oldFps;
	};

	class HudMini : public Hud
	{
	public:

		enum Hash
		{
			psvs_template_hud_mini = 0xf9d6a49a,
			psvs_text_cpu_0 = 0xc1df9fc1,
			psvs_text_cpu_1 = 0x4c9f265a,
			psvs_text_cpu_2 = 0x63d6aaf3,
			psvs_text_cpu_3 = 0xc401e054,
			psvs_text_cpu_peak = 0x4a1b0900,
			psvs_text_fps = 0x7a26c57e,
		};

		static SceVoid Update(ScePVoid arg);

		HudMini(Hud::BaseType type = Hud::BaseType_Mini);

		virtual ~HudMini();
		virtual SceVoid SetPosition(Position pos);

	protected:

		paf::ui::Text *cpuAvg[SCE_KERNEL_MAX_CPU];
		paf::ui::Text *cpuPeak;
		paf::ui::Text *fps;

		SceInt32 oldFps;
	};

	class HudFull : public HudMini
	{
	public:

		#define PSVS_FULL_UPDATE_WINDOW_USEC	(100000)

		enum Hash
		{
			psvs_template_hud_full = 0x367de75c,
			psvs_text_ram_used = 0xe161c101,
			psvs_text_ram_total = 0x588ba69b,
			psvs_text_cdram_used = 0x4561b54f,
			psvs_text_cdram_total = 0x02c2c2ba,
			psvs_text_phy_used = 0x463933d3,
			psvs_text_phy_total = 0x62589882,
			psvs_text_cdlg_used = 0x3ce6a143,
			psvs_text_cdlg_total = 0x937ff95c,
			psvs_text_app = 0xee6b6287,
		};

		static SceVoid Update(ScePVoid arg);

		HudFull();

		virtual ~HudFull();
		virtual SceVoid SetPosition(Position pos);

		static SceVoid SetMemLabel(SceUInt32 used, SceUInt32 total, paf::ui::Text *usedText, paf::ui::Text *totalText);

	protected:

		paf::ui::Text *ramUsed;
		paf::ui::Text *cdramUsed;
		paf::ui::Text *phyUsed;
		paf::ui::Text *cdlgUsed;
		paf::ui::Text *ramTotal;
		paf::ui::Text *cdramTotal;
		paf::ui::Text *phyTotal;
		paf::ui::Text *cdlgTotal;
		paf::ui::Text *app;

		SceUInt32 memTick;
		PSVSMem oldMem;
	};

}