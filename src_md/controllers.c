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

#define RAM_CODE __attribute__((section(".ramcode")))

#define PORT_TYPE_TEAMPLAYER 0x0007
#define PORT_TYPE_PAD        0x000D
#define PORT_TYPE_UNKNOWN    0x000F
#define PORT_TYPE_EA4WAYPLAY 0x0010

#define JOY_TYPE_PAD3        0x0000
#define JOY_TYPE_PAD6        0x0001
#define JOY_TYPE_UNKNOWN     0x000F

#define JOY_SUPPORT_OFF        0x0000
#define JOY_SUPPORT_6BTN       0x0002
#define JOY_SUPPORT_TEAMPLAYER 0x0005
#define JOY_SUPPORT_EA4WAYPLAY 0x0006

#define BUTTON_ALL           0x0FFF

volatile uint16_t controller_mode;
volatile uint16_t controller_force_reset;
volatile uint16_t controller_debug_request;

static volatile uint16_t joy_state[8];
static volatile uint16_t joy_type[8];
static volatile uint16_t port_type[2];
static volatile uint16_t port_support[2];
static uint16_t detected_port_type[2];
static uint16_t detected_ea_four_way;

static RAM_CODE void io_delay(void)
{
    __asm__ volatile ("nop\n\tnop");
}

static RAM_CODE volatile uint8_t *port_data(unsigned int port)
{
    return (volatile uint8_t *)(0xA10003 + (port << 1));
}

static RAM_CODE volatile uint8_t *port_control(unsigned int port)
{
    return (volatile uint8_t *)(0xA10009 + (port << 1));
}

static RAM_CODE uint16_t th_control_phase(volatile uint8_t *data)
{
    uint16_t value;

    *data = 0x00;
    io_delay();
    value = *data;
    *data = 0x40;
    value <<= 8;
    value |= *data;

    return value;
}

static RAM_CODE uint16_t read_pad(unsigned int port, volatile uint16_t *type)
{
    volatile uint8_t *data = port_data(port);
    uint16_t value;
    uint16_t first = th_control_phase(data);
    uint16_t extended;

    (void)th_control_phase(data);
    extended = th_control_phase(data);
    value = th_control_phase(data);

    if ((value & 0x0F00) != 0x0F00)
    {
        *type = JOY_TYPE_PAD3;
        extended = 0x0F00;
    }
    else
    {
        *type = JOY_TYPE_PAD6;
        extended = (extended & 0x000F) << 8;
    }

    value = ((first & 0x3000) >> 6) | (first & 0x003F);
    value |= extended;
    return (value ^ 0x0FFF) & BUTTON_ALL;
}

static RAM_CODE uint8_t handshake(volatile uint8_t *data,
    uint8_t output, uint16_t *retry)
{
    uint8_t value = 0;
    uint8_t expected = (output >> 1) & 0x10;

    *data = output;
    while (*retry)
    {
        value = *data;
        if ((value & 0x10) == expected)
            break;
        (*retry)--;
    }

    return value & 0x0F;
}

static RAM_CODE int start_three_line(unsigned int port, uint8_t *header,
    uint8_t *phase, uint16_t *retry)
{
    volatile uint8_t *control = port_control(port);
    volatile uint8_t *data = port_data(port);
    unsigned int i;
    uint8_t initial;

    *retry = 255;
    *phase = 0x20;
    *control = 0x60;
    *data = 0x60;
    io_delay();

    initial = *data & 0x0F;
    if (initial != 0 && initial != 3)
        return 0;

    header[0] = handshake(data, 0x60, retry);
    for (i = 1; i < 8 && *retry; i++)
    {
        header[i] = handshake(data, *phase, retry);
        *phase ^= 0x20;
    }

    if (!*retry)
    {
        *data = 0x60;
        return 0;
    }

    return 1;
}

static RAM_CODE unsigned int team_player_slot(unsigned int port,
    unsigned int subport)
{
    if (port == 0)
    {
        if (subport == 0) return 0;
        return subport + 1;
    }

    if (subport == 0) return 1;
    return subport + 4;
}

static RAM_CODE void consume_mouse_packet(volatile uint8_t *data,
    uint8_t *phase, uint16_t *retry)
{
    unsigned int i;

    for (i = 0; i < 6 && *retry; i++)
    {
        (void)handshake(data, *phase, retry);
        *phase ^= 0x20;
    }
}

