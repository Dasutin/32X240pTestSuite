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

#include <stdint.h>

extern uint16_t InitCD(void);
extern uint16_t cd_ok;
extern uint16_t gen_lvl2;

#define COMM0       (*(volatile uint16_t *)0xA15120)
#define COMM2       (*(volatile uint16_t *)0xA15122)

#define SCD_RESET   (*(volatile uint8_t  *)0xA12001)
#define SCD_MM      (*(volatile uint16_t *)0xA12002)
#define SCD_HINT    (*(volatile uint16_t *)0xA12006)
#define SCD_MAINCMD (*(volatile uint8_t  *)0xA1200E)
#define SCD_SUBSTAT (*(volatile uint8_t  *)0xA1200F)
#define SCD_PARAM1  (*(volatile uint16_t *)0xA12010)
#define SCD_PARAM2  (*(volatile uint16_t *)0xA12012)
#define SCD_RESULT0 (*(volatile uint16_t *)0xA12020)
#define SCD_RESULT1 (*(volatile uint16_t *)0xA12022)
#define SCD_RESULT2 (*(volatile uint16_t *)0xA12024)

#define SCD_BIOS_BASE    0x400000UL
#define SCD_PROGRAM_RAM  0x420000UL
#define SCD_WORD_RAM     0x600000UL
#define SCD_BIOS_SIZE    0x20000UL
#define SCD_PROGRAM_SIZE 0x20000UL
#define SCD_WORD_SIZE    0x40000UL

#define SCD_STATUS_PRESENT     0x0001
#define SCD_STATUS_INITIALIZED 0x0002
#define SCD_STATUS_BUSY        0x0004

enum {
    SCD_OP_STATUS = 0,
    SCD_OP_INIT,
    SCD_OP_RESULT0_LO,
    SCD_OP_RESULT0_HI,
    SCD_OP_RESULT1_LO,
    SCD_OP_RESULT1_HI,
    SCD_OP_RESULT2,
    SCD_OP_BIOS_WORD,
    SCD_OP_HINT_TEST,
    SCD_OP_FLAG_TEST,
    SCD_OP_COMM_TEST,
    SCD_OP_PROGRAM_RAM_TEST,
    SCD_OP_WORD_RAM_TEST,
    SCD_OP_MEM_ADDR_HI,
    SCD_OP_MEM_ADDR_LO,
    SCD_OP_MEM_READ_WORD,
    SCD_OP_PCM_RAM_TEST,
    SCD_OP_PCM_CONTROL,
    SCD_OP_DISC_INFO,
    SCD_OP_CDDA_PLAY,
    SCD_OP_CDDA_STOP,
    SCD_OP_DATA_SET_LBA,
    SCD_OP_DATA_START,
    SCD_OP_DATA_POLL,
    SCD_OP_WORD_RAM_READ,
    SCD_OP_WORD_RAM_STRESS,
    SCD_OP_SUBCPU_PING,
    SCD_OP_YM_CONTROL,
    SCD_OP_RESET,
    SCD_OP_MEM_BANK,
    SCD_OP_PCM_RAM_SET,
    SCD_OP_PCM_RAM_COMPARE,
    SCD_OP_TRACK_INFO,
    SCD_OP_TRAY_OPEN,
    SCD_OP_TRAY_CLOSE
};

static uint32_t result0;
static uint32_t result1;
static uint16_t result2;
static uint32_t memory_address = SCD_BIOS_BASE;
static uint32_t data_lba = 16;
static uint16_t async_sectors;

static int bytes_equal(const volatile uint8_t *p, const char *s, int count)
{
    int i;
    for (i = 0; i < count; i++)
        if (p[i] != (uint8_t)s[i])
            return 0;
    return 1;
}

static int sega_cd_present(void)
{
    const volatile uint8_t *bios;

    bios = (const volatile uint8_t *)0x415800;
    if (bytes_equal(bios + 0x6D, "SEGA", 4))
        return 1;
    bios = (const volatile uint8_t *)0x416000;
    if (bytes_equal(bios + 0x6D, "SEGA", 4))
        return 1;
    if (bytes_equal(bios + 0x6D, "WONDER", 6))
        return 1;
    bios = (const volatile uint8_t *)0x41AD00;
    return bytes_equal(bios + 0x6D, "SEGA", 4);
}

