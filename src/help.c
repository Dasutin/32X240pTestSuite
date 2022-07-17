/* 
 * 240p Test Suite for the Sega 32X
 * Port by Dasutin
 * Copyright (C)2011-2022 Artemio Urbina
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

extern int fontColorWhite;
extern int fontColorRed;
extern int fontColorGreen;
extern int fontColorGray;
extern int fontColorBlack;

void DrawHelp(int option)
{
	int frameDelay = 5;
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
	case HELP_HSCROLL:
	case HELP_MEMVIEW:
		totalpages = 2;
		break;
	case HELP_PLUGE:
	case HELP_MANUALLAG:
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
					mars_drawTextwShadow("HELP (1/2)", 125, 35, fontColorGreen, fontColorGray);

					mars_drawTextwShadow("The 240p Test Suite was designed", 35, 57, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("with two goals in mind:", 35, 65, fontColorWhite, fontColorGray);

					mars_drawTextwShadow("1) Evaluate 240p signals on TV", 35, 81, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("sets and video processors; and", 35, 90, fontColorWhite, fontColorGray);

					mars_drawTextwShadow("2) Provide calibration patterns", 35, 106, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("from a game console to help in", 35, 114, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("properly calibrating the display", 35, 122, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("black, white, and color levels.", 35, 130, fontColorWhite, fontColorGray);

					mars_drawTextwShadow("Help can be shown with 'Z' in", 35, 146, fontColorGreen, fontColorGray);
					mars_drawTextwShadow("any test. Video Options can be", 35, 154, fontColorGreen, fontColorGray);
					mars_drawTextwShadow("changed with 'Y'.", 35, 162, fontColorGreen, fontColorGray);

					mars_drawTextwShadow("(cont...)", 207, 178, fontColorWhite, fontColorGray);

					mars_drawTextwShadow("Press Z to exit help", 91, 193, fontColorWhite, fontColorGray);
					break;
				case 2:
					mars_drawTextwShadow("HELP (2/2)", 125, 35, fontColorGreen, fontColorGray);
                                                              //|
					mars_drawTextwShadow("The Sega 32X can output 224", 35, 50, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("active video lines. In PAL", 35, 58, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("consoles, it can display either", 35, 66, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("224 or 240 lines.", 35, 74, fontColorWhite, fontColorGray);

					mars_drawTextwShadow("The 240p suite is also available", 35, 90, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("on NES/FC, SNES/SFC, GameCube,", 35, 98, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("Wii, GBA, Neo Geo MVS/AES and CD,", 35, 106, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("Dreamcast, Genesis/Mega Drive,", 35, 114, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("Sega/Mega CD, SMS, and", 35, 122, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("PCE/TG-16/PCE-Duo/SCD/SCD2", 35, 130, fontColorWhite, fontColorGray);

					mars_drawTextwShadow("Visit:", 35, 150, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("http://junkerhq.net/240p", 35, 160, fontColorGreen, fontColorGray);
					mars_drawTextwShadow("for more information", 35, 170, fontColorWhite, fontColorGray);

					mars_drawTextwShadow("Press Z to exit help", 91, 193, fontColorWhite, fontColorGray);
					break;
				}
				break;
			case HELP_PLUGE:
				switch (page)
				{
				case 1:
					mars_drawTextwShadow("PLUGE (1/3)", 115, 35, fontColorGreen, fontColorGray);

					mars_drawTextwShadow("NTSC levels require black to be", 35, 57, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("at 7.5 IRE for video. This HW", 35, 65, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("lowest is 6 IRE (6%), so using", 35, 73, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("this value for general 240p use", 35, 81, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("is not recommended.", 35, 89, fontColorWhite, fontColorGray);

					mars_drawTextwShadow("Of course using it as reference", 35, 106, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("will work perfectly for games", 35, 114, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("in this platform.", 35, 122, fontColorWhite, fontColorGray);

					mars_drawTextwShadow("In PAL - and console gaming in", 35, 139, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("general - it is adviced to use", 35, 147, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("a value of 2 IRE as black.", 35, 155, fontColorWhite, fontColorGray);

					mars_drawTextwShadow("(cont...)", 207, 178, fontColorWhite, fontColorGray);
					break;
				case 2:
					mars_drawTextwShadow("PLUGE (2/3)", 115, 35, fontColorGreen, fontColorGray);

					mars_drawTextwShadow("The PLUGE pattern is used to", 35, 57, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("help adjust the black level to", 35, 65, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("a correct value.", 35, 73, fontColorWhite, fontColorGray);

					mars_drawTextwShadow("The inner bars on the sides are", 35, 90, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("black at 6%, the outer at 12%.", 35, 98, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("If these bars are not visible,", 35, 106, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("adjust the \"brightness\" control", 35, 114, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("until they are.", 35, 122, fontColorWhite, fontColorGray);

					mars_drawTextwShadow("You should lower it until they", 35, 139, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("are not visible, and raise it", 35, 147, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("until they show.", 35, 155, fontColorWhite, fontColorGray);

					mars_drawTextwShadow("(cont...)", 207, 178, fontColorWhite, fontColorGray);
					break;
				case 3:
					mars_drawTextwShadow("PLUGE (3/3)", 115, 35, fontColorGreen, fontColorGray);

					mars_drawTextwShadow("You can change to a contrast", 35, 57, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("test with C.", 35, 65, fontColorWhite, fontColorGray);

					mars_drawTextwShadow("Within it A button changes", 35, 82, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("palettes between the original,", 35, 90, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("highest and lowest values the", 35, 98, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("hardware can display.", 35, 106, fontColorWhite, fontColorGray);
					break;
				}
				break;
			case HELP_COLORS:
				mars_drawTextwShadow("COLORBARS", 125, 35, fontColorGreen, fontColorGray);

				mars_drawTextwShadow("This pattern allows you to", 35, 57, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("calibrate each color: Red, Green", 35, 65, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("and Blue; as well as white.", 35, 73, fontColorWhite, fontColorGray);

				mars_drawTextwShadow("Adjust the white level first,", 35, 90, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("using the \"Contrast\" control", 35, 98, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("on your TV set. Raise it until", 35, 106, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("you cannot distinguish between", 35, 114, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("the blocks under \"C\" and \"E\",", 35, 122, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("and lower it slowly until you", 35, 130, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("can clearly tell them appart.", 35, 138, fontColorWhite, fontColorGray);

				mars_drawTextwShadow("Do the same for each color.", 35, 155, fontColorWhite, fontColorGray);
				break;
			case HELP_GRID:
				mars_drawTextwShadow("GRID", 140, 35, fontColorGreen, fontColorGray);

				mars_drawTextwShadow("This grid uses the full 320x224", 35, 57, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("resolution.", 35, 65, fontColorWhite, fontColorGray);

				mars_drawTextwShadow("You can use it to verify that", 35, 82, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("all the visible area is being", 35, 90, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("displayed, and that there is no", 35, 98, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("distortion present.", 35, 106, fontColorWhite, fontColorGray);

				mars_drawTextwShadow("The full active video signal can", 35, 123, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("be filled with gray by pressing", 35, 131, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("the 'A' button.", 35, 139, fontColorWhite, fontColorGray);
				break;
			case HELP_MONOSCOPE:
				switch (page)
				{
					case 1:
						mars_drawTextwShadow("MONOSCOPE (1/3)", 95, 35, fontColorGreen, fontColorGray);

						mars_drawTextwShadow("This pattern contains elements", 35, 57, fontColorWhite, fontColorGray);
						mars_drawTextwShadow("to calibrate multiple aspects", 35, 65, fontColorWhite, fontColorGray);
						mars_drawTextwShadow("of a CRT.", 35, 73, fontColorWhite, fontColorGray);

						mars_drawTextwShadow("Read your monitor's service", 35, 90, fontColorWhite, fontColorGray);
						mars_drawTextwShadow("manual to learn how, and use", 35, 98, fontColorWhite, fontColorGray);
						mars_drawTextwShadow("'A' button to change IRE.", 35, 106, fontColorWhite, fontColorGray);

						mars_drawTextwShadow("Brightness adjustment: Adjust", 35, 123, fontColorWhite, fontColorGray);
						mars_drawTextwShadow("convergence at low brightness", 35, 131, fontColorWhite, fontColorGray);
						mars_drawTextwShadow("(13/25 IRE). An overly bright", 35, 139, fontColorWhite, fontColorGray);
						mars_drawTextwShadow("pattern can mask convergence.", 35, 147, fontColorWhite, fontColorGray);
						mars_drawTextwShadow("issues.", 35, 155, fontColorWhite, fontColorGray);

						mars_drawTextwShadow("(cont...)", 207, 178, fontColorWhite, fontColorGray);
					break;
					case 2:
						mars_drawTextwShadow("MONOSCOPE (2/3)", 95, 35, fontColorGreen, fontColorGray);

						mars_drawTextwShadow("Convergence: Use the center", 35, 57, fontColorWhite, fontColorGray);
						mars_drawTextwShadow("crosshair to check static", 35, 65, fontColorWhite, fontColorGray);
						mars_drawTextwShadow("(center of screen) convergence.", 35, 73, fontColorWhite, fontColorGray);
						mars_drawTextwShadow("Use the patterns at the sides", 35, 81, fontColorWhite, fontColorGray);
						mars_drawTextwShadow("to check dynamic (edge)", 35, 89, fontColorWhite, fontColorGray);
						mars_drawTextwShadow("convergence.", 35, 97, fontColorWhite, fontColorGray);

						mars_drawTextwShadow("Corners: After setting center", 35, 114, fontColorWhite, fontColorGray);
						mars_drawTextwShadow("and edge convergence, use", 35, 122, fontColorWhite, fontColorGray);
						mars_drawTextwShadow("magnets to adjust corner purity", 35, 130, fontColorWhite, fontColorGray);
						mars_drawTextwShadow("and geometry.", 35, 138, fontColorWhite, fontColorGray);

						mars_drawTextwShadow("(cont...)", 207, 178, fontColorWhite, fontColorGray);
					break;
					case 3:
						mars_drawTextwShadow("MONOSCOPE (3/3)", 95, 35, fontColorGreen, fontColorGray);

						mars_drawTextwShadow("Size and aspect ratio: If", 35, 57, fontColorWhite, fontColorGray);
						mars_drawTextwShadow("vertical and horizontal size are", 35, 65, fontColorWhite, fontColorGray);
						mars_drawTextwShadow("correct, the red squares in the", 35, 73, fontColorWhite, fontColorGray);
						mars_drawTextwShadow("pattern will be perfect squares.", 35, 81, fontColorWhite, fontColorGray);
						mars_drawTextwShadow("After setting H size, use a tape", 35, 89, fontColorWhite, fontColorGray);
						mars_drawTextwShadow("measure to adjust V size to", 35, 97, fontColorWhite, fontColorGray);
						mars_drawTextwShadow("match it.", 35, 105, fontColorWhite, fontColorGray);

						mars_drawTextwShadow("Linearity: The squares in each", 35, 122, fontColorWhite, fontColorGray);
						mars_drawTextwShadow("corner should get you started.", 35, 130, fontColorWhite, fontColorGray);
						mars_drawTextwShadow("Confirm your adjustment using", 35, 138, fontColorWhite, fontColorGray);
						mars_drawTextwShadow("the scroll tests.", 35, 146, fontColorWhite, fontColorGray);

						mars_drawTextwShadow("Designed by Keith Raney.", 35, 164, fontColorWhite, fontColorGray);
					break;
				}
				break;				
			case HELP_BLEED:
				mars_drawTextwShadow("COLOR BLEED", 123, 35, fontColorGreen, fontColorGray);
				mars_drawTextwShadow("This pattern helps diagnose", 35, 57, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("color bleed caused by", 35, 65, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("unneeded color upsampling.", 35, 73, fontColorWhite, fontColorGray);

				mars_drawTextwShadow("You can toggle between", 35, 90, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("vertical bars and", 35, 98, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("checkerboard with 'A'.", 35, 106, fontColorWhite, fontColorGray);
				break;
			case HELP_IRE:
				mars_drawTextwShadow("100 IRE", 125, 35, fontColorGreen, fontColorGray);
				mars_drawTextwShadow("You can vary IRE intensity", 35, 57, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("with A and B. Values are:", 35, 65, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("13, 25, 41, 53, 66, 82, 94", 35, 73, fontColorWhite, fontColorGray);
				break;
			case HELP_601CB:
				mars_drawTextwShadow("601 COLORBARS", 120, 35, fontColorGreen, fontColorGray);

				mars_drawTextwShadow("You can use color filters or the", 35, 57, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("blue only option in your display", 35, 65, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("-if available- to confirm color", 35, 73, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("balance.", 35, 81, fontColorWhite, fontColorGray);
				break;
			case HELP_SHARPNESS:
				mars_drawTextwShadow("SHARPNESS", 125, 35, fontColorGreen, fontColorGray);
				mars_drawTextwShadow("You should set the sharpness of", 35, 57, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("your CRT to a value that shows", 35, 65, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("clean black and gray transitions", 35, 73, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("with no white ghosting between.", 35, 81, fontColorWhite, fontColorGray);

				mars_drawTextwShadow("On most digitally controlled", 35, 98, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("displays, sharpness is an edge-", 35, 106, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("enhancement control and most", 35, 114, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("probably should be set to zero.", 35, 122, fontColorWhite, fontColorGray);

				mars_drawTextwShadow("Pressing B toggles a diagonal", 35, 139, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("brick tileset.", 35, 147, fontColorWhite, fontColorGray);
				break;
			case HELP_OVERSCAN:
				mars_drawTextwShadow("OVERSCAN", 125, 35, fontColorGreen, fontColorGray);
				mars_drawTextwShadow("With this pattern you can", 35, 57, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("interactively find out the", 35, 65, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("overscan in pixels of each edge", 35, 73, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("in a display.", 35, 81, fontColorWhite, fontColorGray);

				mars_drawTextwShadow("Use left and right to increment", 35, 98, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("the overscan until you see the", 35, 106, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("white border, then go back one", 35, 114, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("pixel. The resulting number is", 35, 122, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("the amount of overscan in pixels", 35, 130, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("in each direction.", 35, 138, fontColorWhite, fontColorGray);
				break;
			case HELP_SMPTE:
				mars_drawTextwShadow("SMPTE COLOR BARS", 90, 35, fontColorGreen, fontColorGray);

				mars_drawTextwShadow("This pattern can be used to", 35, 57, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("calibrate for NTSC levels", 35, 65, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("regarding contrast,", 35, 73, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("brightness and colors.", 35, 81, fontColorWhite, fontColorGray);

				mars_drawTextwShadow("You can toggle between 75% and", 35, 98, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("100% SMPTE color bars with A.", 35, 106, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("Of course the percentages are", 35, 114, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("relative to the console output.", 35, 122, fontColorWhite, fontColorGray);

				mars_drawTextwShadow("You can use color filters or the", 35, 139, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("blue only option in your display", 35, 147, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("-if available- to confirm color", 35, 155, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("balance.", 35, 163, fontColorWhite, fontColorGray);

				mars_drawTextwShadow("This HW lowest black is 6%.", 35, 180, fontColorWhite, fontColorGray);
				break;
			case HELP_GRAY:
				mars_drawTextwShadow("GRAY RAMP", 125, 35, fontColorGreen, fontColorGray);
				mars_drawTextwShadow("This gray ramp pattern can be", 35, 57, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("used to check color balance.", 35, 65, fontColorWhite, fontColorGray);

				mars_drawTextwShadow("You should make sure the bars", 35, 82, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("are gray, with no red or blue", 35, 90, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("bias. This can be adjusted with", 35, 98, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("the individual color settings,", 35, 106, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("or the tint control in NTSC", 35, 114, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("displays.", 35, 122, fontColorWhite, fontColorGray);
				break;
			case HELP_WHITE:
				mars_drawTextwShadow("WHITE SCREEN", 115, 35, fontColorGreen, fontColorGray);
				mars_drawTextwShadow("This pattern can be changed", 35, 57, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("between white, black, red,", 35, 65, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("green and blue screens with the", 35, 73, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("'A' and 'B' buttons.", 35, 81, fontColorWhite, fontColorGray);

				mars_drawTextwShadow("Some displays and scalers have", 35, 98, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("issues when changing between a", 35, 106, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("black 0 IRE and a white screen.", 35, 114, fontColorWhite, fontColorGray);

				mars_drawTextwShadow("A custom color mode is", 35, 122, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("available by pressing 'C'.", 35, 130, fontColorWhite, fontColorGray);
				break;
			case HELP_CONVERGENCE:
				mars_drawTextwShadow("CONVERGENCE TESTS", 90, 35, fontColorGreen, fontColorGray);
				mars_drawTextwShadow("These are used to adjust color", 35, 57, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("convergence in CRT displays", 35, 65, fontColorWhite, fontColorGray);

				mars_drawTextwShadow("The 'A' button changes the cross", 35, 82, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("hatch pattern between lines,", 35, 90, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("dots and crosses", 35, 98, fontColorWhite, fontColorGray);

				mars_drawTextwShadow("Resolution can be changed by", 35, 115, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("pressing 'c'", 35, 123, fontColorWhite, fontColorGray);

				mars_drawTextwShadow("The 'B' button changes to a", 35, 140, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("color pattern for transition", 35, 148, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("boundary check.", 35, 156, fontColorWhite, fontColorGray);

				mars_drawTextwShadow("In color modes, 'A' toggles", 35, 173, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("black borders.", 35, 181, fontColorWhite, fontColorGray);
				break;	
			case HELP_CHECK:
				mars_drawTextwShadow("CHECKERBOARD", 115, 35, fontColorGreen, fontColorGray);
				mars_drawTextwShadow("This pattern shows all the", 35, 57, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("visible pixels in an", 35, 65, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("alternating white and black", 35, 73, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("grid array.", 35, 81, fontColorWhite, fontColorGray);

				mars_drawTextwShadow("You can toggle the pattern with", 35, 98, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("button 'Up', or turn on", 35, 106, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("auto-toggle each frame with the", 35, 114, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("'A' button. A frame counter is", 35, 122, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("also available with 'B'.", 35, 130, fontColorWhite, fontColorGray);
				break;
			case HELP_STRIPES:
				switch (page)
				{
				case 1:
					mars_drawTextwShadow("HORIZONTAL STRIPES (1/2)", 75, 35, fontColorGreen, fontColorGray);
					mars_drawTextwShadow("You should see a pattern of", 35, 57, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("lines, each one pixel in height", 35, 65, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("starting with a white one.", 35, 73, fontColorWhite, fontColorGray);

					mars_drawTextwShadow("You can toggle the pattern with", 35, 90, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("button 'UP', or turn on", 35, 98, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("auto-toggle each frame with the", 35, 106, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("'A' button. A frame counter is", 35, 114, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("also available with 'B'.", 35, 122, fontColorWhite, fontColorGray);

					mars_drawTextwShadow("When auto-toggle is set, you", 35, 139, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("should see the lines", 35, 147, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("alternating rapidly. On some", 35, 155, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("setups, the pattern doesn't", 35, 163, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("change at all. This means that", 35, 171, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("(cont...)", 207, 178, fontColorWhite, fontColorGray);
					break;
				case 2:
					mars_drawTextwShadow("HORIZONTAL STRIPES (2/2)", 75, 35, fontColorGreen, fontColorGray);
					mars_drawTextwShadow("the signal is being treated as", 35, 57, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("480i/576i and odd or even frames", 35, 65, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("are being discarded completely.", 35, 73, fontColorWhite, fontColorGray);

					mars_drawTextwShadow("You can also display vertical", 35, 90, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("bars by pressing 'LEFT'. That", 35, 98, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("pattern will help you", 35, 106, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("evaluate if the signal is not", 35, 114, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("distorted horizontaly, since", 35, 122, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("all lines should be one pixel", 35, 130, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("wide.", 35, 138, fontColorWhite, fontColorGray);

					break;
				}
				break;
			case HELP_SHADOW:
				switch (page)
				{
				case 1:
					mars_drawTextwShadow("DROP SHADOW TEST (1/2)", 75, 35, fontColorGreen, fontColorGray);
					mars_drawTextwShadow("This is a crucial test for 240p", 35, 57, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("upscan converters. It displays a", 35, 65, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("simple sprite shadow against a", 35, 73, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("background, but the shadow is", 35, 81, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("shown only on each other frame.", 35, 89, fontColorWhite, fontColorGray);

					mars_drawTextwShadow("On a CRT this achieves a", 35, 106, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("transparency effect, since you", 35, 114, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("are watching a 30hz (25Hz)", 35, 122, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("shadow on a 60hz (50hz) signal.", 35, 130, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("No background detail should be", 35, 138, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("lost and the shadow should be", 35, 146, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("visible.", 35, 154, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("(cont...)", 207, 178, fontColorWhite, fontColorGray);
					break;
				case 2:
					mars_drawTextwShadow("DROP SHADOW TEST (2/2)", 80, 35, fontColorGreen, fontColorGray);
					mars_drawTextwShadow("The user can toggle the frame", 35, 57, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("used to draw the shadow with", 35, 65, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("button 'A'. Backgrounds can be", 35, 73, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("switched with the 'B' button and", 35, 81, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("button 'C' toggles sprites.", 35, 89, fontColorWhite, fontColorGray);
					break;
				}
				break;
			case HELP_STRIPED:
				mars_drawTextwShadow("STRIPED SPRITE TEST", 84, 35, fontColorGreen, fontColorGray);
				mars_drawTextwShadow("There are deinterlacers out", 35, 57, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("there that can display the drop", 35, 65, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("shadows correctly and still", 35, 73, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("interpret 240p as 480i. With a", 35, 81, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("striped sprite it should be easy", 35, 89, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("to tell if a processor tries to", 35, 97, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("deinterlace (plus interpolate).", 35, 105, fontColorWhite, fontColorGray);

				mars_drawTextwShadow("You can change backgrounds with", 35, 122, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("'A'.", 35, 130, fontColorWhite, fontColorGray);
				break;
			case HELP_MANUALLAG:
				switch (page)
				{
				case 1:
					mars_drawTextwShadow("MANUAL LAG TEST (1/3)", 75, 35, fontColorGreen, fontColorGray);
					mars_drawTextwShadow("The main intention is to show a", 35, 57, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("changing pattern on the screen,", 35, 65, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("which can be complemented with", 35, 73, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("audio. This should show to some", 35, 81, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("degree any lag when processing", 35, 89, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("the signal.", 35, 97, fontColorWhite, fontColorGray);

					mars_drawTextwShadow("As an added feature, the user", 35, 114, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("can click the 'A' button when", 35, 122, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("the sprite is aligned with the", 35, 130, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("one on the background, and the", 35, 138, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("offset in frames from the actual", 35, 146, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("intersection will be shown on", 35, 154, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("screen. A 1khz tone will be", 35, 162, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("played for 1 frame when pressed.", 35, 170, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("(cont...)", 207, 178, fontColorWhite, fontColorGray);
					break;
				case 2:
					mars_drawTextwShadow("MANUAL LAG TEST (2/3)", 75, 35, fontColorGreen, fontColorGray);
					mars_drawTextwShadow("This can be repeated ten", 35, 57, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("times and the software will", 35, 65, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("calculate the average. Whenever", 35, 73, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("the button was pressed before", 35, 81, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("the actual intersection frame,", 35, 89, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("the result will be ignored, but", 35, 97, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("still shown onscreen. Button 'B'", 35, 105, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("can be used to change the", 35, 113, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("direction of the sprite from", 35, 121, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("vertical to horizontal, or both", 35, 129, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("at the same time.", 35, 137, fontColorWhite, fontColorGray);

					mars_drawTextwShadow("Of course the evaluation is", 35, 154, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("dependent on reflexes and/or", 35, 162, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("rhythm more than anything. The", 35, 170, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("(cont...)", 207, 178, fontColorWhite, fontColorGray);
					break;
				case 3:
					mars_drawTextwShadow("MANUAL LAG TEST (3/3)", 75, 35, fontColorGreen, fontColorGray);
					mars_drawTextwShadow("visual and audio cues are the", 35, 57, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("more revealing aspects which", 35, 65, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("the user should consider, of", 35, 73, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("course the interactive factor", 35, 81, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("can give an experienced player", 35, 89, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("the hang of the system when", 35, 97, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("testing via different", 35, 105, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("connections.", 35, 113, fontColorWhite, fontColorGray);
					break;
				}
				break;
			case HELP_HSCROLL:
				switch (page)
				{
				case 1:
					mars_drawTextwShadow("SCROLL TEST (1/2)", 115, 35, fontColorGreen, fontColorGray);
					mars_drawTextwShadow("This test shows either an", 35, 57, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("horizontal 320x224 back from", 35, 65, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("Sonic or a vertical 256x224", 35, 73, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("back from Kiki Kaikai.", 35, 81, fontColorWhite, fontColorGray);

					mars_drawTextwShadow("Speed can be varied with Up &", 35, 98, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("Down and scroll direction with", 35, 106, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("Left. The 'A' button stops the", 35, 114, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("scroll and 'B' toggles between", 35, 122, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("vertical and horizontal.", 35, 130, fontColorWhite, fontColorGray);

					mars_drawTextwShadow("The vertical 256 scroll is also", 35, 147, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("used while in 320 mode but it", 35, 155, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("can be positioned with 'Right'.", 35, 163, fontColorWhite, fontColorGray);
					
					mars_drawTextwShadow("(cont...)", 207, 178, fontColorWhite, fontColorGray);
					break;					
				case 2:
					mars_drawTextwShadow("SCROLL TEST (2/2)", 115, 35, fontColorGreen, fontColorGray);
					mars_drawTextwShadow("This can be used to notice any", 35, 57, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("drops in framerate, or pixel", 35, 65, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("width inconsistencies.", 35, 73, fontColorWhite, fontColorGray);

					mars_drawTextwShadow("Sonic is a trademark of Sega", 35, 90, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("Enterprises Ltd. Kiki Kaikai", 35, 98, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("is a trademark of Taito.", 35, 106, fontColorWhite, fontColorGray);
					break;
				}
				break;
			case HELP_VSCROLL:
				mars_drawTextwShadow("GRID SCROLL TEST", 125, 35, fontColorGreen, fontColorGray);
				mars_drawTextwShadow("A grid is scrolled vertically or", 35, 57, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("horizontally, which can be used", 35, 65, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("to test linearity of the signal", 35, 73, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("and how well the display or", 35, 81, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("video processor copes with", 35, 89, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("scrolling and framerate.", 35, 97, fontColorWhite, fontColorGray);

				mars_drawTextwShadow("Button 'B' can be used to toggle", 35, 114, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("between horizontal and vertical,", 35, 122, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("while Up/Down regulate speed.", 35, 130, fontColorWhite, fontColorGray);

				mars_drawTextwShadow("Button 'A' stops the scroll and", 35, 147, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("'Left' changes direction.", 35, 155, fontColorWhite, fontColorGray);
				break;
		case HELP_LED:
				switch (page)
				{
				case 1:
					mars_drawTextwShadow("BACKLIT TEST (1/2)", 85, 35, fontColorGreen, fontColorGray);
					mars_drawTextwShadow("This test allows you to check", 35, 57, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("how the display's backlit works", 35, 65, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("when only a small array of", 35, 73, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("pixels is shown. This can be", 35, 81, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("very revealing and can be used", 35, 89, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("to detect the zones in LED", 35, 97, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("backlit displays, or to evaluate", 35, 105, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("global and local dimming.", 35, 113, fontColorWhite, fontColorGray);

					mars_drawTextwShadow("The user can move around the", 35, 130, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("white pixel arrays with the", 35, 138, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("d-pad, and change the size of", 35, 146, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("the pixel array with 'A'.", 35, 154, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("The 'B' button allows the user", 35, 162, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("to hide the pixel array in", 35, 170, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("(cont...)", 207, 178, fontColorWhite, fontColorGray);
					break;
				case 2:
					mars_drawTextwShadow("BACKLIT TEST (2/2)", 85, 35, fontColorGreen, fontColorGray);
					mars_drawTextwShadow("order to alternate a fully black", 35, 57, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("screen.", 35, 65, fontColorWhite, fontColorGray);

					mars_drawTextwShadow("It might be necessary to have a", 35, 82, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("fully-off panel (if possible)", 35, 90, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("when displaying the black", 35, 98, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("background,  in order to", 35, 106, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("evaluate full-backlit displays", 35, 114, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("by adjusting brightness", 35, 122, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("accordingly for this test.", 35, 130, fontColorWhite, fontColorGray);
					break;
				}
				break;
			case HELP_LAG:
				switch (page)
				{
				case 1:
					mars_drawTextwShadow("LAG TEST (1/2)", 125, 35, fontColorGreen, fontColorGray);
					mars_drawTextwShadow("This test is designed to be used", 35, 57, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("with two displays conected at", 35, 65, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("the same time. One being a CRT,", 35, 73, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("or a display with a known lag as", 35, 81, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("reference, and the other the", 35, 89, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("display to test.", 35, 97, fontColorWhite, fontColorGray);

					mars_drawTextwShadow("Using a camera, a picture should", 35, 114, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("be taken of both screens at the", 35, 122, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("same time. The picture will show", 35, 130, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("the frame discrepancy between", 35, 138, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("them.", 35, 146, fontColorWhite, fontColorGray);

					mars_drawTextwShadow("The circles in the bottom help", 35, 163, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("determine the frame even when", 35, 171, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("(cont...)", 207, 178, fontColorWhite, fontColorGray);
					break;
				case 2:
					mars_drawTextwShadow("LAG TEST (2/2)", 125, 35, fontColorGreen, fontColorGray);
					mars_drawTextwShadow("the numbers are blurry.", 35, 57, fontColorWhite, fontColorGray);

					mars_drawTextwShadow("This version of the suite can be", 35, 74, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("used with a Sega Nomad as the", 35, 82, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("reference display.", 35, 90, fontColorWhite, fontColorGray);

					mars_drawTextwShadow("You can also split the video", 35, 107, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("signal and feed both displays.", 35, 115, fontColorWhite, fontColorGray);

					mars_drawTextwShadow("The vertical bars on the sides", 35, 132, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("change color each frame to help", 35, 140, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("when using LCD photos.", 35, 148, fontColorWhite, fontColorGray);

					mars_drawTextwShadow("Press A to start/stop, B to", 35, 165, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("reset and C for B&W test.", 35, 173, fontColorWhite, fontColorGray);
					break;
				}
				break;
			case HELP_ALTERNATE:
				mars_drawTextwShadow("ALTERNATE 240p/480i", 125, 35, fontColorGreen, fontColorGray);
				mars_drawTextwShadow("Some devices have a delay when", 35, 57, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("the source changes between", 35, 65, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("resolutions, which happens in", 35, 73, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("some games. This test allows to", 35, 81, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("time that delay manually.", 35, 89, fontColorWhite, fontColorGray);

				mars_drawTextwShadow("Press A to switch the", 35, 106, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("resolution; press it again when", 35, 114, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("you are able to see the screen", 35, 122, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("back in your display.", 35, 130, fontColorWhite, fontColorGray);
				break;
			case HELP_VIDEO:
				mars_drawTextwShadow("VIDEO OPTIONS", 125, 35, fontColorGreen, fontColorGray);
				mars_drawTextwShadow("Here you can enable interlaced", 35, 57, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("(480i) mode for the whole suite", 35, 65, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("and disable horizontal 256 in", 35, 73, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("compatible screens.", 35, 81, fontColorWhite, fontColorGray);

				mars_drawTextwShadow("For PAL consoles, you can also", 35, 98, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("enable full 240p, instead of", 35, 106, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("224p.", 35, 114, fontColorWhite, fontColorGray);
				break;
			case HELP_AUDIOSYNC:
				mars_drawTextwShadow("AUDIO SYNC", 125, 35, fontColorGreen, fontColorGray);
				mars_drawTextwShadow("This test flashes the whole", 35, 57, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("screen white for 2 frames,", 35, 65, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("along with a 1khz test tone.", 35, 73, fontColorWhite, fontColorGray);

				mars_drawTextwShadow("You can verify the sync between", 35, 90, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("audio and video with recording", 35, 98, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("equipment or specialized", 35, 106, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("hardware.", 35, 114, fontColorWhite, fontColorGray);

				mars_drawTextwShadow("Press A to start/stop the test.", 35, 131, fontColorWhite, fontColorGray);
				break;
			case HELP_CONTRAST:
				mars_drawTextwShadow("PLUGE CONTRAST", 125, 35, fontColorGreen, fontColorGray);
				mars_drawTextwShadow("This sub test fills the screen", 35, 57, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("with a pattern. By default it", 35, 65, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("is drawn in color.", 35, 73, fontColorWhite, fontColorGray);

				mars_drawTextwShadow("You can change palettes with", 35, 90, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("A to display a dark or white", 35, 98, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("pattern in order to check", 35, 106, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("contrast and brightness.", 35, 114, fontColorWhite, fontColorGray);

				mars_drawTextwShadow("The C button returns to PLUGE.", 35, 131, fontColorWhite, fontColorGray);
				break;
			case HELP_MEMVIEW:
				switch (page)
				{
				case 1:
					mars_drawTextwShadow("MEMORY VIEWER", 125, 35, fontColorGreen, fontColorGray);
					mars_drawTextwShadow("This shows selected regions of", 35, 57, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("the main CPU memory map.", 35, 65, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("The current address range is", 35, 73, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("shown in red at the right from", 35, 81, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("top to bottom.", 35, 89, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("The left part of the screen", 35, 97, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("shows 0x1C0 bytes in hex.", 35, 105, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("- Button A enabled CRC of of", 35, 113, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("current screen.", 35, 121, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("- Button B jumps to relevant", 35, 129, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("memory locations", 35, 137, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("(cont...)", 207, 178, fontColorWhite, fontColorGray);
					break;
				case 2:
					mars_drawTextwShadow("- Button C switches banks in", 35, 57, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("SCD Program RAM.", 35, 65, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("With a working SCD system", 35, 73, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("Program RAM banks should show", 35, 81, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("0xAA for bank 1 (0xBB for bank", 35, 89, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("2 etc.) in all four corners.", 35, 97, fontColorWhite, fontColorGray);
					break;
				}
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

		Hw32xDelay(frameDelay);
	}
}