static RAM_CODE void read_team_player(unsigned int port)
{
    volatile uint8_t *data = port_data(port);
    uint8_t header[8];
    uint8_t phase;
    uint16_t retry;
    unsigned int subport;

    if (!start_three_line(port, header, &phase, &retry) ||
        header[0] != 0x03 || header[1] != 0x0F ||
        header[2] != 0x00 || header[3] != 0x00)
    {
        *data = 0x60;
        return;
    }

    for (subport = 0; subport < 4 && retry; subport++)
    {
        unsigned int slot = team_player_slot(port, subport);
        uint16_t value = 0;

        if (header[4 + subport] == JOY_TYPE_PAD3)
        {
            uint16_t dpad = handshake(data, phase, &retry);
            phase ^= 0x20;
            if (retry)
            {
                uint16_t buttons = handshake(data, phase, &retry);
                phase ^= 0x20;
                value = ((buttons << 4) | dpad) ^ 0x00FF;
                joy_type[slot] = JOY_TYPE_PAD3;
            }
        }
        else if (header[4 + subport] == JOY_TYPE_PAD6)
        {
            uint16_t dpad = handshake(data, phase, &retry);
            uint16_t buttons = 0;
            uint16_t extra = 0;
            phase ^= 0x20;
            if (retry)
            {
                buttons = handshake(data, phase, &retry);
                phase ^= 0x20;
            }
            if (retry)
            {
                extra = handshake(data, phase, &retry);
                phase ^= 0x20;
                value = ((extra << 8) | (buttons << 4) | dpad) ^ 0x0FFF;
                joy_type[slot] = JOY_TYPE_PAD6;
            }
        }
        else if (header[4 + subport] == 0x02)
        {
            consume_mouse_packet(data, &phase, &retry);
        }

        if (retry)
            joy_state[slot] = value & BUTTON_ALL;
    }

    *data = 0x60;
}

static RAM_CODE int detect_ea_four_way(void)
{
    volatile uint8_t *data1 = port_data(0);
    volatile uint8_t *data2 = port_data(1);
    uint8_t first;
    uint8_t second;

    *port_control(0) = 0x40;
    *port_control(1) = 0x7F;
    *data1 = 0x40;
    *data2 = 0x0C;
    io_delay();
    first = *data1 & 3;
    *data2 = 0x7C;
    io_delay();
    second = *data1 & 3;

    return first != 0 && second == 0;
}

static RAM_CODE void read_ea_four_way(void)
{
    volatile uint8_t *data1 = port_data(0);
    volatile uint8_t *data2 = port_data(1);
    unsigned int subport;

    for (subport = 0; subport < 4; subport++)
    {
        uint16_t first;
        uint16_t value;
        uint16_t extended;
        uint16_t type;
        unsigned int slot;

        *data2 = (subport << 4) | 0x0C;
        first = th_control_phase(data1);
        if (first & 0x0C00)
            continue;
        (void)th_control_phase(data1);
        extended = th_control_phase(data1);
        value = th_control_phase(data1);
        *data2 = 0x7C;

        if ((value & 0x0F00) != 0x0F00)
        {
            type = JOY_TYPE_PAD3;
            extended = 0x0F00;
        }
        else
        {
            type = JOY_TYPE_PAD6;
            extended = (extended & 0x000F) << 8;
        }

        value = ((first & 0x3000) >> 6) | (first & 0x003F) | extended;
        slot = subport == 0 ? 0 : subport + 1;
        joy_type[slot] = type;
        joy_state[slot] = (value ^ 0x0FFF) & BUTTON_ALL;
    }
}

static RAM_CODE uint16_t detect_port_type(unsigned int port)
{
    volatile uint8_t *data = port_data(port);
    uint8_t value;
    uint16_t id;

    *port_control(port) = 0x40;
    *data = 0x40;
    io_delay();
    value = *data;
    *data = 0x00;
    id = ((value & 0x0C) ? 8 : 0) | ((value & 0x03) ? 4 : 0);
    io_delay();
    value = *data;
    *data = 0x40;
    id |= ((value & 0x0C) ? 2 : 0) | ((value & 0x03) ? 1 : 0);

    return id;
}

static RAM_CODE void clear_controller_state(void)
{
    unsigned int i;

    for (i = 0; i < 8; i++)
    {
        joy_state[i] = 0;
        joy_type[i] = JOY_TYPE_UNKNOWN;
    }
    for (i = 0; i < 2; i++)
    {
        port_type[i] = PORT_TYPE_UNKNOWN;
        port_support[i] = JOY_SUPPORT_OFF;
    }
}

