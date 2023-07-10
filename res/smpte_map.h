#ifndef SMPTE_MAP_H
#define SMPTE_MAP_H

#include <stdint.h>

const uint16_t smpte_map_Tiles_Tile_Layer_1[] = {
0x4,0x4,0x28,0x2c,0x2c,0xc,0x10,0x10,0x14,0x38,0x38,0x70,0x50,0x50,0x54,0x58,0x58,0x7c,0x5c,0x5c,
0x4,0x4,0x28,0x2c,0x2c,0xc,0x10,0x10,0x14,0x38,0x38,0x70,0x50,0x50,0x54,0x58,0x58,0x7c,0x5c,0x5c,
0x4,0x4,0x28,0x2c,0x2c,0xc,0x10,0x10,0x14,0x38,0x38,0x70,0x50,0x50,0x54,0x58,0x58,0x7c,0x5c,0x5c,
0x4,0x4,0x28,0x2c,0x2c,0xc,0x10,0x10,0x14,0x38,0x38,0x70,0x50,0x50,0x54,0x58,0x58,0x7c,0x5c,0x5c,
0x4,0x4,0x28,0x2c,0x2c,0xc,0x10,0x10,0x14,0x38,0x38,0x70,0x50,0x50,0x54,0x58,0x58,0x7c,0x5c,0x5c,
0x4,0x4,0x28,0x2c,0x2c,0xc,0x10,0x10,0x14,0x38,0x38,0x70,0x50,0x50,0x54,0x58,0x58,0x7c,0x5c,0x5c,
0x4,0x4,0x28,0x2c,0x2c,0xc,0x10,0x10,0x14,0x38,0x38,0x70,0x50,0x50,0x54,0x58,0x58,0x7c,0x5c,0x5c,
0x4,0x4,0x28,0x2c,0x2c,0xc,0x10,0x10,0x14,0x38,0x38,0x70,0x50,0x50,0x54,0x58,0x58,0x7c,0x5c,0x5c,
0x4,0x4,0x28,0x2c,0x2c,0xc,0x10,0x10,0x14,0x38,0x38,0x70,0x50,0x50,0x54,0x58,0x58,0x7c,0x5c,0x5c,
0x94,0x94,0x98,0x9c,0x9c,0xa0,0xa4,0xa4,0xb8,0xbc,0xbc,0xc0,0xc4,0xc4,0x18,0x1c,0x1c,0x20,0x24,0x24,
0x3c,0x3c,0x40,0x44,0x48,0x64,0x68,0x6c,0x84,0x88,0x8c,0x90,0xa8,0xa8,0xac,0xb0,0xb4,0xcc,0xd0,0xd0,
0xc8,0xc8,0xc8,0xd4,0xd8,0xd8,0xd8,0xdc,0xe0,0xe0,0xe4,0xe8,0xe8,0xe8,0xec,0xf0,0xf4,0x60,0xf8,0xf8,
0xc8,0xc8,0xc8,0xd4,0xd8,0xd8,0xd8,0xdc,0xe0,0xe0,0xe4,0xe8,0xe8,0xe8,0xec,0xf0,0xf4,0x60,0xf8,0xf8,
0xc8,0xc8,0xc8,0xd4,0xd8,0xd8,0xd8,0xdc,0xe0,0xe0,0xe4,0xe8,0xe8,0xe8,0xec,0xf0,0xf4,0x60,0xf8,0xf8
};

const uint16_t *smpte_map_Layers[] = {smpte_map_Tiles_Tile_Layer_1};

const int smpte_map_Parallax[][2] = {{65536,65536}};

const dtilemap_t smpte_map_Map = {16,16,20,14,1,0,0,(int *)smpte_map_Parallax,(uint16_t **)smpte_map_Layers};

#endif
