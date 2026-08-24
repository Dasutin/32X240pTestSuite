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

#include "32x.h"
#include "perf.h"

volatile uint16_t perf_current_scene = PERF_SCENE_OTHER;
perf_counter_t perf_counters[PERF_CPU_COUNT][PERF_SCENE_COUNT]
	[PERF_METRIC_COUNT] ATTR_CACHE_ALIGNED;

#define PERF_UNCACHED(pointer) \
	((void *)((uintptr_t)(pointer) | 0x20000000u))

static uint32_t perf_ticks(void)
{
	uint32_t high = SH2_FRT_FRCH;
	uint32_t low = SH2_FRT_FRCL;

	/* Reading FRCH latches FRCL in the FRT temporary register. */
	return (high << 8) | low;
}

uint32_t perf_master_ticks(void)
{
	return perf_ticks();
}

uint32_t perf_slave_ticks(void)
{
	return perf_ticks();
}

void perf_record(perf_cpu_t cpu, perf_metric_t metric, uint32_t elapsed)
{
	volatile perf_counter_t *counter;
	uint16_t scene = *(volatile uint16_t *)PERF_UNCACHED(
		&perf_current_scene);

	if ((unsigned)cpu >= PERF_CPU_COUNT ||
		(unsigned)metric >= PERF_METRIC_COUNT)
		return;
	if (scene >= PERF_SCENE_COUNT)
		scene = PERF_SCENE_OTHER;

	/* Every measured region is shorter than one 16-bit FRT period. */
	elapsed &= 0xFFFF;

	/* Bypass both SH-2 caches so the other CPU and debugger see each sample. */
	counter = (volatile perf_counter_t *)PERF_UNCACHED(
		&perf_counters[cpu][scene][metric]);
	counter->ticks += elapsed;
	counter->calls++;
	if (elapsed > counter->maximum)
		counter->maximum = elapsed;
}

void perf_record_sprite(perf_cpu_t cpu, const void *source, uint32_t elapsed)
{
	uintptr_t address = (uintptr_t)source & 0x1FFFFFFF;

	perf_record(cpu, PERF_METRIC_SPRITE, elapsed);
	if (address >= 0x06000000 && address < 0x06040000)
		perf_record(cpu, PERF_METRIC_SPRITE_SDRAM, elapsed);
	else
		perf_record(cpu, PERF_METRIC_SPRITE_ROM, elapsed);
}

void perf_reset(void)
{
	unsigned cpu;
	unsigned scene;
	unsigned metric;

	for (cpu = 0; cpu < PERF_CPU_COUNT; cpu++)
		for (scene = 0; scene < PERF_SCENE_COUNT; scene++)
			for (metric = 0; metric < PERF_METRIC_COUNT; metric++)
			{
				volatile perf_counter_t *counter =
					(volatile perf_counter_t *)PERF_UNCACHED(
						&perf_counters[cpu][scene][metric]);
				counter->ticks = 0;
				counter->calls = 0;
				counter->maximum = 0;
			}
}

void perf_set_scene(perf_scene_t scene)
{
	if ((unsigned)scene < PERF_SCENE_COUNT)
		*(volatile uint16_t *)PERF_UNCACHED(&perf_current_scene) = scene;
}
