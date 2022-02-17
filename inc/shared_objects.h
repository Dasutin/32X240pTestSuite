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

#ifndef _SHARED_OBJECTS_
#define _SHARED_OBJECTS_

// Constants

#define MASTER_STATUS_OK 1
#define MASTER_LOCK 4
#define SLAVE_STATUS_OK 2
#define SLAVE_LOCK 8

#define PAUSED  1
#define UNPAUSED 0

#define TRUE 1
#define FALSE 0

extern u8 paused;

extern unsigned short int currentFB;

extern void DrawMainBG(void);
extern void DrawMainBGwGillian(void);
extern void loadTextPalette(void);
extern void cleanup(void);
extern void marsVDP256Start(void);
extern void marsVDP32KStart(void);
extern void swapBuffers(void);
extern void handle_input();
void CRC32_reset();
void CRC32_update(u8 data);
u32 CRC32_finalize();
u32 CalculateCRC(u32 startAddress, u32 size);
int memcmp1(const void *s1, const void *s2, int n);

void setRandomSeed(u16 seed);
u16 random();

#define FIX32_INT_BITS              22
#define FIX32_FRAC_BITS             (32 - FIX32_INT_BITS)

#define FIX32_INT_MASK              (((1 << FIX32_INT_BITS) - 1) << FIX32_FRAC_BITS)
#define FIX32_FRAC_MASK             ((1 << FIX32_FRAC_BITS) - 1)

#define FIX32(value)                ((fix32) ((value) * (1 << FIX32_FRAC_BITS)))

#define fix32Div(val1, val2)        (((val1) << (FIX32_FRAC_BITS / 2)) / ((val2) >> (FIX32_FRAC_BITS / 2)))

#define intToFix32(value)           ((fix32) ((value) << FIX32_FRAC_BITS))
#define fix32ToInt(value)           ((s32) ((value) >> FIX32_FRAC_BITS))
#define fix32Frac(value)            ((value) & FIX32_FRAC_MASK)
#define fix32Mul(val1, val2)        (((val1) >> (FIX32_FRAC_BITS / 2)) * ((val2) >> (FIX32_FRAC_BITS / 2)))
#define fix32Div(val1, val2)        (((val1) << (FIX32_FRAC_BITS / 2)) / ((val2) >> (FIX32_FRAC_BITS / 2)))

#endif
