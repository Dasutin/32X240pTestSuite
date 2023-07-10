/*
 * 240p Test Suite for the Sega 32X
 * Port by Dasutin (Dustin Dembrosky)
 * Copyright (C)2011-2023 Artemio Urbina
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

#ifndef _TESTS_H_
#define _TESTS_H_

#include "types.h"

#define MEMORY_OK 0xFFFFFFFF

void ShowMessageAndData(char *message, u32 address, int color, int len, int xpos, int ypos);

void MDPSG_init();
void MDPSG_stop();

void vt_drop_shadow_test(void);
void vt_striped_sprite_test(void);
void vt_lag_test(void);
void vt_reflex_test(void);
void vt_scroll_test(void);
void vt_gridscroll_test(void);
void vt_stripes(void);
void vt_checkerboard(void);
void vt_backlitzone_test(void);
void vt_DisappearingLogo(void);
void at_sound_test(void);
void at_audiosync_test(void);
void ht_controller_test(void);
void ht_memory_viewer(u32 address);
void ht_check_32x_bios_crc(u32 address);
void ht_test_32x_sdram();

typedef struct timecode
{
	u16 hours;
	u16 minutes;
	u16 seconds;
	u16 frames;
	u16 type;
	u16 res;
} timecode;

#endif // _TESTS_H_