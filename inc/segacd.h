/*
 * 240p Test Suite for the Sega 32X
 * Port by Dasutin (Dustin Dembrosky)
 * Copyright (C)2011-2026 Artemio Urbina
 *
 * This file is part of the 240p Test Suite
 *
 * The 240p Test Suite is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * The 240p Test Suite is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with 240p Test Suite; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _SEGACD_H_
#define _SEGACD_H_

#include "types.h"

#define SCD_STATUS_PRESENT     0x0001
#define SCD_STATUS_INITIALIZED 0x0002
#define SCD_STATUS_BUSY        0x0004

enum SegaCDOperation {
	SCD_OP_STATUS = 0,
	SCD_OP_INIT,
	SCD_OP_RESULT0_LO,
	SCD_OP_RESULT0_HI,
	SCD_OP_RESULT1_LO,
	SCD_OP_RESULT1_HI,
	SCD_OP_RESULT2,
	SCD_OP_BIOS_WORD,
	SCD_OP_HINT_TEST,
	SCD_OP_FLAG_TEST,
	SCD_OP_COMM_TEST,
	SCD_OP_PROGRAM_RAM_TEST,
	SCD_OP_WORD_RAM_TEST,
	SCD_OP_MEM_ADDR_HI,
	SCD_OP_MEM_ADDR_LO,
	SCD_OP_MEM_READ_WORD,
	SCD_OP_PCM_RAM_TEST,
	SCD_OP_PCM_CONTROL,
	SCD_OP_DISC_INFO,
	SCD_OP_CDDA_PLAY,
	SCD_OP_CDDA_STOP,
	SCD_OP_DATA_SET_LBA,
	SCD_OP_DATA_START,
	SCD_OP_DATA_POLL,
	SCD_OP_WORD_RAM_READ,
	SCD_OP_WORD_RAM_STRESS,
	SCD_OP_SUBCPU_PING,
	SCD_OP_YM_CONTROL,
	SCD_OP_RESET,
	SCD_OP_MEM_BANK,
	SCD_OP_PCM_RAM_SET,
	SCD_OP_PCM_RAM_COMPARE,
	SCD_OP_TRACK_INFO,
	SCD_OP_TRAY_OPEN,
	SCD_OP_TRAY_CLOSE,
	SCD_OP_INIT_INFO,
	SCD_OP_PCM_LOAD,
	SCD_OP_PCM_CENTER,
	SCD_OP_DISC_ID_PARSE,
	SCD_OP_DISC_ID_FIELD,
	SCD_OP_YM_TEST
};

#define SCD_YM_TEST_INIT 0x0001
#define SCD_YM_TEST_PLAY 0x8000

enum SegaCDDiscIDField {
	SCD_DISC_ID_DOMESTIC = 0,
	SCD_DISC_ID_OVERSEAS,
	SCD_DISC_ID_PRODUCT,
	SCD_DISC_ID_REGION
};

#define SCD_DISC_ID_TITLE_SIZE   49
#define SCD_DISC_ID_PRODUCT_SIZE 15
#define SCD_DISC_ID_REGION_SIZE  17

enum SegaCDInitInfo {
	SCD_INIT_INFO_BIOS_ADDRESS = 0,
	SCD_INIT_INFO_PROGRAM_SIZE,
	SCD_INIT_INFO_SP_INIT_WAIT,
	SCD_INIT_INFO_SP_MAIN_WAIT,
	SCD_INIT_INFO_FAILURE
};

u16 HwMdSegaCDCommand(u16 operation, u16 argument);
u32 HwMdSegaCDResult0(void);
u32 HwMdSegaCDResult1(void);
u16 HwMdSegaCDResult2(void);

void menu_segacd(void);
void menu_segacd32x(void);

#endif