static RAM_CODE uint16_t packed_joy_type(uint16_t type)
{
    if (type == JOY_TYPE_PAD3)
        return 0;
    if (type == JOY_TYPE_PAD6)
        return 1;
    return 3;
}

static RAM_CODE void publish_controller_state(void)
{
    uint16_t s0 = joy_state[0] & BUTTON_ALL;
    uint16_t s1 = joy_state[1] & BUTTON_ALL;
    uint16_t s2 = joy_state[2] & BUTTON_ALL;
    uint16_t s3 = joy_state[3] & BUTTON_ALL;
    uint16_t s4 = joy_state[4] & BUTTON_ALL;
    uint16_t s5 = joy_state[5] & BUTTON_ALL;
    uint16_t s6 = joy_state[6] & BUTTON_ALL;
    uint16_t s7 = joy_state[7] & BUTTON_ALL;

    *(volatile uint16_t *)0xA15126 = (s1 >> 4) | ((s2 & 0x00FF) << 8);
    *(volatile uint16_t *)0xA15128 = (s2 >> 8) | (s3 << 4);
    *(volatile uint16_t *)0xA1512A = s4 | ((s5 & 0x000F) << 12);
    *(volatile uint16_t *)0xA1512C = (s5 >> 4) | ((s6 & 0x00FF) << 8);
    *(volatile uint16_t *)0xA1512E = (s6 >> 8) | (s7 << 4);
    *(volatile uint16_t *)0xA15122 = s0 | ((s1 & 0x000F) << 12);
}

RAM_CODE void controller_update(void)
{
    unsigned int port;

    if (controller_force_reset)
    {
        detected_port_type[0] = PORT_TYPE_UNKNOWN;
        detected_port_type[1] = PORT_TYPE_UNKNOWN;
        detected_ea_four_way = 0;
    }

    clear_controller_state();

    if (detected_ea_four_way)
    {
        port_type[0] = PORT_TYPE_EA4WAYPLAY;
        port_type[1] = PORT_TYPE_EA4WAYPLAY;
        port_support[0] = JOY_SUPPORT_EA4WAYPLAY;
        read_ea_four_way();
    }
    else
    {
        for (port = 0; port < 2; port++)
        {
            uint16_t type = detected_port_type[port];

            if (type == PORT_TYPE_TEAMPLAYER)
            {
                port_type[port] = type;
                port_support[port] = JOY_SUPPORT_TEAMPLAYER;
                read_team_player(port);
            }
            else if (type == PORT_TYPE_PAD)
            {
                port_type[port] = type;
                port_support[port] = JOY_SUPPORT_6BTN;
                joy_state[port] = read_pad(port, &joy_type[port]);
            }
        }
    }

    if (detect_ea_four_way())
    {
        detected_ea_four_way = 1;
        detected_port_type[0] = PORT_TYPE_EA4WAYPLAY;
        detected_port_type[1] = PORT_TYPE_EA4WAYPLAY;
    }
    else
    {
        detected_ea_four_way = 0;
        *port_control(0) = 0x40;
        *port_control(1) = 0x40;
        *port_data(0) = 0x40;
        *port_data(1) = 0x40;
        for (port = 0; port < 2; port++)
            detected_port_type[port] = detect_port_type(port);
    }

    for (port = 0; port < 2; port++)
    {
        port_type[port] = detected_port_type[port];
        if (detected_port_type[port] == PORT_TYPE_TEAMPLAYER)
            port_support[port] = JOY_SUPPORT_TEAMPLAYER;
        else if (detected_port_type[port] == PORT_TYPE_PAD)
            port_support[port] = JOY_SUPPORT_6BTN;
        else if (detected_port_type[port] == PORT_TYPE_EA4WAYPLAY && port == 0)
            port_support[port] = JOY_SUPPORT_EA4WAYPLAY;
        else
            port_support[port] = JOY_SUPPORT_OFF;
    }

    controller_force_reset = 0;
    if (controller_debug_request)
    {
        uint16_t types = 0;

        for (port = 0; port < 8; port++)
            types |= packed_joy_type(joy_type[port]) << (port << 1);

        *(volatile uint16_t *)0xA15126 = types;
        *(volatile uint16_t *)0xA15128 = port_type[0];
        *(volatile uint16_t *)0xA1512A = port_type[1];
        *(volatile uint16_t *)0xA1512C = port_support[0];
        *(volatile uint16_t *)0xA1512E = port_support[1];
        *(volatile uint16_t *)0xA15122 = 0xFFFF;
        controller_debug_request = 0;
    }
    else
        publish_controller_state();
}
