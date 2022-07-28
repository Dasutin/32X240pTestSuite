#include <stdint.h>

const uint8_t buzz_sprite[] __attribute__((aligned(16))) =
{
    0x00,0x00,0xF9,0x00,0xF9,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xF9,0x00,0xF9,0x00,
    0x00,0xF9,0x00,0xF9,0x00,0xF9,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xF9,0x00,0xF9,0x00,0xF9,0x00,0xF9,
    0xF9,0x00,0xF9,0x00,0xF9,0x00,0xF9,0x00,0xF9,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xF9,0x00,0xF9,0x00,0xF9,0x00,0xF9,0x00,
    0x00,0xF9,0x00,0xF9,0x00,0xF9,0x00,0xF9,0x00,0xF9,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xF9,0x00,0xF9,0x00,0xF9,0x00,0xF9,0x00,0xF9,0x00,0x00,
    0x00,0x00,0xF9,0x00,0xF9,0x00,0xF9,0xFa,0xF2,0x00,0xF9,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xF9,0x00,0xF9,0x00,0xF9,0x00,0xF9,0x00,0xF9,0x00,0x00,0x00,
    0x00,0x00,0xF2,0xF1,0x00,0xF9,0xFa,0xFa,0xF0,0xF2,0x00,0xF9,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xF9,0x00,0xF9,0x00,0xF9,0x00,0xF9,0x00,0xF9,0x00,0x00,0x00,0x00,
    0x00,0xF2,0xF2,0xF0,0xF1,0xFa,0xFa,0xF0,0xF0,0xF2,0xF9,0x00,0xF9,0x00,0x00,0x00,0xF4,0xF4,0xF4,0xF4,0xF4,0xF4,0xF9,0x00,0xF9,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0xF2,0xF2,0xF0,0x00,0xFa,0xFa,0xF0,0xF5,0xF0,0xF2,0xF3,0xF3,0xF3,0xF9,0x00,0xF4,0xF0,0xF4,0xF4,0xF0,0xF0,0xF0,0xF4,0xF4,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0xF2,0xF0,0x00,0x00,0x00,0xF0,0xF2,0xF6,0xF1,0xF2,0xF5,0xF3,0xF3,0xF3,0xF0,0xF0,0xF4,0xFb,0xF9,0xF4,0xF0,0xF0,0xF0,0xF0,0xF4,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0xF3,0xF5,0xF0,0xF8,0xF8,0xF6,0xF5,0xF5,0xF3,0xF3,0xF3,0xF0,0xF4,0xF9,0xF9,0xF4,0xF0,0xF0,0xF0,0xF0,0xF0,0xF4,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0xF9,0xF5,0xF6,0xF8,0xF8,0xFb,0xFb,0xFb,0xF3,0xF3,0xF3,0xF3,0xF0,0xF4,0xF4,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF4,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0xF7,0xF9,0xFb,0xF5,0xF6,0xFb,0xFb,0xFb,0xFb,0xF9,0xF3,0xF3,0xF3,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF4,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0xF7,0xF0,0xFb,0xF5,0xF5,0xFb,0xF0,0xF0,0xF9,0xF9,0xF3,0xF3,0xF3,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF4,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0xF7,0xF0,0xF9,0xF3,0xF5,0xFb,0xF0,0xF0,0xF9,0xF7,0xF3,0xF3,0xF3,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF4,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0xF3,0xF9,0xF3,0xF3,0xF3,0xF3,0xF9,0xF9,0xF9,0xF3,0xF3,0xF3,0xF0,0xF0,0xF0,0xF0,0xF4,0xF7,0xF0,0xF0,0xF4,0xF7,0xF0,0xF4,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0xF3,0xF3,0xF3,0xF3,0xF3,0xF3,0xF3,0xF3,0xF3,0xF3,0xF3,0xF3,0xF0,0xF0,0xF0,0xF0,0xF4,0xF7,0xF0,0xF0,0xF4,0xF7,0xF0,0xF4,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0xF0,0xF4,0xFb,0xF0,0xFb,0xF9,0xF9,0xF0,0xF4,0xF3,0xF3,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF4,0xF7,0xF0,0xF0,0xF4,0xF7,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0xF0,0xF9,0xFb,0xF3,0xFb,0xFb,0xF9,0xF9,0xF4,0xF3,0xF0,0xF4,0xF0,0xF0,0xF0,0xF0,0xF4,0xFb,0xFb,0xF9,0xF9,0xF9,0xF9,0xF9,0xF9,0xF9,0xF4,0x00,0x00,
    0x00,0x00,0x00,0x00,0xF0,0xF9,0xF3,0xFb,0xF9,0xF4,0xF0,0xF0,0xF0,0xF0,0xF4,0xF4,0xF4,0xF0,0xF4,0xF7,0xF7,0xFb,0xFb,0xF7,0xF7,0xF7,0xF7,0xF7,0xF7,0xF7,0xF4,0xF0,
    0x00,0x00,0x00,0x00,0xF0,0xF7,0xF0,0xF9,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF4,0xF4,0xF7,0xFb,0xFb,0xF7,0xF7,0xF7,0xF7,0xF7,0xF7,0xF7,0xF7,0xF0,
    0x00,0x00,0x00,0x00,0x00,0xF0,0x00,0xF0,0xF0,0xF3,0xF8,0xF3,0xF0,0xF0,0xF0,0xF0,0xF3,0xF0,0xF5,0xF4,0xF4,0xF7,0xF4,0xF4,0xF4,0xF4,0xF4,0xF4,0xF4,0xF4,0xF4,0xF0,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xF0,0xF0,0xF0,0xF0,0xF0,0xF4,0xF4,0xF5,0xF5,0xF0,0xF6,0xF8,0xF6,0xF3,0xF3,0xF3,0xF3,0xF3,0xF3,0xF4,0xF4,0xF0,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xF0,0xF4,0xF8,0xFb,0xF8,0xF4,0xF0,0xF3,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xF4,0xF8,0xFb,0xF8,0xF8,0xF8,0xF3,0xF0,0xF3,0xF3,0xF0,0xF0,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xF4,0xF4,0xFb,0xF4,0xF8,0xF5,0xF3,0xF3,0xF0,0xF3,0xF3,0xF0,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xF0,0xF8,0xFb,0xF8,0xF8,0xF0,0xF5,0xF3,0xF3,0xF0,0xF3,0xF3,0xF0,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xF0,0xF8,0xFb,0xF8,0xF8,0xF5,0xF0,0xF3,0xF3,0xF3,0xF0,0xF3,0xF0,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xF0,0xF5,0xF8,0xF5,0xF5,0xF5,0xF0,0xF3,0xF3,0xF3,0xF0,0xF3,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xF0,0xF4,0xF4,0xF4,0xF5,0xF3,0xF3,0xF0,0xF3,0xF3,0xF0,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xF9,0xFb,0xF7,0xF0,0xF3,0xF3,0xF3,0xF0,0xF3,0xF0,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xF4,0xFb,0xF4,0xF4,0xF0,0xF3,0xF3,0xF0,0xF0,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xF0,0xF0,0xF4,0xF0,0xF0,0xF0,0xF0,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};