static uint32_t crc32_region(const volatile uint8_t *data, uint32_t size,
    int patch_bios)
{
    uint32_t crc = 0xFFFFFFFFUL;
    uint32_t i;
    int bit;

    for (i = 0; i < size; i++)
    {
        uint8_t value = data[i];

        if (patch_bios && i == 0x70 && value == 0x00)
            value = 0xFF;
        if (patch_bios && i == 0x72)
            value = 0xFD;
        if (patch_bios && i == 0x73)
            value = 0x0C;

        crc ^= value;
        for (bit = 0; bit < 8; bit++)
            crc = (crc >> 1) ^ ((crc & 1) ? 0xEDB88320UL : 0);
    }
    return crc ^ 0xFFFFFFFFUL;
}

static int wait_byte(volatile uint8_t *address, uint8_t value)
{
    uint32_t timeout = 2000000UL;
    while (*address != value && timeout)
        timeout--;
    return timeout != 0;
}

static int sub_command(uint8_t command, uint16_t param1, uint16_t param2)
{
    uint8_t status;

    if (!cd_ok && !InitCD())
        return 0;
    cd_ok = 1;

    if (!wait_byte(&SCD_SUBSTAT, 0))
        return 0;
    SCD_PARAM1 = param1;
    SCD_PARAM2 = param2;
    SCD_MAINCMD = command;

    {
        uint32_t timeout = 8000000UL;
        while ((status = SCD_SUBSTAT) == 0 && timeout)
            timeout--;
        if (!timeout)
        {
            SCD_MAINCMD = 0;
            return 0;
        }
    }

    result0 = ((uint32_t)SCD_RESULT0 << 16) | SCD_RESULT1;
    result2 = SCD_RESULT2;
    SCD_MAINCMD = 0;
    wait_byte(&SCD_SUBSTAT, 0);
    return status == 'D';
}

static int sub_command_start(uint8_t command, uint16_t param1, uint16_t param2)
{
    if (!cd_ok && !InitCD())
        return 0;
    cd_ok = 1;
    if (!wait_byte(&SCD_SUBSTAT, 0))
        return 0;

    SCD_PARAM1 = param1;
    SCD_PARAM2 = param2;
    SCD_MAINCMD = command;
    return 1;
}

static uint16_t sub_command_poll(void)
{
    uint8_t status;

    if (!SCD_MAINCMD)
        return 0x8000;
    status = SCD_SUBSTAT;
    if (!status)
        return 0;

    result0 = ((uint32_t)SCD_RESULT0 << 16) | SCD_RESULT1;
    result2 = SCD_RESULT2;
    SCD_MAINCMD = 0;
    wait_byte(&SCD_SUBSTAT, 0);
    return status == 'D' ? 1 : (uint16_t)(0x8000 | result2);
}

static int reset_and_hold_subcpu(void)
{
    uint32_t timeout;

    if (!sega_cd_present())
        return 0;

    gen_lvl2 = 0;
    cd_ok = 0;
    SCD_MM = 0xFF00;
    SCD_RESET = 0x03;
    SCD_RESET = 0x02;
    SCD_RESET = 0x00;
    SCD_RESET = 0x02;

    timeout = 2000000UL;
    while (!(SCD_RESET & 2) && timeout)
    {
        SCD_RESET = 0x02;
        timeout--;
    }
    return timeout != 0;
}

static uint16_t test_register_byte(volatile uint8_t *reg)
{
    static const uint8_t patterns[] = { 0x55, 0xAA, 0xFF, 0x00 };
    int i;

    for (i = 0; i < 4; i++)
    {
        *reg = patterns[i];
        if (*reg != patterns[i])
        {
            result0 = (uint32_t)reg;
            result1 = ((uint32_t)patterns[i] << 16) | *reg;
            return 0;
        }
    }
    return 1;
}

