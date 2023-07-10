#ifndef BG_MAP_H
#define BG_MAP_H

#include <stdint.h>

const uint16_t bg_map_Tiles_Tile_Layer_1[] = {
0x9c,0x4,0x8,0xc,0x10,0x14,0x18,0x1c,0x20,0x24,0x28,0x2c,0x30,0x34,0x38,0x3c,0x3c,0x3c,0xb4,0xd0,
0x9c,0x40,0x44,0x48,0x4c,0x50,0x54,0x58,0x5c,0x60,0x64,0x68,0x6c,0x70,0x74,0x78,0x78,0x78,0xe4,0xd0,
0x9c,0x7c,0x80,0x80,0x84,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x80,0x80,0xd4,0xd0,
0x9c,0xb8,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xd8,0xd0,
0x9c,0xb8,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xd8,0xd0,
0x9c,0xb8,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xd8,0xd0,
0x9c,0xb8,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xd8,0xd0,
0x9c,0xb8,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xd8,0xd0,
0x9c,0xb8,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xd8,0xd0,
0x9c,0xb8,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xd8,0xd0,
0x9c,0xb8,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xd8,0xd0,
0x9c,0x8c,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0xe0,0xd0,
0x9c,0x94,0x98,0x98,0xa8,0xa8,0xa8,0xa8,0xa8,0xa8,0xa8,0xa8,0xa8,0xa8,0xa8,0xac,0x98,0x98,0xb0,0xd0,
0x9c,0xbc,0xc0,0xc4,0xc8,0xcc,0xcc,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0xb4,0xd0
};

const uint16_t *bg_map_Layers[] = {bg_map_Tiles_Tile_Layer_1};

const int bg_map_Parallax[][2] = {{65536,65536}};

const dtilemap_t bg_map_Map = {16,16,20,14,1,0,0,(int *)bg_map_Parallax,(uint16_t **)bg_map_Layers};

#endif