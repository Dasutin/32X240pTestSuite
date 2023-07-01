/* 
 * 240p Test Suite for the Sega 32X
 * Port by Dasutin (Dustin Dembrosky)
 * Copyright (C)2011-2023 Artemio Urbina
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

#include <math.h>
#include "types.h"
#include "string.h"
#include "32x.h"
#include "hw_32x.h"
#include "32x_images.h"
#include "shared_objects.h"
#include "main.h"
#include "help.h"

extern int fontColorWhite, fontColorRed, fontColorGreen, fontColorGray, fontColorBlack;
extern int fontColorWhiteHighlight, fontColorRedHighlight, fontColorGreenHighlight;

void DrawHelp(int option)
{
	unsigned short button, pressedButton, oldButton = 0xFFFF;
	int exit = 0;
	int page = 1;
	int totalpages = 1;

	marsVDP256Start();

	switch (option)
	{
	case HELP_GENERAL:
	case HELP_STRIPES:
	case HELP_SHADOW:
	case HELP_LED:
	case HELP_LAG:
	case HELP_MANUALLAG:
	case HELP_HSCROLL:
	case HELP_MEMVIEW:
		totalpages = 2;
		break;
	case HELP_PLUGE:
	case HELP_MONOSCOPE:
		totalpages = 3;
		break;
	}

	Hw32xScreenFlip(0);

	while (!exit)
	{
		Hw32xFlipWait();

		button = MARS_SYS_COMM8;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
		{
			button = MARS_SYS_COMM10;
		}

		pressedButton = button & ~oldButton;
		oldButton = button & 0x0FFF;

		DrawMainBG();
		loadTextPalette();

		switch (option)
			{
			case HELP_GENERAL:
				switch (page)
				{
				case 1:
					mars_drawTextwShadow("HELP (1/2)", 125, 35, fontColorGreen, fontColorGreenHighlight);

					mars_drawTextwShadow("The 240p Test Suite was designed", 35, 57, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("with two goals in mind:", 35, 65, fontColorWhite, fontColorWhiteHighlight);

					mars_drawTextwShadow("1) Evaluate 240p signals on TV", 35, 81, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("sets and video processors; and", 35, 90, fontColorWhite, fontColorWhiteHighlight);

					mars_drawTextwShadow("2) Provide calibration patterns", 35, 106, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("from a game console to help in", 35, 114, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("properly calibrating the display", 35, 122, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("black, white, and color levels.", 35, 130, fontColorWhite, fontColorWhiteHighlight);

					mars_drawTextwShadow("Help can be shown with 'Z' in", 35, 146, fontColorGreen, fontColorGreenHighlight);
					mars_drawTextwShadow("any test.", 35, 154, fontColorGreen, fontColorGreenHighlight);

					mars_drawTextwShadow("(cont...)", 207, 178, fontColorWhite, fontColorWhiteHighlight);
					if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_THREE)
					{
						mars_drawTextwShadow("Press C to exit help", 91, 193, fontColorWhite, fontColorWhiteHighlight);
					}
					else
					{
						mars_drawTextwShadow("Press Z to exit help", 91, 193, fontColorWhite, fontColorWhiteHighlight);
					}
					break;
				case 2:
					mars_drawTextwShadow("HELP (2/2)", 125, 35, fontColorGreen, fontColorGreenHighlight);

					mars_drawTextwShadow("The Sega 32X can output 224", 35, 50, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("active video lines. In PAL", 35, 58, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("consoles, it can display either", 35, 66, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("224 or 240 lines.", 35, 74, fontColorWhite, fontColorWhiteHighlight);

					mars_drawTextwShadow("The 240p suite is also available", 35, 90, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("on NES/FC, SNES/SFC, GameCube,", 35, 98, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("Wii, GBA, Neo Geo MVS/AES and CD,", 35, 106, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("Dreamcast, Genesis/Mega Drive,", 35, 114, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("Sega/Mega CD, SMS, and", 35, 122, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("PCE/TG-16/PCE-Duo/SCD/SCD2", 35, 130, fontColorWhite, fontColorWhiteHighlight);

					mars_drawTextwShadow("Visit:", 35, 150, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("http://junkerhq.net/240p", 35, 160, fontColorGreen, fontColorGreenHighlight);
					mars_drawTextwShadow("for more information", 35, 170, fontColorWhite, fontColorWhiteHighlight);

					if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_THREE)
					{
						mars_drawTextwShadow("Press C to exit help", 91, 193, fontColorWhite, fontColorWhiteHighlight);
					}
					else
					{
						mars_drawTextwShadow("Press Z to exit help", 91, 193, fontColorWhite, fontColorWhiteHighlight);
					}
					break;
				}
				break;
			case HELP_PLUGE:
				switch (page)
				{
				case 1:
					mars_drawTextwShadow("PLUGE (1/3)", 115, 35, fontColorGreen, fontColorGreenHighlight);

					mars_drawTextwShadow("NTSC levels require black to be", 32, 57, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("at 7.5 IRE for video. This HW", 32, 65, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("lowest is 6 IRE (6%), so using", 32, 73, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("this value for general 240p use", 32, 81, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("is not recommended.", 32, 89, fontColorWhite, fontColorWhiteHighlight);

					mars_drawTextwShadow("Of course using it as reference", 32, 106, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("will work perfectly for games", 32, 114, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("in this platform.", 32, 122, fontColorWhite, fontColorWhiteHighlight);

					mars_drawTextwShadow("In PAL - and console gaming in", 32, 139, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("general - it is adviced to use", 32, 147, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("a value of 2 IRE as black.", 32, 155, fontColorWhite, fontColorWhiteHighlight);

					mars_drawTextwShadow("(cont...)", 207, 178, fontColorWhite, fontColorWhiteHighlight);
					break;
				case 2:
					mars_drawTextwShadow("PLUGE (2/3)", 115, 35, fontColorGreen, fontColorGreenHighlight);

					mars_drawTextwShadow("The PLUGE pattern is used to", 32, 57, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("help adjust the black level to", 32, 65, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("a correct value.", 32, 73, fontColorWhite, fontColorWhiteHighlight);

					mars_drawTextwShadow("The inner bars on the sides are", 32, 90, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("black at 6%, the outer at 12%.", 32, 98, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("If these bars are not visible,", 32, 106, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("adjust the \"brightness\" control", 32, 114, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("until they are.", 32, 122, fontColorWhite, fontColorWhiteHighlight);

					mars_drawTextwShadow("You should lower it until they", 32, 139, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("are not visible, and raise it", 32, 147, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("until they show.", 32, 155, fontColorWhite, fontColorWhiteHighlight);

					mars_drawTextwShadow("(cont...)", 207, 178, fontColorWhite, fontColorWhiteHighlight);
					break;
				case 3:
					mars_drawTextwShadow("PLUGE (3/3)", 115, 35, fontColorGreen, fontColorGreenHighlight);

					mars_drawTextwShadow("Within it A button changes", 32, 56, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("palettes between the original,", 32, 64, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("highest and lowest values the", 32, 72, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("hardware can display.", 32, 80, fontColorWhite, fontColorWhiteHighlight);
					break;
				}
				break;
			case HELP_COLORS:
				mars_drawTextwShadow("COLORBARS", 125, 35, fontColorGreen, fontColorGreenHighlight);

				mars_drawTextwShadow("This pattern allows you to", 32, 57, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("calibrate each color: Red, Green", 32, 65, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("and Blue; as well as white.", 32, 73, fontColorWhite, fontColorWhiteHighlight);

				mars_drawTextwShadow("Adjust the white level first,", 32, 90, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("using the \"Contrast\" control", 32, 98, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("on your TV set. Raise it until", 32, 106, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("you cannot distinguish between", 32, 114, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("the blocks under \"E\" and \"F\",", 32, 122, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("and lower it slowly until you", 32, 130, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("can clearly tell them appart.", 32, 138, fontColorWhite, fontColorWhiteHighlight);

				mars_drawTextwShadow("Do the same for each color.", 32, 155, fontColorWhite, fontColorWhiteHighlight);
				break;
			case HELP_GRID:
				mars_drawTextwShadow("GRID", 140, 35, fontColorGreen, fontColorGreenHighlight);

				mars_drawTextwShadow("This grid uses the full 320x224", 32, 57, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("resolution.", 32, 65, fontColorWhite, fontColorWhiteHighlight);

				mars_drawTextwShadow("You can use it to verify that", 32, 82, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("all the visible area is being", 32, 90, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("displayed, and that there is no", 32, 98, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("distortion present.", 32, 106, fontColorWhite, fontColorWhiteHighlight);

				mars_drawTextwShadow("The full active video signal can", 32, 123, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("be filled with gray by pressing", 32, 131, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("the 'A' button.", 32, 139, fontColorWhite, fontColorWhiteHighlight);
				break;
			case HELP_MONOSCOPE:
				switch (page)
				{
					case 1:
						mars_drawTextwShadow("MONOSCOPE (1/3)", 95, 35, fontColorGreen, fontColorGreenHighlight);

						mars_drawTextwShadow("This pattern contains elements", 32, 57, fontColorWhite, fontColorWhiteHighlight);
						mars_drawTextwShadow("to calibrate multiple aspects", 32, 65, fontColorWhite, fontColorWhiteHighlight);
						mars_drawTextwShadow("of a CRT.", 32, 73, fontColorWhite, fontColorWhiteHighlight);

						mars_drawTextwShadow("Read your monitor's service", 32, 90, fontColorWhite, fontColorWhiteHighlight);
						mars_drawTextwShadow("manual to learn how, and use", 32, 98, fontColorWhite, fontColorWhiteHighlight);
						mars_drawTextwShadow("'A' button to change IRE.", 32, 106, fontColorWhite, fontColorWhiteHighlight);

						mars_drawTextwShadow("Brightness adjustment: Adjust", 32, 123, fontColorWhite, fontColorWhiteHighlight);
						mars_drawTextwShadow("convergence at low brightness", 32, 131, fontColorWhite, fontColorWhiteHighlight);
						mars_drawTextwShadow("(13/25 IRE). An overly bright", 32, 139, fontColorWhite, fontColorWhiteHighlight);
						mars_drawTextwShadow("pattern can mask convergence.", 32, 147, fontColorWhite, fontColorWhiteHighlight);
						mars_drawTextwShadow("issues.", 32, 155, fontColorWhite, fontColorWhiteHighlight);

						mars_drawTextwShadow("(cont...)", 207, 178, fontColorWhite, fontColorWhiteHighlight);
					break;
					case 2:
						mars_drawTextwShadow("MONOSCOPE (2/3)", 95, 35, fontColorGreen, fontColorGreenHighlight);

						mars_drawTextwShadow("Convergence: Use the center", 32, 57, fontColorWhite, fontColorWhiteHighlight);
						mars_drawTextwShadow("crosshair to check static", 32, 65, fontColorWhite, fontColorWhiteHighlight);
						mars_drawTextwShadow("(center of screen) convergence.", 32, 73, fontColorWhite, fontColorWhiteHighlight);
						mars_drawTextwShadow("Use the patterns at the sides", 32, 81, fontColorWhite, fontColorWhiteHighlight);
						mars_drawTextwShadow("to check dynamic (edge)", 32, 89, fontColorWhite, fontColorWhiteHighlight);
						mars_drawTextwShadow("convergence.", 32, 97, fontColorWhite, fontColorWhiteHighlight);

						mars_drawTextwShadow("Corners: After setting center", 32, 114, fontColorWhite, fontColorWhiteHighlight);
						mars_drawTextwShadow("and edge convergence, use", 32, 122, fontColorWhite, fontColorWhiteHighlight);
						mars_drawTextwShadow("magnets to adjust corner purity", 32, 130, fontColorWhite, fontColorWhiteHighlight);
						mars_drawTextwShadow("and geometry.", 32, 138, fontColorWhite, fontColorWhiteHighlight);

						mars_drawTextwShadow("(cont...)", 207, 178, fontColorWhite, fontColorWhiteHighlight);
					break;
					case 3:
						mars_drawTextwShadow("MONOSCOPE (3/3)", 95, 35, fontColorGreen, fontColorGreenHighlight);

						mars_drawTextwShadow("Size and aspect ratio: If", 32, 57, fontColorWhite, fontColorWhiteHighlight);
						mars_drawTextwShadow("vertical and horizontal size are", 32, 65, fontColorWhite, fontColorWhiteHighlight);
						mars_drawTextwShadow("correct, the red squares in the", 32, 73, fontColorWhite, fontColorWhiteHighlight);
						mars_drawTextwShadow("pattern will be perfect squares.", 32, 81, fontColorWhite, fontColorWhiteHighlight);
						mars_drawTextwShadow("After setting H size, use a tape", 32, 89, fontColorWhite, fontColorWhiteHighlight);
						mars_drawTextwShadow("measure to adjust V size to", 32, 97, fontColorWhite, fontColorWhiteHighlight);
						mars_drawTextwShadow("match it.", 32, 105, fontColorWhite, fontColorWhiteHighlight);

						mars_drawTextwShadow("Linearity: The squares in each", 32, 122, fontColorWhite, fontColorWhiteHighlight);
						mars_drawTextwShadow("corner should get you started.", 32, 130, fontColorWhite, fontColorWhiteHighlight);
						mars_drawTextwShadow("Confirm your adjustment using", 32, 138, fontColorWhite, fontColorWhiteHighlight);
						mars_drawTextwShadow("the scroll tests.", 32, 146, fontColorWhite, fontColorWhiteHighlight);

						mars_drawTextwShadow("Designed by Keith Raney.", 32, 164, fontColorWhite, fontColorWhiteHighlight);
					break;
				}
				break;
			case HELP_BLEED:
				mars_drawTextwShadow("COLOR BLEED", 123, 35, fontColorGreen, fontColorGreenHighlight);
				mars_drawTextwShadow("This pattern helps diagnose", 32, 56, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("color bleed caused by", 32, 64, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("unneeded color upsampling.", 32, 72, fontColorWhite, fontColorWhiteHighlight);

				mars_drawTextwShadow("You can toggle between", 32, 90, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("vertical bars and", 32, 98, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("checkerboard with 'A'.", 32, 106, fontColorWhite, fontColorWhiteHighlight);
				break;
			case HELP_IRE:
				mars_drawTextwShadow("100 IRE", 125, 35, fontColorGreen, fontColorGreenHighlight);
				mars_drawTextwShadow("You can vary IRE intensity", 32, 56, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("with 'A' and 'B'. Values are:", 32, 64, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("13, 25, 41, 53, 66, 82, 94", 32, 72, fontColorWhite, fontColorWhiteHighlight);
				break;
			case HELP_601CB:
				mars_drawTextwShadow("601 COLORBARS", 120, 35, fontColorGreen, fontColorGreenHighlight);
				mars_drawTextwShadow("You can use color filters or the", 32, 56, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("blue only option in your display", 32, 64, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("to confirm color balance.", 32, 72, fontColorWhite, fontColorWhiteHighlight);
				break;
			case HELP_SHARPNESS:
				mars_drawTextwShadow("SHARPNESS", 125, 35, fontColorGreen, fontColorGreenHighlight);
				mars_drawTextwShadow("You should set the sharpness of", 32, 56, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("your CRT to a value that shows", 32, 64, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("clean black and gray transitions", 32, 72, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("with no white ghosting between.", 32, 80, fontColorWhite, fontColorWhiteHighlight);

				mars_drawTextwShadow("Pressing 'A' toggles a diagonal", 32, 96, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("brick tileset.", 32, 104, fontColorWhite, fontColorWhiteHighlight);
				break;
			case HELP_OVERSCAN:
				mars_drawTextwShadow("OVERSCAN", 125, 35, fontColorGreen, fontColorGreenHighlight);
				mars_drawTextwShadow("With this pattern you can", 32, 56, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("interactively find out the", 32, 64, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("overscan in pixels of each edge", 32, 72, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("in a display.", 32, 80, fontColorWhite, fontColorWhiteHighlight);

				mars_drawTextwShadow("Use left and right to increment", 32, 96, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("the overscan until you see the", 32, 104, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("white border, then go back one", 32, 112, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("pixel. The resulting number is", 32, 120, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("the amount of overscan in pixels", 32, 128, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("in each direction.", 32, 136, fontColorWhite, fontColorWhiteHighlight);
				break;
			case HELP_SMPTE:
				mars_drawTextwShadow("SMPTE COLOR BARS", 90, 35, fontColorGreen, fontColorGreenHighlight);

				mars_drawTextwShadow("This pattern can be used to", 32, 56, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("approximate for NTSC levels", 32, 64, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("regarding contrast,", 32, 72, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("brightness and colors.", 32, 80, fontColorWhite, fontColorWhiteHighlight);

				mars_drawTextwShadow("You can toggle between 75% and", 32, 96, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("100% SMPTE color bars with A.", 32, 104, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("Of course the percentages are", 32, 112, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("relative to the console output.", 32, 120, fontColorWhite, fontColorWhiteHighlight);

				mars_drawTextwShadow("You can use color filters or the", 32, 136, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("blue only option in your display", 32, 144, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("to confirm color balance", 32, 152, fontColorWhite, fontColorWhiteHighlight);
				break;
			case HELP_GRAY:
				mars_drawTextwShadow("GRAY RAMP", 125, 35, fontColorGreen, fontColorGreenHighlight);
				mars_drawTextwShadow("This gray ramp pattern can be", 32, 56, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("used to check color balance.", 32, 64, fontColorWhite, fontColorWhiteHighlight);

				mars_drawTextwShadow("You should make sure the bars", 32, 80, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("are gray, with no color bias.", 32, 88, fontColorWhite, fontColorWhiteHighlight);
				break;
			case HELP_WHITE:
				mars_drawTextwShadow("WHITE SCREEN", 115, 35, fontColorGreen, fontColorGreenHighlight);
				mars_drawTextwShadow("This pattern can be changed", 32, 56, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("between white, black, red,", 32, 64, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("green and blue screens with the", 32, 72, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("'A' and 'B' buttons.", 32, 80, fontColorWhite, fontColorWhiteHighlight);

				mars_drawTextwShadow("Some displays and scalers have", 32, 96, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("issues when changing between a", 32, 104, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("black 0 IRE and a white screen.", 32, 112, fontColorWhite, fontColorWhiteHighlight);

				mars_drawTextwShadow("A custom color mode is", 32, 128, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("available by pressing 'C'.", 32, 136, fontColorWhite, fontColorWhiteHighlight);
				break;
			case HELP_CONVERGENCE:
				mars_drawTextwShadow("CONVERGENCE TESTS", 90, 35, fontColorGreen, fontColorGreenHighlight);
				mars_drawTextwShadow("These are used to adjust color", 32, 56, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("convergence in CRT displays", 32, 64, fontColorWhite, fontColorWhiteHighlight);

				mars_drawTextwShadow("The 'A' button changes the cross", 32, 80, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("hatch pattern between lines,", 32, 88, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("dots and crosses", 32, 96, fontColorWhite, fontColorWhiteHighlight);

				mars_drawTextwShadow("The 'B' button changes to a", 32, 112, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("color pattern for transition", 32, 120, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("boundary check.", 32, 128, fontColorWhite, fontColorWhiteHighlight);
				break;
			case HELP_CHECK:
				mars_drawTextwShadow("CHECKERBOARD", 115, 35, fontColorGreen, fontColorGreenHighlight);
				mars_drawTextwShadow("This pattern shows all the", 32, 57, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("visible pixels in an", 32, 65, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("alternating white and black", 32, 73, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("grid array.", 32, 81, fontColorWhite, fontColorWhiteHighlight);

				mars_drawTextwShadow("You can toggle the pattern with", 32, 98, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("button 'Up', or turn on", 32, 106, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("auto-toggle each frame with the", 32, 114, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("'A' button. A frame counter is", 32, 122, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("also available with 'B'.", 32, 130, fontColorWhite, fontColorWhiteHighlight);
				break;
			case HELP_STRIPES:
				switch (page)
				{
				case 1:
					mars_drawTextwShadow("HOR/VER STRIPES (1/2)", 75, 35, fontColorGreen, fontColorGreenHighlight);
					mars_drawTextwShadow("You should see a pattern of", 32, 57, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("lines, each one pixel in height", 32, 65, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("starting with a white one.", 32, 73, fontColorWhite, fontColorWhiteHighlight);

					mars_drawTextwShadow("You can toggle the pattern with", 32, 90, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("button 'UP', or turn on", 32, 98, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("auto-toggle each frame with the", 32, 106, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("'A' button. A frame counter is", 32, 114, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("also available with 'B'.", 32, 122, fontColorWhite, fontColorWhiteHighlight);

					mars_drawTextwShadow("When auto-toggle is set, you", 32, 139, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("should see the lines", 32, 147, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("alternating rapidly.", 32, 155, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("(cont...)", 207, 178, fontColorWhite, fontColorWhiteHighlight);
					break;
				case 2:
					mars_drawTextwShadow("HOR/VER STRIPES (2/2)", 75, 35, fontColorGreen, fontColorGreenHighlight);
					mars_drawTextwShadow("You can also display vertical", 32, 57, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("bars by pressing 'LEFT'. That", 32, 65, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("pattern will help you", 32, 73, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("evaluate if the signal is not", 32, 81, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("distorted horizontaly, since", 32, 89, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("all lines should be one pixel", 32, 97, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("wide.", 32, 105, fontColorWhite, fontColorWhiteHighlight);
					break;
				}
				break;
			case HELP_SHADOW:
				mars_drawTextwShadow("DROP SHADOW TEST", 96, 35, fontColorGreen, fontColorGreenHighlight);
				mars_drawTextwShadow("This is a crucial test for 240p", 32, 56, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("It displays a simple sprite", 32, 64, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("shadow against a background,", 32, 72, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("but the shadow is shown only", 32, 80, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("on each other frame. Achieving", 32, 88, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("a transparency effect.", 32, 96, fontColorWhite, fontColorWhiteHighlight);

				mars_drawTextwShadow("The user can toggle the frame", 32, 112, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("used to draw the shadow with", 32, 120, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("button 'A'. Backgrounds can be", 32, 128, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("switched with the 'B' button,", 32, 136, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("and button 'C' toggles sprites.", 32, 144, fontColorWhite, fontColorWhiteHighlight);
				break;
			case HELP_STRIPED:
				mars_drawTextwShadow("STRIPED SPRITE TEST", 84, 35, fontColorGreen, fontColorGreenHighlight);
				mars_drawTextwShadow("There are deinterlacers out", 32, 57, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("there that can display the drop", 32, 65, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("shadows correctly and still", 32, 73, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("interpret 240p as 480i. With a", 32, 81, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("striped sprite it should be easy", 32, 89, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("to tell if a processor tries to", 32, 97, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("deinterlace (plus interpolate).", 32, 105, fontColorWhite, fontColorWhiteHighlight);

				mars_drawTextwShadow("You can change backgrounds with", 32, 122, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("'A'.", 32, 130, fontColorWhite, fontColorWhiteHighlight);
				break;
			case HELP_MANUALLAG:
				switch (page)
				{
				case 1:
					mars_drawTextwShadow("TIMING & REFLEX (1/2)", 75, 35, fontColorGreen, fontColorGreenHighlight);
					mars_drawTextwShadow("The main intention is to show a", 32, 56, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("changing pattern on the screen,", 32, 64, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("which can be complemented with", 32, 72, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("audio. This should show to some", 32, 80, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("degree any lag when processing", 32, 88, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("the signal.", 32, 96, fontColorWhite, fontColorWhiteHighlight);

					mars_drawTextwShadow("As an added feature, the user", 32, 112, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("can click the 'A' button when", 32, 120, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("the sprite is aligned with the", 32, 128, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("one on the background, and the", 32, 136, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("offset in frames from the actual", 32, 144, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("intersection will be shown on", 32, 152, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("screen. A 1khz tone will be", 32, 160, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("played for 1 frame when pressed.", 32, 168, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("(cont...)", 207, 178, fontColorWhite, fontColorWhiteHighlight);
					break;
				case 2:
					mars_drawTextwShadow("TIMING & REFLEX (2/2)", 75, 35, fontColorGreen, fontColorGreenHighlight);
					mars_drawTextwShadow("Button 'B' can be used to", 32, 56, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("change the direction of the", 32, 64, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("sprite.", 32, 72, fontColorWhite, fontColorWhiteHighlight);

					mars_drawTextwShadow("Of course the evaluation is", 32, 88, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("dependent on reflexes and/or", 32, 96, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("rhythm more than anything. The", 32, 104, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("visual and audio cues are the", 32, 112, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("more revealing aspects which", 32, 120, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("the user should consider, of", 32, 128, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("course the interactive factor", 32, 136, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("can give an experienced player", 32, 144, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("the hang of the system when", 32, 152, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("testing via different", 32, 160, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("connections.", 32, 168, fontColorWhite, fontColorWhiteHighlight);
					break;
				}
				break;
			case HELP_HSCROLL:
				mars_drawTextwShadow("SCROLL TEST", 115, 35, fontColorGreen, fontColorGreenHighlight);
				mars_drawTextwShadow("This test shows either an", 32, 57, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("horizontal 320x224 back from", 32, 65, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("Sonic or a vertical 256x224", 32, 73, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("back from Kiki Kaikai.", 32, 81, fontColorWhite, fontColorWhiteHighlight);

				mars_drawTextwShadow("Speed can be varied with Up &", 32, 98, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("Down and scroll direction with", 32, 106, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("Left. The 'A' button stops the", 32, 114, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("scroll and 'B' toggles between", 32, 122, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("vertical and horizontal.", 32, 130, fontColorWhite, fontColorWhiteHighlight);

				mars_drawTextwShadow("This can be used to notice any", 32, 147, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("drops in framerate, or pixel", 32, 155, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("width inconsistencies.", 32, 163, fontColorWhite, fontColorWhiteHighlight);
				break;
			case HELP_VSCROLL:
				mars_drawTextwShadow("GRID SCROLL TEST", 104, 35, fontColorGreen, fontColorGreenHighlight);
				mars_drawTextwShadow("A grid is scrolled vertically or", 32, 57, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("horizontally, which can be used", 32, 65, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("to test linearity of the signal", 32, 73, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("and how well the display or", 32, 81, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("video processor copes with", 32, 89, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("scrolling and framerate.", 32, 97, fontColorWhite, fontColorWhiteHighlight);

				mars_drawTextwShadow("Button 'B' can be used to toggle", 32, 114, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("between horizontal and vertical,", 32, 122, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("while Up/Down regulate speed.", 32, 130, fontColorWhite, fontColorWhiteHighlight);

				mars_drawTextwShadow("Button 'A' stops the scroll and", 32, 147, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("'Left' changes direction.", 32, 155, fontColorWhite, fontColorWhiteHighlight);
				break;
				case HELP_SOUND:
				mars_drawTextwShadow("SOUND TEST", 125, 35, fontColorGreen, fontColorGreenHighlight);
				mars_drawTextwShadow("You can test the sound from the", 32, 56, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("32X 2-channel PWM and", 32, 64, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("Genesis/Mega Drive PSG here.", 32, 72, fontColorWhite, fontColorWhiteHighlight);

				mars_drawTextwShadow("Panning can be changed when", 32, 88, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("possible. This can help you", 32, 96, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("identify stereo cabling issues.", 32, 104, fontColorWhite, fontColorWhiteHighlight);

				mars_drawTextwShadow("PSG has 200hz, 2khz and 4khz", 32, 120, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("tones on its channels and white", 32, 128, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("noise at 500hz.", 32, 136, fontColorWhite, fontColorWhiteHighlight);
				break;
		case HELP_LED:
				switch (page)
				{
				case 1:
					mars_drawTextwShadow("BACKLIT TEST", 106, 35, fontColorGreen, fontColorGreenHighlight);
					mars_drawTextwShadow("This test allows you to check", 32, 57, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("how the display's backlit works", 32, 65, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("when only a small array of", 32, 73, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("pixels is shown.", 32, 81, fontColorWhite, fontColorWhiteHighlight);

					mars_drawTextwShadow("The user can move around the", 32, 97, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("white pixel arrays with the", 32, 105, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("d-pad, and change the size of", 32, 113, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("the pixel array with 'A'.", 32, 121, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("The 'B' button allows the user", 32, 129, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("to hide the pixel array in", 32, 137, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("order to alternate a fully black", 32, 145, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("screen.", 32, 153, fontColorWhite, fontColorWhiteHighlight);
					break;
				}
				break;
			case HELP_LAG:
				switch (page)
				{
				case 1:
					mars_drawTextwShadow("LAG TEST (1/2)", 125, 35, fontColorGreen, fontColorGreenHighlight);
					mars_drawTextwShadow("This test is designed to be used", 32, 57, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("with two displays conected at", 32, 65, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("the same time. One being a CRT,", 32, 73, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("or a display with a known lag as", 32, 81, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("reference, and the other the", 32, 89, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("display to test.", 32, 97, fontColorWhite, fontColorWhiteHighlight);

					mars_drawTextwShadow("Using a camera, a picture should", 32, 114, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("be taken of both screens at the", 32, 122, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("same time. The picture will show", 32, 130, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("the frame discrepancy between", 32, 138, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("them.", 32, 146, fontColorWhite, fontColorWhiteHighlight);

					mars_drawTextwShadow("The circles in the bottom help", 32, 163, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("determine the frame even when", 32, 171, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("(cont...)", 207, 178, fontColorWhite, fontColorWhiteHighlight);
					break;
				case 2:
					mars_drawTextwShadow("LAG TEST (2/2)", 125, 35, fontColorGreen, fontColorGreenHighlight);
					mars_drawTextwShadow("the numbers are blurry.", 32, 57, fontColorWhite, fontColorWhiteHighlight);

					mars_drawTextwShadow("This version of the suite can be", 32, 74, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("used with a Sega Nomad as the", 32, 82, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("reference display.", 32, 90, fontColorWhite, fontColorWhiteHighlight);

					mars_drawTextwShadow("You can also split the video", 32, 107, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("signal and feed both displays.", 32, 115, fontColorWhite, fontColorWhiteHighlight);

					mars_drawTextwShadow("The vertical bars on the sides", 32, 132, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("change color each frame to help", 32, 140, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("when using LCD photos.", 32, 148, fontColorWhite, fontColorWhiteHighlight);

					mars_drawTextwShadow("Press A to start/stop, B to", 32, 165, fontColorWhite, fontColorWhiteHighlight);
					mars_drawTextwShadow("reset and C for B&W test.", 32, 173, fontColorWhite, fontColorWhiteHighlight);
					break;
				}
				break;
			case HELP_ALTERNATE:
				mars_drawTextwShadow("ALTERNATE 240p/480i", 125, 35, fontColorGreen, fontColorGreenHighlight);
				mars_drawTextwShadow("Some devices have a delay when", 32, 57, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("the source changes between", 32, 65, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("resolutions, which happens in", 32, 73, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("some games. This test allows to", 32, 81, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("time that delay manually.", 32, 89, fontColorWhite, fontColorWhiteHighlight);

				mars_drawTextwShadow("Press A to switch the", 32, 106, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("resolution; press it again when", 32, 114, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("you are able to see the screen", 32, 122, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("back in your display.", 32, 130, fontColorWhite, fontColorWhiteHighlight);
				break;
			case HELP_VIDEO:
				mars_drawTextwShadow("VIDEO OPTIONS", 125, 35, fontColorGreen, fontColorGreenHighlight);
				mars_drawTextwShadow("Here you can enable interlaced", 32, 57, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("(480i) mode for the whole suite", 32, 65, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("and disable horizontal 256 in", 32, 73, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("compatible screens.", 32, 81, fontColorWhite, fontColorWhiteHighlight);

				mars_drawTextwShadow("For PAL consoles, you can also", 32, 98, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("enable full 240p, instead of", 32, 106, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("224p.", 32, 114, fontColorWhite, fontColorWhiteHighlight);
				break;
			case HELP_AUDIOSYNC:
				mars_drawTextwShadow("AUDIO SYNC", 125, 35, fontColorGreen, fontColorGreenHighlight);
				mars_drawTextwShadow("This test flashes the whole", 32, 57, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("screen white for 2 frames,", 32, 65, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("along with a 1khz test tone.", 32, 73, fontColorWhite, fontColorWhiteHighlight);

				mars_drawTextwShadow("You can verify the sync between", 32, 90, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("audio and video with recording", 32, 98, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("equipment or specialized", 32, 106, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("hardware.", 32, 114, fontColorWhite, fontColorWhiteHighlight);

				mars_drawTextwShadow("Press A to start/stop the test.", 32, 131, fontColorWhite, fontColorWhiteHighlight);
				break;
			case HELP_CONTRAST:
				mars_drawTextwShadow("PLUGE CONTRAST", 125, 35, fontColorGreen, fontColorGreenHighlight);
				mars_drawTextwShadow("This sub test fills the screen", 32, 57, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("with a pattern. By default it", 32, 65, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("is drawn in color.", 32, 73, fontColorWhite, fontColorWhiteHighlight);

				mars_drawTextwShadow("You can change palettes with", 32, 90, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("A to display a dark or white", 32, 98, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("pattern in order to check", 32, 106, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("contrast and brightness.", 32, 114, fontColorWhite, fontColorWhiteHighlight);

				mars_drawTextwShadow("The C button returns to PLUGE.", 32, 131, fontColorWhite, fontColorWhiteHighlight);
				break;
			case HELP_MEMVIEW:
				mars_drawTextwShadow("MEMORY VIEWER", 110, 35, fontColorGreen, fontColorGreenHighlight);
				mars_drawTextwShadow("This shows selected regions of", 32, 56, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("the main CPU memory map.", 32, 64, fontColorWhite, fontColorWhiteHighlight);

				mars_drawTextwShadow("The current address range is", 32, 80, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("shown in red at the right from", 32, 88, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("top to bottom.", 32, 96, fontColorWhite, fontColorWhiteHighlight);

				mars_drawTextwShadow("The left part of the screen", 32, 112, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("shows 0x1C0 bytes in hex.", 32, 120, fontColorWhite, fontColorWhiteHighlight);

				mars_drawTextwShadow("- Button A enabled CRC of the", 32, 136, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("current screen.", 32, 144, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("- Button B jumps to relevant", 32, 152, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("memory locations.", 32, 160, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("- Button C switches to ASCII", 32, 168, fontColorWhite, fontColorWhiteHighlight);
				mars_drawTextwShadow("mode.", 32, 176, fontColorWhite, fontColorWhiteHighlight);
				break;
		default:
			screenFadeOut(1);
			exit = 1;
			break;
	}
		if (pressedButton & SEGA_CTRL_RIGHT)
		{
			if(page + 1 <= totalpages)
			{
				page++;
			}
		}

		if (pressedButton & SEGA_CTRL_LEFT)
		{
			if(page - 1 > 0)
			{
				page--;
			}
		}

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_THREE)
		{
			if (pressedButton & SEGA_CTRL_C)
			{
				screenFadeOut(1);
				exit = 1;
			}
		}

		if (pressedButton & SEGA_CTRL_Z)
		{
			screenFadeOut(1);
			exit = 1;
		}

		if (pressedButton & SEGA_CTRL_START)
		{
			screenFadeOut(1);
			exit = 1;
		}

		if (pressedButton & SEGA_CTRL_B)
		{
			screenFadeOut(1);
			exit = 1;
		}

		Hw32xScreenFlip(0);
	}
}