static uint16_t test_register_word(volatile uint16_t *reg)
{
    static const uint16_t patterns[] = { 0x5555, 0xAAAA, 0xFFFF, 0x0000 };
    int i;

    for (i = 0; i < 4; i++)
    {
        *reg = patterns[i];
        if (*reg != patterns[i])
        {
            result0 = (uint32_t)reg;
            result1 = ((uint32_t)patterns[i] << 16) | *reg;
            return 0;
        }
    }
    return 1;
}

static uint16_t hint_test(void)
{
    const volatile uint8_t *bios = (const volatile uint8_t *)SCD_BIOS_BASE;

    result0 = 0;
    result1 = 0;
    if (!reset_and_hold_subcpu())
        return 0;
    SCD_HINT = 0xFD0C;
    result0 = SCD_HINT;
    result1 = ((uint32_t)bios[0x72] << 8) | bios[0x73];
    return SCD_HINT == 0xFD0C && bios[0x72] == 0xFD && bios[0x73] == 0x0C;
}

static uint16_t comm_test(void)
{
    int i;

    result0 = 0xA12010UL;
    result1 = 0x55550000UL;
    if (!reset_and_hold_subcpu())
        return 0;
    for (i = 0; i < 8; i++)
        if (!test_register_word((volatile uint16_t *)(0xA12010UL + i * 2)))
            return 0;
    return 1;
}

static int test_ram_pattern(volatile uint16_t *ram, uint32_t bytes,
    uint16_t pattern, uint32_t base)
{
    uint32_t words = bytes >> 1;
    uint32_t i;

    for (i = 0; i < words; i++)
    {
        ram[i] = pattern;
        if (ram[i] != pattern)
        {
            result0 = base + i * 2;
            result1 = ((uint32_t)pattern << 16) | ram[i];
            return 0;
        }
    }
    for (i = 0; i < words; i++)
    {
        if (ram[i] != pattern)
        {
            result0 = base + i * 2;
            result1 = ((uint32_t)pattern << 16) | ram[i];
            return 0;
        }
    }
    return 1;
}

static uint16_t program_ram_test(void)
{
    static const uint16_t patterns[] = { 0xAAAA, 0x5555, 0xFFFF, 0x0000 };
    int bank, pattern;

    result0 = 0xA12002UL;
    result1 = 0;
    if (!reset_and_hold_subcpu())
        return 0;
    for (bank = 0; bank < 4; bank++)
    {
        SCD_MM = (uint16_t)(bank << 6);
        if ((SCD_MM & 0x00C0) != (bank << 6))
        {
            result0 = 0xA12002UL;
            result1 = ((uint32_t)(bank << 6) << 16) | (SCD_MM & 0x00C0);
            return 0;
        }
        for (pattern = 0; pattern < 4; pattern++)
            if (!test_ram_pattern((volatile uint16_t *)SCD_PROGRAM_RAM,
                    SCD_PROGRAM_SIZE, patterns[pattern],
                    SCD_PROGRAM_RAM + bank * SCD_PROGRAM_SIZE))
                return 0;
    }
    return 1;
}

static uint16_t word_ram_test(void)
{
    static const uint16_t patterns[] = { 0xAAAA, 0x5555, 0xFFFF, 0x0000 };
    int pattern;

    result0 = SCD_WORD_RAM;
    result1 = 0xAAAA0000UL;
    if (!reset_and_hold_subcpu())
        return 0;
    SCD_MM = 0x0000;
    for (pattern = 0; pattern < 4; pattern++)
        if (!test_ram_pattern((volatile uint16_t *)SCD_WORD_RAM,
                SCD_WORD_SIZE, patterns[pattern], SCD_WORD_RAM))
            return 0;
    return 1;
}

static uint16_t pcm_ram_test(void)
{
    static const uint16_t patterns[] = { 0x55, 0xAA, 0xFF, 0x00 };
    int i;

    result0 = result1 = 0;
    result2 = 0;
    for (i = 0; i < 4; i++)
    {
        if (!sub_command('M', patterns[i], 0))
        {
            result1 = patterns[i];
            return 0;
        }
    }
    return 1;
}

