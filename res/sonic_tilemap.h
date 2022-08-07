#ifndef SONIC_TILEMAP_H
#define SONIC_TILEMAP_H

#include <stdint.h>

const uint16_t sonic_tilemap_Tiles_Background[] = {
0x4,0x8,0xc,0x10,0x14,0x4,0x4,0x54,0x58,0x5c,0x4,0x80,0x84,0x88,0x8c,0x4,0x4,0x8,0xc,0x10,0x14,0x4,0x4,0x54,0x58,0x5c,0x4,0x80,0x84,0x88,0x8c,0x4,0x4,0x8,0xc,0x10,0x14,0x4,0x4,0x54,0x58,0x5c,0x4,0x80,0x84,0x88,0x8c,0x4,0x4,0x8,0xc,0x10,0x14,
0x4,0x30,0x34,0x38,0x3c,0x2c,0x4,0x4,0x4,0x4,0xa4,0xa8,0xac,0xb0,0xb4,0x4,0x4,0x30,0x34,0x38,0x3c,0x2c,0x4,0x4,0x4,0x4,0xa4,0xa8,0xac,0xb0,0xb4,0x4,0x4,0x30,0x34,0x38,0x3c,0x2c,0x4,0x4,0x4,0x4,0xa4,0xa8,0xac,0xb0,0xb4,0x4,0x4,0x30,0x34,0x38,0x3c,
0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x54,0x58,0x5c,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x54,0x58,0x5c,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x54,0x58,0x5c,0x4,0x4,0x4,0x4,0x4,
0x4,0x4,0x4,0x4,0x54,0x58,0x5c,0x4,0x4,0x60,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x54,0x58,0x5c,0x4,0x4,0x60,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x54,0x58,0x5c,0x4,0x4,0x60,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x54,
0x4,0x4,0x64,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x64,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x64,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x64,0x4,0x4,
0xd0,0xd0,0xcc,0x64,0x4,0x4,0xd4,0x64,0xd8,0x4,0x4,0xd0,0x64,0xd8,0x4,0x4,0xd0,0xd0,0xcc,0x64,0x4,0x4,0xd4,0x64,0xd8,0x4,0x4,0xd0,0x64,0xd8,0x4,0x4,0xd0,0xd0,0xcc,0x64,0x4,0x4,0xd4,0x64,0xd8,0x4,0x4,0xd0,0x64,0xd8,0x4,0x4,0xd0,0xd0,0xcc,0x64,0x4,
0xdc,0xf4,0xf8,0xfc,0x100,0x104,0x11c,0x120,0x124,0x100,0x128,0xdc,0x120,0x124,0x128,0x12c,0xdc,0xf4,0xf8,0xfc,0x100,0x104,0x11c,0x120,0x124,0x100,0x128,0xdc,0x120,0x124,0x128,0x12c,0xdc,0xf4,0xf8,0xfc,0x100,0x104,0x11c,0x120,0x124,0x100,0x128,0xdc,0x120,0x124,0x128,0x12c,0xdc,0xf4,0xf8,0xfc,0x100,
0x144,0x148,0x14c,0x150,0x154,0x18,0x1c,0x20,0x24,0x44,0x24,0x40,0x28,0x28,0x40,0x28,0x144,0x148,0x14c,0x150,0x154,0x18,0x1c,0x20,0x24,0x44,0x24,0x40,0x28,0x28,0x40,0x28,0x144,0x148,0x14c,0x150,0x154,0x18,0x1c,0x20,0x24,0x44,0x24,0x40,0x28,0x28,0x40,0x28,0x144,0x148,0x14c,0x150,0x154,
0x48,0x4c,0x50,0x68,0x6c,0x70,0x74,0x78,0x6c,0x90,0x94,0x98,0x9c,0xa0,0x94,0x98,0x48,0x4c,0x50,0x68,0x6c,0x70,0x74,0x78,0x6c,0x90,0x94,0x98,0x9c,0xa0,0x94,0x98,0x48,0x4c,0x50,0x68,0x6c,0x70,0x74,0x78,0x6c,0x90,0x94,0x98,0x9c,0xa0,0x94,0x98,0x48,0x4c,0x50,0x68,0x6c,
0xbc,0xc0,0xc4,0xc8,0xe0,0xe4,0xe8,0xec,0xc8,0xc0,0xf0,0xf0,0x108,0x10c,0x110,0x114,0xbc,0xc0,0xc4,0xc8,0xe0,0xe4,0xe8,0xec,0xc8,0xc0,0xf0,0xf0,0x108,0x10c,0x110,0x114,0xbc,0xc0,0xc4,0xc8,0xe0,0xe4,0xe8,0xec,0xc8,0xc0,0xf0,0xf0,0x108,0x10c,0x110,0x114,0xbc,0xc0,0xc4,0xc8,0xe0,
0x118,0x130,0x134,0x138,0x13c,0x138,0x13c,0x140,0x13c,0x138,0x118,0x134,0x134,0x158,0x138,0x13c,0x118,0x130,0x134,0x138,0x13c,0x138,0x13c,0x140,0x13c,0x138,0x118,0x134,0x134,0x158,0x138,0x13c,0x118,0x130,0x134,0x138,0x13c,0x138,0x13c,0x140,0x13c,0x138,0x118,0x134,0x134,0x158,0x138,0x13c,0x118,0x130,0x134,0x138,0x13c,
0x130,0x158,0x118,0x130,0x130,0x15c,0x158,0x138,0x118,0x134,0x134,0x130,0x160,0x160,0x134,0x134,0x130,0x158,0x118,0x130,0x130,0x15c,0x158,0x138,0x118,0x134,0x134,0x130,0x160,0x160,0x134,0x134,0x130,0x158,0x118,0x130,0x130,0x15c,0x158,0x138,0x118,0x134,0x134,0x130,0x160,0x160,0x134,0x134,0x130,0x158,0x118,0x130,0x130,
0x164,0x168,0x168,0x164,0x164,0x164,0x164,0x168,0x168,0x168,0x16c,0x164,0x164,0x164,0x164,0x16c,0x164,0x168,0x168,0x164,0x164,0x164,0x164,0x168,0x168,0x168,0x16c,0x164,0x164,0x164,0x164,0x16c,0x164,0x168,0x168,0x164,0x164,0x164,0x164,0x168,0x168,0x168,0x16c,0x164,0x164,0x164,0x164,0x16c,0x164,0x168,0x168,0x164,0x164,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0
};
const uint16_t sonic_tilemap_Tiles_Foreground[] = {
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x170,0x174,0x178,0x17c,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x170,0x174,0x178,0x17c,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x170,0x174,0x178,0x17c,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x180,0x184,0x188,0x18c,0x190,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x180,0x184,0x188,0x18c,0x190,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x180,0x184,0x188,0x18c,0x190,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x194,0x198,0x19c,0x1a0,0x1a4,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x194,0x198,0x19c,0x1a0,0x1a4,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x194,0x198,0x19c,0x1a0,0x1a4,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x1a8,0x1ac,0x1b0,0x1b4,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1a8,0x1ac,0x1b0,0x1b4,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1a8,0x1ac,0x1b0,0x1b4,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x1b8,0x1bc,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1b8,0x1bc,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1b8,0x1bc,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x1c8,0x1c8,0x1c8,0x1c8,0x1c8,0x1c8,0x1c0,0x1c4,0x1c8,0x1c8,0x1c8,0x1c8,0x1c8,0x1c8,0x1c8,0x1c8,0x1c8,0x1c8,0x1c8,0x1c8,0x1c8,0x1c8,0x1c0,0x1c4,0x1c8,0x1c8,0x1c8,0x1c8,0x1c8,0x1c8,0x1c8,0x1c8,0x1c8,0x1c8,0x1c8,0x1c8,0x1c8,0x1c8,0x1c0,0x1c4,0x1c8,0x1c8,0x1c8,0x1c8,0x1c8,0x1c8,0x1c8,0x1c8,0x1c8,0x1c8,0x1c8,0x1c8,0x1c8,
0x1d8,0x1d8,0x1d8,0x1d8,0x1d8,0x1d8,0x1e0,0x1e0,0x1e4,0x1d8,0x1d8,0x1d8,0x1d8,0x1d8,0x1d8,0x1d8,0x1d8,0x1d8,0x1d8,0x1d8,0x1d8,0x1d8,0x1e0,0x1e0,0x1e4,0x1d8,0x1d8,0x1d8,0x1d8,0x1d8,0x1d8,0x1d8,0x1d8,0x1d8,0x1d8,0x1d8,0x1d8,0x1d8,0x1e0,0x1e0,0x1e4,0x1d8,0x1d8,0x1d8,0x1d8,0x1d8,0x1d8,0x1d8,0x1d8,0x1d8,0x1d8,0x1d8,0x1d8,
0x1e8,0x1e8,0x1e8,0x1e8,0x1e8,0x1e8,0x1e8,0x1e8,0x1e8,0x1e8,0x1e8,0x1e8,0x1e8,0x1e8,0x1e8,0x1e8,0x1e8,0x1e8,0x1e8,0x1e8,0x1e8,0x1e8,0x1e8,0x1e8,0x1e8,0x1e8,0x1e8,0x1e8,0x1e8,0x1e8,0x1e8,0x1e8,0x1e8,0x1e8,0x1e8,0x1e8,0x1e8,0x1e8,0x1e8,0x1e8,0x1e8,0x1e8,0x1e8,0x1e8,0x1e8,0x1e8,0x1e8,0x1e8,0x1e8,0x1e8,0x1e8,0x1e8,0x1e8
};

const uint16_t *sonic_tilemap_Layers[] = {sonic_tilemap_Tiles_Background,sonic_tilemap_Tiles_Foreground};

const int sonic_tilemap_Parallax[][2] = {{32768,65536},{65536,65536}};

const dtilemap_t sonic_tilemap_Map = {16,16,53,14,2,512,0,(int *)sonic_tilemap_Parallax,(uint16_t **)sonic_tilemap_Layers};

#endif