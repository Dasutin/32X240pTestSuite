#ifndef EBU_MAP_H
#define EBU_MAP_H

#include <stdint.h>

const uint16_t ebu_map_Tiles_Tile_Layer_1[] = {
0x4,0x4,0x8,0xc,0xc,0x10,0x10,0x14,0x18,0x18,0x1c,0x1c,0x20,0x24,0x24,0x28,0x28,0x2c,0x30,0x30,
0x4,0x4,0x8,0xc,0xc,0x10,0x10,0x14,0x18,0x18,0x1c,0x1c,0x20,0x24,0x24,0x28,0x28,0x2c,0x30,0x30,
0x4,0x4,0x8,0xc,0xc,0x10,0x10,0x14,0x18,0x18,0x1c,0x1c,0x20,0x24,0x24,0x28,0x28,0x2c,0x30,0x30,
0x4,0x4,0x8,0xc,0xc,0x10,0x10,0x14,0x18,0x18,0x1c,0x1c,0x20,0x24,0x24,0x28,0x28,0x2c,0x30,0x30,
0x4,0x4,0x8,0xc,0xc,0x10,0x10,0x14,0x18,0x18,0x1c,0x1c,0x20,0x24,0x24,0x28,0x28,0x2c,0x30,0x30,
0x4,0x4,0x8,0xc,0xc,0x10,0x10,0x14,0x18,0x18,0x1c,0x1c,0x20,0x24,0x24,0x28,0x28,0x2c,0x30,0x30,
0x4,0x4,0x8,0xc,0xc,0x10,0x10,0x14,0x18,0x18,0x1c,0x1c,0x20,0x24,0x24,0x28,0x28,0x2c,0x30,0x30,
0x4,0x4,0x8,0xc,0xc,0x10,0x10,0x14,0x18,0x18,0x1c,0x1c,0x20,0x24,0x24,0x28,0x28,0x2c,0x30,0x30,
0x4,0x4,0x8,0xc,0xc,0x10,0x10,0x14,0x18,0x18,0x1c,0x1c,0x20,0x24,0x24,0x28,0x28,0x2c,0x30,0x30,
0x4,0x4,0x8,0xc,0xc,0x10,0x10,0x14,0x18,0x18,0x1c,0x1c,0x20,0x24,0x24,0x28,0x28,0x2c,0x30,0x30,
0x4,0x4,0x8,0xc,0xc,0x10,0x10,0x14,0x18,0x18,0x1c,0x1c,0x20,0x24,0x24,0x28,0x28,0x2c,0x30,0x30,
0x4,0x4,0x8,0xc,0xc,0x10,0x10,0x14,0x18,0x18,0x1c,0x1c,0x20,0x24,0x24,0x28,0x28,0x2c,0x30,0x30,
0x4,0x4,0x8,0xc,0xc,0x10,0x10,0x14,0x18,0x18,0x1c,0x1c,0x20,0x24,0x24,0x28,0x28,0x2c,0x30,0x30,
0x4,0x4,0x8,0xc,0xc,0x10,0x10,0x14,0x18,0x18,0x1c,0x1c,0x20,0x24,0x24,0x28,0x28,0x2c,0x30,0x30
};

const uint16_t *ebu_map_Layers[] = {ebu_map_Tiles_Tile_Layer_1};

const int ebu_map_Parallax[][2] = {{65536,65536}};

const dtilemap_t ebu_map_Map = {16,16,20,14,1,0,0,(int *)ebu_map_Parallax,(uint16_t **)ebu_map_Layers};

#endif