static void ym_wait(void)
{
    uint32_t timeout = 100000UL;
    while ((*(volatile uint8_t *)0xA04000 & 0x80) && timeout)
        timeout--;
}

static void ym_write(uint8_t reg, uint8_t value)
{
    ym_wait();
    *(volatile uint8_t *)0xA04000 = reg;
    ym_wait();
    *(volatile uint8_t *)0xA04001 = value;
}

static uint16_t ym_control(uint16_t arg)
{
    uint8_t action = arg & 0xFF;
    uint8_t pan = (arg >> 8) & 3;
    uint8_t stereo = pan == 0 ? 0x80 : (pan == 2 ? 0x40 : 0xC0);
    static const uint8_t op_offsets[] = { 0, 4, 8, 12 };
    int i;

    *(volatile uint16_t *)0xA11100 = 0x0100;
    {
        uint32_t timeout = 100000UL;
        while ((*(volatile uint16_t *)0xA11100 & 0x0100) == 0 && timeout)
            timeout--;
    }

    if (!action)
    {
        ym_write(0x28, 0x00);
        *(volatile uint16_t *)0xA11100 = 0;
        return 1;
    }

    ym_write(0x28, 0x00);
    ym_write(0x22, 0x00);
    ym_write(0x27, 0x00);
    ym_write(0xB0, 0x07);
    ym_write(0xB4, stereo);
    for (i = 0; i < 4; i++)
    {
        uint8_t o = op_offsets[i];
        ym_write(0x30 + o, 0x01);
        ym_write(0x40 + o, 0x18);
        ym_write(0x50 + o, 0x1F);
        ym_write(0x60 + o, 0x00);
        ym_write(0x70 + o, 0x00);
        ym_write(0x80 + o, 0x0F);
    }
    ym_write(0xA4, 0x22);
    ym_write(0xA0, 0x69);
    ym_write(0x28, 0xF0);
    *(volatile uint16_t *)0xA11100 = 0;
    return 1;
}

