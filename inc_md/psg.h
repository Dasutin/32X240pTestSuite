/**
 *  \file psg.h
 *  \brief PSG support
 *  \author Stephane Dallongeville
 *  \date 08/2011
 *
 * This unit provides access to the PSG through the 68000 CPU
 */

#ifndef _PSG_H
#define _PSG_H

#include "../inc_md/types.h"

// PSG port address
#define PSG_PORT            0xC00011
// Minimum PSG envelope value
#define PSG_ENVELOPE_MIN    15
// Maximum PSG envelope value
#define PSG_ENVELOPE_MAX    0
// Periodic noise type (like low-frequency tone)
#define PSG_NOISE_TYPE_PERIODIC 0
// White noise type (hiss)
#define PSG_NOISE_TYPE_WHITE    1
 // Noise frequency = PSG clock/2 (less coarse)
#define PSG_NOISE_FREQ_CLOCK2   0
// Noise frequency = PSG clock/4
#define PSG_NOISE_FREQ_CLOCK4   1
// Noise frequency = PSG clock / 8 (more coarse)
#define PSG_NOISE_FREQ_CLOCK8   2
// Noise frequency = Tone generator #3
#define PSG_NOISE_FREQ_TONE3    3

// Initialize PSG chip
void PSG_init();
// Write the specified value to PSG data port
void PSG_write(u8 data);

/*
 *  \brief
 *      Set envelope level
 *  \param channel
 *      Channel we want to set envelope (0-3)
 *  \param value
 *      Envelope level to set (#PSG_ENVELOPE_MIN - #PSG_ENVELOPE_MAX)
 * Set envelope level for the specified PSG channel
 */
void PSG_setEnvelope(u8 channel, u8 value);
/*
 *  -brief
 *      Set tone
 *  -param channel
 *      Channel we want to set tone (0-3)
 *  -param value
 *      Tone value to set (0-1023)
 * Set direct tone value for the specified PSG channel
 */
void PSG_setTone(u8 channel, u16 value);
/*
 *  -brief
 *      Set frequency
 *  -param channel
 *      Channel we want to set frequency (0-3)
 *  -param value
 *      Frequency value to set in Hz (0-4095)
 * Set frequency for the specified PSG channel
 * This method actually converts the specified frequency value in PSG tone value
 */
void PSG_setFrequency(u8 channel, u16 value);
/*
 *  -brief
 *      Set noise type and frequency
 *  -param type
 *      Noise type, accepted values are:
 *      #PSG_NOISE_TYPE_PERIODIC<br>
 *      #PSG_NOISE_TYPE_WHITE
 *  -param frequency
 *      Noise frequency, accepted values are:
 *      #PSG_NOISE_FREQ_CLOCK2
 *      #PSG_NOISE_FREQ_CLOCK4
 *      #PSG_NOISE_FREQ_CLOCK8
 *      #PSG_NOISE_FREQ_TONE3
 */
void PSG_setNoise(u8 type, u8 frequency);

#endif