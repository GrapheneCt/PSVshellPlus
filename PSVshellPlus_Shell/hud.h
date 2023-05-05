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
			BaseType_Dev,
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
			psvs_template_hud_bg_dev = 0xddc6d9a0,
		};

		static Hud *GetCurrentHud();
		static void SetCasShift(int32_t shift);

		Hud(BaseType type);

		virtual ~Hud();
		virtual void SetPosition(Position pos) = 0;

	protected:

		paf::ui::Widget *root;
		paf::ui::Scene *coreRoot;

	};

	class HudMicro : public Hud
	{
	public:

		enum Hash
		{
			psvs_template_hud_micro = 0xcd140cb5,
			psvs_text_fps_micro = 0x8d7c94a7,
		};

		static void Update(void *arg);

		HudMicro();

		virtual ~HudMicro();
		virtual void SetPosition(Position pos);

	protected:

		paf::ui::Text *fps;

		int32_t oldFps;
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

		static void Update(void *arg);

		HudMini(Hud::BaseType type = Hud::BaseType_Mini);

		virtual ~HudMini();
		virtual void SetPosition(Position pos);

	protected:

		paf::ui::Text *cpuAvg[SCE_KERNEL_MAX_CPU];
		paf::ui::Text *cpuPeak;
		paf::ui::Text *fps;

		int32_t oldFps;
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

		static void Update(void *arg);

		HudFull();

		virtual ~HudFull();
		virtual void SetPosition(Position pos);

		static void SetMemLabel(uint32_t used, uint32_t total, paf::ui::Text *usedText, paf::ui::Text *totalText);

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

		uint32_t memTick;
		PSVSMem oldMem;
	};

	class HudDev : public HudMini
	{
	public:

		#define PSVS_VNZ_UPDATE_WINDOW_USEC		(1000)
		#define PSVS_BAT_UPDATE_WINDOW_USEC		(500000)

		enum Hash
		{
			psvs_template_hud_dev = 0xf6e17467,
			psvs_text_vnz_0 = 0x7524c263,
			psvs_text_vnz_1 = 0x56c14879,
			psvs_text_vnz_2 = 0x03cd5e60,
			psvs_text_vnz_3 = 0x8c6b209e,
			psvs_text_vnz_4 = 0x98e73415,
			psvs_text_vnz_5 = 0xd8c59975,
			psvs_text_vnz_6 = 0x7cb8ea8d,
			psvs_text_vnz_7 = 0xf646c3a1,
			psvs_text_vnz_peak = 0xebc15716,
			psvs_text_bat_cur = 0x18869f58,
		};

		static void Update(void *arg);

		HudDev();

		virtual ~HudDev();
		virtual void SetPosition(Position pos);

		static void SetMemLabel(uint32_t used, uint32_t total, paf::ui::Text *usedText, paf::ui::Text *totalText);

	protected:

		paf::ui::Text *vnz[8];
		paf::ui::Text *vnzPeak;
		paf::ui::Text *batCur;
		paf::ui::Text *ramUsed;
		paf::ui::Text *cdramUsed;
		paf::ui::Text *phyUsed;
		paf::ui::Text *cdlgUsed;
		paf::ui::Text *ramTotal;
		paf::ui::Text *cdramTotal;
		paf::ui::Text *phyTotal;
		paf::ui::Text *cdlgTotal;
		paf::ui::Text *app;

		uint32_t memTickCommon;
		uint32_t memTickVnzUpd;
		uint32_t memTickBat;
		bool vnzNeedUpdate;
		PSVSMem oldMem;
		PSVSVenezia oldVnz;
		PSVSBattery oldBat;
	};

}