void segacd_dispatch(void)
{
    uint16_t request = COMM0;
    uint16_t arg = COMM2;
    uint16_t op = request & 0x00FF;
    uint16_t response = 0;

    if ((request & 0xFF00) != 0x1600)
    {
        COMM2 = 0xFFFF;
        COMM0 = 0;
        return;
    }

    switch (op)
    {
        case SCD_OP_STATUS:
            response = sega_cd_present() ? SCD_STATUS_PRESENT : 0;
            if (cd_ok)
                response |= SCD_STATUS_INITIALIZED;
            if (SCD_MAINCMD)
                response |= SCD_STATUS_BUSY;
            break;
        case SCD_OP_INIT:
            response = sega_cd_present() && (cd_ok || InitCD());
            if (response)
                cd_ok = 1;
            break;
        case SCD_OP_RESULT0_LO: response = (uint16_t)result0; break;
        case SCD_OP_RESULT0_HI: response = (uint16_t)(result0 >> 16); break;
        case SCD_OP_RESULT1_LO: response = (uint16_t)result1; break;
        case SCD_OP_RESULT1_HI: response = (uint16_t)(result1 >> 16); break;
        case SCD_OP_RESULT2: response = result2; break;
        case SCD_OP_BIOS_WORD:
            if (sega_cd_present())
            {
                if (arg == 0xFFFF)
                {
                    result0 = crc32_region((const volatile uint8_t *)SCD_BIOS_BASE,
                        SCD_BIOS_SIZE, 1);
                    response = 1;
                }
                else
                    response = *(const volatile uint16_t *)(SCD_BIOS_BASE + arg);
            }
            break;
        case SCD_OP_HINT_TEST:
            response = hint_test();
            break;
        case SCD_OP_FLAG_TEST:
            result0 = 0xA1200EUL;
            result1 = 0x00550000UL;
            response = reset_and_hold_subcpu() ?
                test_register_byte((volatile uint8_t *)0xA1200E) : 0;
            break;
        case SCD_OP_COMM_TEST:
            response = comm_test();
            break;
        case SCD_OP_PROGRAM_RAM_TEST:
            response = program_ram_test();
            break;
        case SCD_OP_WORD_RAM_TEST:
            response = word_ram_test();
            break;
        case SCD_OP_MEM_ADDR_HI:
            memory_address = (memory_address & 0x0000FFFFUL) |
                ((uint32_t)arg << 16);
            response = 1;
            break;
        case SCD_OP_MEM_ADDR_LO:
            memory_address = (memory_address & 0xFFFF0000UL) | arg;
            response = 1;
            break;
        case SCD_OP_MEM_READ_WORD:
            response = *(const volatile uint16_t *)memory_address;
            memory_address += 2;
            break;
        case SCD_OP_PCM_RAM_TEST:
            response = pcm_ram_test();
            break;
        case SCD_OP_PCM_CONTROL:
            response = sub_command('A', arg, 0);
            break;
        case SCD_OP_DISC_INFO:
            response = sub_command('D', 0, 0);
            break;
        case SCD_OP_CDDA_PLAY:
            response = sub_command('P', arg, 0);
            break;
        case SCD_OP_CDDA_STOP:
            response = sub_command('S', 0, 0);
            break;
        case SCD_OP_DATA_SET_LBA:
            data_lba = (data_lba & 0xFFFF0000UL) | arg;
            response = 1;
            break;
        case SCD_OP_DATA_START:
            async_sectors = arg ? arg : 1;
            if (async_sectors > 120)
                async_sectors = 120;
            SCD_MM = 0x0002;
            response = sub_command_start('R', (uint16_t)data_lba, async_sectors);
            break;
        case SCD_OP_DATA_POLL:
            response = sub_command_poll();
            if (response == 1)
            {
                uint32_t sub_crc = result0;
                result1 = crc32_region((const volatile uint8_t *)SCD_WORD_RAM,
                    (uint32_t)async_sectors * 2048UL, 0);
                result0 = sub_crc;
                result2 = async_sectors;
                if (result0 != result1)
                    response = 2;
            }
            break;
        case SCD_OP_WORD_RAM_READ:
            response = ((const volatile uint16_t *)SCD_WORD_RAM)[arg];
            break;
        case SCD_OP_WORD_RAM_STRESS:
            SCD_MM = 0x0002;
            if (sub_command('W', arg, 0))
            {
                uint32_t i;
                const volatile uint16_t *wordram =
                    (const volatile uint16_t *)SCD_WORD_RAM;
                response = 1;
                for (i = 0; i < 4096; i++)
                    if (wordram[i] != (uint16_t)(arg ^ i))
                    {
                        result0 = SCD_WORD_RAM + i * 2;
                        result1 = ((uint32_t)(arg ^ i) << 16) | wordram[i];
                        response = 0;
                        break;
                    }
            }
            break;
        case SCD_OP_SUBCPU_PING:
            response = sub_command('Q', arg, 0);
            break;
        case SCD_OP_YM_CONTROL:
            response = ym_control(arg);
            break;
        case SCD_OP_RESET:
            response = reset_and_hold_subcpu();
            break;
        case SCD_OP_MEM_BANK:
            SCD_MM = (uint16_t)((arg & 3) << 6);
            response = (SCD_MM & 0x00C0) == ((arg & 3) << 6);
            break;
        case SCD_OP_PCM_RAM_SET:
            response = sub_command('F', arg & 0xFF, arg >> 8);
            break;
        case SCD_OP_PCM_RAM_COMPARE:
            response = sub_command('M', arg & 0xFF, arg >> 8);
            break;
        case SCD_OP_TRACK_INFO:
            response = sub_command('T', arg, 0);
            break;
        case SCD_OP_TRAY_OPEN:
            response = sub_command('O', 0, 0);
            break;
        case SCD_OP_TRAY_CLOSE:
            response = sub_command('C', 0, 0);
            break;
        default:
            response = 0xFFFF;
            break;
    }

    COMM2 = response;
    COMM0 = 0;
}
