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

#ifndef _TYPES_H_
#define _TYPES_H_

#include <stdint.h>
#include <stdlib.h>
#include <limits.h>

#include "fixed.h"

#ifndef NULL
#define NULL 0
#endif

#define ATTR_CACHE_ALIGNED  __attribute__((aligned(16)))
#define ATTR_DATA_ALIGNED   __attribute__((section(".data"), aligned(16)))

enum {
	DRAWSPR_NORM        = 0,
	DRAWSPR_HFLIP       = 1,
	DRAWSPR_VFLIP       = 2,

	DRAWSPR_OVERWRITE   = 4,
	DRAWSPR_PRECISE     = 8,
	DRAWSPR_SCALE       = 16,
	DRAWSPR_MULTICORE   = 32,
};

typedef struct {
	int16_t x, y;
} point_t;

typedef struct {
	int16_t x1, y1;
	int16_t x2, y2;
} rect_t;

typedef struct {
	int offset[2];
	fixed_t parallax[2];
	char *bitmap;
	uint16_t *tiles;
	int objectLayer;
} dtilelayer_t;

// "on-disk" tilemap
typedef struct {
	int tilew, tileh;
	int numtw, numth;
	int numlayers;
	int wrapX, wrapY;
	dtilelayer_t *layers;
	int mdPriority;
	dtilelayer_t mdPlaneA, mdPlaneB;
} dtilemap_t;

// in-memory tilemap
typedef struct {
	unsigned tw, th;
	fixed_t wrapX, wrapY;

	int numtiles;
	uint16_t id;
	uint16_t numlayers;

	dtilelayer_t *layers;
	dtilelayer_t *mdPlane[2];

	uint8_t **reslist;

	unsigned tiles_hor, tiles_ver;
	unsigned canvas_tiles_hor, canvas_tiles_ver;
	unsigned scroll_tiles_hor, scroll_interval_hor;
	unsigned scroll_tiles_ver, scroll_interval_ver;
} tilemap_t;

typedef struct {
	void *sdata;
	uint16_t flags;
	int16_t sx, sy;
	uint16_t sw, sh;
	uint16_t x, y;
	uint16_t w, h;
	fixed_t scale;
} drawsprcmd_t;

typedef struct {
	void* sdata;
	uint16_t flags;
	int16_t sx, sy;
	uint16_t sw, sh;
	uint16_t x1, y1;
	uint16_t x2, y2;
	uint16_t x3, y3;
	uint16_t x4, y4;
	uint16_t w, h;
	fixed_t scale;
} drawspr4cmd_t;

typedef struct {
	tilemap_t* tm;
	int16_t startlayer;
	uint16_t parallax;
	int32_t camera_x, camera_y;
	int16_t x, y;
	uint16_t start_tile, end_tile;
	uint16_t scroll_tile_id;
	uint16_t num_tiles_x;
	uint16_t drawmode;
} drawtilelayerscmd_t;

typedef void(*draw_spritefn_t)(void *dst, drawsprcmd_t* cmd);

extern const uint8_t yatssd_empty_tile[32 * 32];

//typedef unsigned long int size_t;

typedef unsigned char u8;
typedef unsigned short int u16;
typedef unsigned long int u32;

typedef signed char s8;
typedef signed short int s16;
typedef signed long int s32;

typedef volatile unsigned char vu8;
typedef volatile unsigned short int vu16;
typedef volatile unsigned long int vu32;

typedef volatile signed char v8;
typedef volatile signed short int v16;
typedef volatile signed long int v32;

typedef s32 fix32;

#define abs(n) ((n)<0?-(n):(n))

#endif /* _TYPES_H_ */
