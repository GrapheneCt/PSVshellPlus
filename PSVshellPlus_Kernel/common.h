#pragma once

#define SYSCALL_ENTER		unsigned int scstate = __builtin_mrc(15, 0, 13, 0, 3); \
							__builtin_mcr(15, 0, 13, 0, 3, scstate << 16);

#define SYSCALL_EXIT		__builtin_mcr(15, 0, 13, 0, 3, scstate);

#define SCE_KERNEL_PARTITION_OWNER_APP			(0x00000)
#define SCE_KERNEL_PARTITION_OWNER_SYSTEM		(0x00002)
#define SCE_KERNEL_PARTITION_OWNER_GAME			(0x00004)

#define SCE_KERNEL_PARTITION_MEMTYPE_MAIN		(0x10000)
#define SCE_KERNEL_PARTITION_MEMTYPE_CDRAM		(0x30000)

typedef struct SceKernelPhyMemPart { // size is at least 0x9C on FWs 0.931-3.60
	uint32_t field_0x0;
	uint32_t field_0x4;
	uint32_t some_counter; // always positive
	uint32_t field_0xc;
	char *field_0x10;
	int type; // 0x10000 for Lpddr2Main, 0x30000 for Cdram
	int mutex;
	void* pbase;
	SceSize totalSize;
	int field_0x24;
	void *pRoot;
	uint32_t field_0x2c;
	uint32_t field_0x30;
	uint32_t field_0x34;
	uint32_t field_0x38;
	int status;
	int pindex40; //1
	int field_0x44;
	int pindex48; //2
	int field_0x4c;
	int pindex50; //3
	int field_0x54;
	int pindex58; //4
	int field_0x5c;
	int pindex60; //5
	int field_0x64;
	int pindex68; //6
	int field_0x6c;
	int pindex70; //7
	int field_0x74;
	int pindex78; //8
	int field_0x7c;
	int pindex80; //9
	SceSize freeSize;
	int min;
	char name[32]; //10
	int field_0xac;
	SceUInt32 nClient;
	SceSize field_0xb4;
} SceKernelPhyMemPart;

typedef struct ScePhyMemPartInfoCore { // size is 0x10 on FWs 0.990-3.60
	SceUInt32 unk_0; // same as SceKernelPhyMemPart field 0x1C
	SceSize totalSize;
	SceSize freeSize;
	SceUInt32 unk_C; // maybe some counter
} ScePhyMemPartInfoCore;

typedef enum SceDisplayHead {
	SCE_DISPLAY_HEAD_MAIN_LCD_OLED = 0,
	SCE_DISPLAY_HEAD_HDMI = 1,
	SCE_DISPLAY_HEAD_SUB_LCD = 2
} SceDisplayHead;

typedef enum SceDisplayFrameBufType {
	SCE_DISPLAY_FRAMEBUF_GAME_APP = 0,
	SCE_DISPLAY_FRAMEBUF_LIVEAREA = 1 // including HOME and PS overlays
} SceDisplayFrameBufType;

typedef struct PSVSClockFrequency {
	SceInt32 cpu;
	SceInt32 gpu;
	SceInt32 xbar;
	SceInt32 bus;
} PSVSClockFrequency;

typedef struct PSVSMem {
	SceUInt32 mainFree;
	SceUInt32 mainTotal;
	SceUInt32 cdramFree;
	SceUInt32 cdramTotal;
	SceUInt32 phycontFree;
	SceUInt32 phycontTotal;
	SceUInt32 cdialogFree;
	SceUInt32 cdialogTotal;
} PSVSMem;

typedef struct PSVSVenezia {
	SceInt32 core0;
	SceInt32 core1;
	SceInt32 core2;
	SceInt32 core3;
	SceInt32 core4;
	SceInt32 core5;
	SceInt32 core6;
	SceInt32 core7;
	SceInt32 average;
	SceInt32 peak;
} PSVSVenezia;

typedef struct PSVSKPLS {
	SceUInt32 lock;
} PSVSKPLS;

extern SceInt32 g_psvsKpls;

SceUID psvsGetClockingPid();