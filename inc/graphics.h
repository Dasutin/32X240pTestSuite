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

#ifndef _GRAPHICS_H_
#define _GRAPHICS_H_

#include "types.h"
#include "32x.h"

// For a given details value 0 - 63 returns the equivalent of sin((d/10)). Precalculated.
double xSinTable(const int details);

// For a given details value 0 - 63 returns the equivalent of sin((d/10)+(pi/2)). Precalculated.
double ySinTable(const int details);

// Draws an elipse
void circle2(const s16 height, const s16 width, const s16 rx, const s16 ry, const s16 r, vu8 *spriteBuffer, s16 colorIndex);

#endif /* _GRAPHICS_H_ */
