/* 
 * 240p Test Suite for the Sega 32X
 * Port by Dasutin
 * Copyright (C)2011-2022 Artemio Urbina
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

#ifndef _TESTS_H
#define _TESTS_H

#define MEMORY_OK 0xFFFFFFFF

void ShowMessageAndData(char *message, u32 address, u8 color, int len, int xpos, int ypos);

void vt_drop_shadow_test(void);
void vt_striped_sprite_test(void);
void vt_horizontal_stripes(void);
void vt_vertical_stripes(void);
void vt_checkerboard(void);
void vt_backlitzone_test(void);
void at_sound_test(void);
void ht_controller_test(void);
void ht_memory_viewer(u32 address);
void ht_check_32x_bios_crc(u32 address);
void ht_test_32x_sdram();

#endif