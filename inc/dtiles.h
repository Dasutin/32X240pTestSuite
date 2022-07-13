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

#ifndef DTILES_H_
#define DTILES_H_

//drawtilelayerscmd_t slave_drawtilelayerscmd;
//extern drawtilelayerscmd_t slave_drawtilelayerscmd;

static int old_camera_x, old_camera_y;
static int main_camera_x, main_camera_y;

static int camera_x, camera_y;

static int draw_tile_layer(tilemap_t *tm, int layer, int fpcamera_x, 
int fpcamera_y, int numlayers, int *pclipped)
ATTR_DATA_ALIGNED;

#endif /* _DTILES_H_ */