const uint8_t buzz_shadow_sprite[] __attribute__((aligned(16))) =
{
    0x00,0x00,0xF0,0x00,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xF0,0x00,0xF0,0x00,
    0x00,0xF0,0x00,0xF0,0x00,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xF0,0x00,0xF0,0x00,0xF0,0x00,0xF0,
    0xF0,0x00,0xF0,0x00,0xF0,0x00,0xF0,0x00,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xF0,0x00,0xF0,0x00,0xF0,0x00,0xF0,0x00,
    0x00,0xF0,0x00,0xF0,0x00,0xF0,0x00,0xF0,0x00,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xF0,0x00,0xF0,0x00,0xF0,0x00,0xF0,0x00,0xF0,0x00,0x00,
    0x00,0x00,0xF0,0x00,0xF0,0x00,0xF0,0xF0,0xF0,0x00,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xF0,0x00,0xF0,0x00,0xF0,0x00,0xF0,0x00,0xF0,0x00,0x00,0x00,
    0x00,0x00,0xF0,0xF0,0x00,0xF0,0xF0,0xF0,0xF0,0xF0,0x00,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xF0,0x00,0xF0,0x00,0xF0,0x00,0xF0,0x00,0xF0,0x00,0x00,0x00,0x00,
    0x00,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0x00,0xF0,0x00,0x00,0x00,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0x00,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0xF0,0xF0,0xF0,0x00,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0x00,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0xF0,0xF0,0x00,0x00,0x00,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0x00,0x00,
    0x00,0x00,0x00,0x00,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,
    0x00,0x00,0x00,0x00,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,
    0x00,0x00,0x00,0x00,0x00,0xF0,0x00,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};