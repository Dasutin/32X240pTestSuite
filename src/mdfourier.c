#include "32x.h"
#include "32x_images.h"
#include "draw.h"
#include "help.h"
#include "hw_32x.h"
#include "shared_objects.h"
#include "sound.h"

#define drawTextwHighlight drawMenuTextwHighlight
#define MDF_SYNC_FREQUENCY 8820
#define MDF_TONE_FRAMES 20

static int mdfCancelled;
static int mdfToneFailed;

static u16 mdfReadPad(void)
{
	u16 button = MARS_SYS_COMM8;

	if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
		button = MARS_SYS_COMM10;
	return button;
}

static int mdfWaitFrames(int frames)
{
	u32 tick = Hw32xGetTicks();

	while (frames-- > 0)
	{
		while (Hw32xGetTicks() == tick)
		{
			if (mdfReadPad() & SEGA_CTRL_START)
			{
				mdfCancelled = 1;
				return 0;
			}
		}
		tick = Hw32xGetTicks();
	}
	return 1;
}

static int mdfStartTone(u32 frequency)
{
	if (!sound_test_pwm_start(frequency, 3))
	{
		mdfToneFailed = 1;
		return 0;
	}
	return 1;
}

static void mdfStopTone(void)
{
	sound_test_pwm_stop();
}

static int mdfPulseTrain(void)
{
	int pulse;

	for (pulse = 0; pulse < 10; pulse++)
	{
		if (!mdfStartTone(MDF_SYNC_FREQUENCY) || !mdfWaitFrames(1))
			return 0;
		mdfStopTone();
		if (!mdfWaitFrames(1))
			return 0;
	}
	return 1;
}

static int mdfSilence(void)
{
	mdfStopTone();
	return mdfWaitFrames(MDF_TONE_FRAMES);
}

static int mdfSteppedTones(void)
{
	u32 frequency;

	for (frequency = 500; frequency <= 20000; frequency += 500)
	{
		if (!mdfStartTone(frequency) || !mdfWaitFrames(MDF_TONE_FRAMES - 1))
			return 0;
		mdfStopTone();
		if (!mdfWaitFrames(1))
			return 0;
	}
	return 1;
}

static int mdfToneRamp(void)
{
	u32 frequency;

	for (frequency = 50; frequency <= 20000; frequency += 50)
	{
		if (!mdfStartTone(frequency) || !mdfWaitFrames(1))
			return 0;
	}
	mdfStopTone();
	return 1;
}

static int mdfRunSequence(void)
{
	mdfCancelled = 0;
	mdfToneFailed = 0;
	if (!mdfPulseTrain())
		return 0;
	if (!mdfSilence())
		return 0;
	if (!mdfSteppedTones())
		return 0;
	if (!mdfToneRamp())
		return 0;
	if (!mdfSilence())
		return 0;
	if (!mdfPulseTrain())
		return 0;
	mdfStopTone();
	return 1;
}

static void mdfDrawScreen(int state)
{
	draw_dirtyrect(&tm, 32, 80, 256, 40);
	draw_dirtyrect(&tm, 48, 176, 224, 16);
	draw_tilemap(&tm, 0, 0, 0, NULL, NULL);
	invalidateMenuText();
	drawMainBG();
	drawTextwHighlight("MDFourier", 120, 48,
		fontColorGreen, fontColorGreenHighlight);
	drawTextwHighlight("Auto sequence", 76, 72,
		fontColorGreen, fontColorGreenHighlight);
	if (state == 0)
		drawTextwHighlight("Start recording and press A", 56, 88,
			fontColorRed, fontColorRedHighlight);
	else if (state == 1)
		drawTextwHighlight("Please wait while recording", 56, 88,
			fontColorRed, fontColorRedHighlight);
	else if (state == 2)
	{
		drawTextwHighlight("You can now stop recording.", 48, 88,
			fontColorWhite, fontColorWhiteHighlight);
		drawTextwHighlight("Press any button to continue", 48, 104,
			fontColorRed, fontColorRedHighlight);
	}
	else
	{
		drawTextwHighlight("PWM tone generator unavailable", 40, 88,
			fontColorRed, fontColorRedHighlight);
		drawTextwHighlight("Press any button to continue", 48, 104,
			fontColorWhite, fontColorWhiteHighlight);
	}
	if (state == 0)
		drawTextwHighlight("Press Z for help", 96, 180,
			fontColorWhite, fontColorWhiteHighlight);
	else if (state >= 2)
		drawTextwHighlight("Press START to exit", 88, 180,
			fontColorWhite, fontColorWhiteHighlight);
}

void MDFourier(void)
{
	int done = 0;
	int state = 0;
	u16 oldButton = 0xFFFF;

	initMainBG();
	Hw32xScreenFlip(0);
	Mars_InitSoundDMA();
	mdfCancelled = 0;
	if (mdfStartTone(100))
	{
		mdfWaitFrames(1);
		mdfStopTone();
	}
	mdfToneFailed = 0;

	while (!done)
	{
		u16 button;
		u16 pressedButton;

		Hw32xFlipWait();
		mdfDrawScreen(state);
		button = mdfReadPad();
		pressedButton = button & ~oldButton;
		oldButton = button;

		if (pressedButton & (SEGA_CTRL_B | SEGA_CTRL_START))
			done = 1;
		else if (state >= 2 && pressedButton)
			state = 0;
		else if (state == 0 && (pressedButton & SEGA_CTRL_Z))
		{
			screenFadeOut(1);
			DrawHelp(HELP_MDFOURIER);
			initMainBG();
			Hw32xScreenFlip(0);
			oldButton = mdfReadPad();
		}
		else if (state == 0 && (pressedButton & SEGA_CTRL_A))
		{
			state = 1;
			mdfDrawScreen(state);
			Hw32xScreenFlip(0);
			Mars_StopSoundMixer();
			mdfRunSequence();
			mdfStopTone();
			Mars_StartSoundMixer();
			if (mdfToneFailed)
				state = 3;
			else if (mdfCancelled)
				state = 0;
			else
				state = 2;
			oldButton = mdfReadPad();
		}

		if (!done)
			Hw32xScreenFlip(0);
	}

	mdfStopTone();
	screenFadeOut(1);
}
