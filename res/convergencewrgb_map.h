#ifndef CONVERGENCEWRGB_MAP_H
#define CONVERGENCEWRGB_MAP_H

#include <stdint.h>

const uint16_t convergencewrgb_map_Tiles_Tile_Layer_1[] = {
0x4,0x4,0x8,0xc,0xc,0x10,0x10,0x14,0x18,0x18,0x4,0x4,0x8,0xc,0xc,0x10,0x10,0x14,0x18,0x18,
0x4,0x4,0x8,0xc,0xc,0x10,0x10,0x14,0x18,0x18,0x4,0x4,0x8,0xc,0xc,0x10,0x10,0x14,0x18,0x18,
0x10,0x10,0x14,0x18,0x18,0x4,0x4,0x8,0xc,0xc,0x10,0x10,0x14,0x18,0x18,0x4,0x4,0x8,0xc,0xc,
0x10,0x10,0x14,0x18,0x18,0x4,0x4,0x8,0xc,0xc,0x10,0x10,0x14,0x18,0x18,0x4,0x4,0x8,0xc,0xc,
0x4,0x4,0x8,0xc,0xc,0x10,0x10,0x14,0x18,0x18,0x4,0x4,0x8,0xc,0xc,0x10,0x10,0x14,0x18,0x18,
0x4,0x4,0x8,0xc,0xc,0x10,0x10,0x14,0x18,0x18,0x4,0x4,0x8,0xc,0xc,0x10,0x10,0x14,0x18,0x18,
0x10,0x10,0x14,0x18,0x18,0x4,0x4,0x8,0xc,0xc,0x10,0x10,0x14,0x18,0x18,0x4,0x4,0x8,0xc,0xc,
0x10,0x10,0x14,0x18,0x18,0x4,0x4,0x8,0xc,0xc,0x10,0x10,0x14,0x18,0x18,0x4,0x4,0x8,0xc,0xc,
0x4,0x4,0x8,0xc,0xc,0x10,0x10,0x14,0x18,0x18,0x4,0x4,0x8,0xc,0xc,0x10,0x10,0x14,0x18,0x18,
0x4,0x4,0x8,0xc,0xc,0x10,0x10,0x14,0x18,0x18,0x4,0x4,0x8,0xc,0xc,0x10,0x10,0x14,0x18,0x18,
0x10,0x10,0x14,0x18,0x18,0x4,0x4,0x8,0xc,0xc,0x10,0x10,0x14,0x18,0x18,0x4,0x4,0x8,0xc,0xc,
0x10,0x10,0x14,0x18,0x18,0x4,0x4,0x8,0xc,0xc,0x10,0x10,0x14,0x18,0x18,0x4,0x4,0x8,0xc,0xc,
0x4,0x4,0x8,0xc,0xc,0x10,0x10,0x14,0x18,0x18,0x4,0x4,0x8,0xc,0xc,0x10,0x10,0x14,0x18,0x18,
0x4,0x4,0x8,0xc,0xc,0x10,0x10,0x14,0x18,0x18,0x4,0x4,0x8,0xc,0xc,0x10,0x10,0x14,0x18,0x18
};

const uint16_t *convergencewrgb_map_Layers[] = {convergencewrgb_map_Tiles_Tile_Layer_1};

const int convergencewrgb_map_Parallax[][2] = {{65536,65536}};

const dtilemap_t convergencewrgb_map_Map = {16,16,20,14,1,0,0,(int *)convergencewrgb_map_Parallax,(uint16_t **)convergencewrgb_map_Layers};

#endif
