#ifndef CHECK_H
#define CHECK_H

#include <stdint.h>

#define CHECK_ROW_BW 0x02,0x03,0x02,0x03,0x02,0x03,0x02,0x03,0x02,0x03,0x02,0x03,0x02,0x03,0x02,0x03
#define CHECK_ROW_WB 0x03,0x02,0x03,0x02,0x03,0x02,0x03,0x02,0x03,0x02,0x03,0x02,0x03,0x02,0x03,0x02

uint8_t check_Res0[] __attribute__((aligned(16))) = {
CHECK_ROW_BW, CHECK_ROW_WB, CHECK_ROW_BW, CHECK_ROW_WB,
CHECK_ROW_BW, CHECK_ROW_WB, CHECK_ROW_BW, CHECK_ROW_WB,
CHECK_ROW_BW, CHECK_ROW_WB, CHECK_ROW_BW, CHECK_ROW_WB,
CHECK_ROW_BW, CHECK_ROW_WB, CHECK_ROW_BW, CHECK_ROW_WB
};

#undef CHECK_ROW_BW
#undef CHECK_ROW_WB


const uint8_t * check_Reslist[] = {
yatssd_empty_tile,
check_Res0
};

#endif
