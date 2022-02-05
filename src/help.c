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
	u16 button, pressedButton, oldButton = 0xFFFF;
	u16 exit = 0;
	u16 page = 1;
	u16 totalpages = 1;

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
					mars_drawTextwShadow("HELP (1/2)", 60, 35, fontColorGreen, fontColorGray);

					mars_drawTextwShadow("The 240p Test Suite was designed", -30, 57, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("with two goals in mind:", -30, 65, fontColorWhite, fontColorGray);

					mars_drawTextwShadow("1) Evaluate 240p signals on TV", -30, 81, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("sets and video processors; and", -30, 90, fontColorWhite, fontColorGray);

					mars_drawTextwShadow("2) Provide calibration patterns", -30, 106, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("from a game console to help in", -30, 114, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("properly calibrating the display", -30, 122, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("black, white, and color levels.", -30, 130, fontColorWhite, fontColorGray);

					mars_drawTextwShadow("Help can be shown with 'Z' in", -30, 146, fontColorGreen, fontColorGray);
					mars_drawTextwShadow("any test. Video Options can be", -30, 154, fontColorGreen, fontColorGray);
					mars_drawTextwShadow("changed with 'Y'.", -30, 162, fontColorGreen, fontColorGray);

					mars_drawTextwShadow("(cont...)", 142, 178, fontColorWhite, fontColorGray);

					mars_drawTextwShadow("Press Z to exit help", 26, 193, fontColorWhite, fontColorGray);
					break;
				case 2:
					mars_drawTextwShadow("HELP (2/2)", 60, 35, fontColorGreen, fontColorGray);
                                                              //|
					mars_drawTextwShadow("The Sega 32X can output 224", -30, 50, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("active video lines. In PAL", -30, 58, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("consoles, it can display either", -30, 66, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("224 or 240 lines.", -30, 74, fontColorWhite, fontColorGray);

					mars_drawTextwShadow("The 240p suite is also available", -30, 90, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("on NES/FC, SNES/SFC, GameCube,", -30, 98, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("Wii, GBA, Neo Geo MVS/AES and CD,", -30, 106, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("Dreamcast, Genesis/Mega Drive,", -30, 114, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("Sega/Mega CD, SMS, and", -30, 122, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("PCE/TG-16/PCE-Duo/SCD/SCD2", -30, 130, fontColorWhite, fontColorGray);

					mars_drawTextwShadow("Visit:", -30, 150, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("http://junkerhq.net/240p", -30, 160, fontColorGreen, fontColorGray);
					mars_drawTextwShadow("for more information", -30, 170, fontColorWhite, fontColorGray);

					mars_drawTextwShadow("Press Z to exit help", 26, 193, fontColorWhite, fontColorGray);
					break;
				}
				break;
			case HELP_PLUGE:
				switch (page)
				{
				case 1:
					mars_drawTextwShadow("PLUGE (1/3)", 50, 35, fontColorGreen, fontColorGray);

					mars_drawTextwShadow("NTSC levels require black to be", -30, 57, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("at 7.5 IRE for video. This HW", -30, 65, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("lowest is 6 IRE (6%), so using", -30, 73, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("this value for general 240p use", -30, 81, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("is not recommended.", -30, 89, fontColorWhite, fontColorGray);

					mars_drawTextwShadow("Of course using it as reference", -30, 106, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("will work perfectly for games", -30, 114, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("in this platform.", -30, 122, fontColorWhite, fontColorGray);

					mars_drawTextwShadow("In PAL - and console gaming in", -30, 139, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("general - it is adviced to use", -30, 147, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("a value of 2 IRE as black.", -30, 155, fontColorWhite, fontColorGray);

					mars_drawTextwShadow("(cont...)", 142, 178, fontColorWhite, fontColorGray);
					break;
				case 2:
					mars_drawTextwShadow("PLUGE (2/3)", 50, 35, fontColorGreen, fontColorGray);

					mars_drawTextwShadow("The PLUGE pattern is used to", -30, 57, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("help adjust the black level to", -30, 65, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("a correct value.", -30, 73, fontColorWhite, fontColorGray);

					mars_drawTextwShadow("The inner bars on the sides are", -30, 90, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("black at 6%, the outer at 12%.", -30, 98, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("If these bars are not visible,", -30, 106, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("adjust the \"brightness\" control", -30, 114, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("until they are.", -30, 122, fontColorWhite, fontColorGray);

					mars_drawTextwShadow("You should lower it until they", -30, 139, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("are not visible, and raise it", -30, 147, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("until they show.", -30, 155, fontColorWhite, fontColorGray);

					mars_drawTextwShadow("(cont...)", 142, 178, fontColorWhite, fontColorGray);
					break;
				case 3:
					mars_drawTextwShadow("PLUGE (3/3)", 50, 35, fontColorGreen, fontColorGray);

					mars_drawTextwShadow("You can change to a contrast", -30, 57, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("test with C.", -30, 65, fontColorWhite, fontColorGray);

					mars_drawTextwShadow("Within it A button changes", -30, 82, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("palettes between the original,", -30, 90, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("highest and lowest values the", -30, 98, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("hardware can display.", -30, 106, fontColorWhite, fontColorGray);
					break;
				}
				break;
			case HELP_COLORS:
				mars_drawTextwShadow("COLORBARS", 60, 35, fontColorGreen, fontColorGray);

				mars_drawTextwShadow("This pattern allows you to", -30, 57, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("calibrate each color: Red, Green", -30, 65, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("and Blue; as well as white.", -30, 73, fontColorWhite, fontColorGray);

				mars_drawTextwShadow("Adjust the white level first,", -30, 90, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("using the \"Contrast\" control", -30, 98, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("on your TV set. Raise it until", -30, 106, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("you cannot distinguish between", -30, 114, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("the blocks under \"C\" and \"E\",", -30, 122, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("and lower it slowly until you", -30, 130, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("can clearly tell them appart.", -30, 138, fontColorWhite, fontColorGray);

				mars_drawTextwShadow("Do the same for each color.", -30, 155, fontColorWhite, fontColorGray);
				break;
			case HELP_GRID:
				mars_drawTextwShadow("GRID", 75, 35, fontColorGreen, fontColorGray);

				mars_drawTextwShadow("This grid uses the full 320x224", -30, 57, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("resolution.", -30, 65, fontColorWhite, fontColorGray);

				mars_drawTextwShadow("You can use it to verify that", -30, 82, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("all the visible area is being", -30, 90, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("displayed, and that there is no", -30, 98, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("distortion present.", -30, 106, fontColorWhite, fontColorGray);

				mars_drawTextwShadow("The full active video signal can", -30, 123, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("be filled with gray by pressing", -30, 131, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("the 'A' button.", -30, 139, fontColorWhite, fontColorGray);
				break;
			case HELP_MONOSCOPE:
				switch (page)
				{
					case 1:
						mars_drawTextwShadow("MONOSCOPE (1/3)", 30, 35, fontColorGreen, fontColorGray);

						mars_drawTextwShadow("This pattern contains elements", -30, 57, fontColorWhite, fontColorGray);
						mars_drawTextwShadow("to calibrate multiple aspects", -30, 65, fontColorWhite, fontColorGray);
						mars_drawTextwShadow("of a CRT.", -30, 73, fontColorWhite, fontColorGray);

						mars_drawTextwShadow("Read your monitor's service", -30, 90, fontColorWhite, fontColorGray);
						mars_drawTextwShadow("manual to learn how, and use", -30, 98, fontColorWhite, fontColorGray);
						mars_drawTextwShadow("'A' button to change IRE.", -30, 106, fontColorWhite, fontColorGray);

						mars_drawTextwShadow("Brightness adjustment: Adjust", -30, 123, fontColorWhite, fontColorGray);
						mars_drawTextwShadow("convergence at low brightness", -30, 131, fontColorWhite, fontColorGray);
						mars_drawTextwShadow("(13/25 IRE). An overly bright", -30, 139, fontColorWhite, fontColorGray);
						mars_drawTextwShadow("pattern can mask convergence.", -30, 147, fontColorWhite, fontColorGray);
						mars_drawTextwShadow("issues.", -30, 155, fontColorWhite, fontColorGray);

						mars_drawTextwShadow("(cont...)", 142, 178, fontColorWhite, fontColorGray);
					break;
					case 2:
						mars_drawTextwShadow("MONOSCOPE (2/3)", 30, 35, fontColorGreen, fontColorGray);

						mars_drawTextwShadow("Convergence: Use the center", -30, 57, fontColorWhite, fontColorGray);
						mars_drawTextwShadow("crosshair to check static", -30, 65, fontColorWhite, fontColorGray);
						mars_drawTextwShadow("(center of screen) convergence.", -30, 73, fontColorWhite, fontColorGray);
						mars_drawTextwShadow("Use the patterns at the sides", -30, 81, fontColorWhite, fontColorGray);
						mars_drawTextwShadow("to check dynamic (edge)", -30, 89, fontColorWhite, fontColorGray);
						mars_drawTextwShadow("convergence.", -30, 97, fontColorWhite, fontColorGray);

						mars_drawTextwShadow("Corners: After setting center", -30, 114, fontColorWhite, fontColorGray);
						mars_drawTextwShadow("and edge convergence, use", -30, 122, fontColorWhite, fontColorGray);
						mars_drawTextwShadow("magnets to adjust corner purity", -30, 130, fontColorWhite, fontColorGray);
						mars_drawTextwShadow("and geometry.", -30, 138, fontColorWhite, fontColorGray);

						mars_drawTextwShadow("(cont...)", 142, 178, fontColorWhite, fontColorGray);
					break;
					case 3:
						mars_drawTextwShadow("MONOSCOPE (3/3)", 30, 35, fontColorGreen, fontColorGray);

						mars_drawTextwShadow("Size and aspect ratio: If", -30, 57, fontColorWhite, fontColorGray);
						mars_drawTextwShadow("vertical and horizontal size are", -30, 65, fontColorWhite, fontColorGray);
						mars_drawTextwShadow("correct, the red squares in the", -30, 73, fontColorWhite, fontColorGray);
						mars_drawTextwShadow("pattern will be perfect squares.", -30, 81, fontColorWhite, fontColorGray);
						mars_drawTextwShadow("After setting H size, use a tape", -30, 89, fontColorWhite, fontColorGray);
						mars_drawTextwShadow("measure to adjust V size to", -30, 97, fontColorWhite, fontColorGray);
						mars_drawTextwShadow("match it.", -30, 105, fontColorWhite, fontColorGray);

						mars_drawTextwShadow("Linearity: The squares in each", -30, 122, fontColorWhite, fontColorGray);
						mars_drawTextwShadow("corner should get you started.", -30, 130, fontColorWhite, fontColorGray);
						mars_drawTextwShadow("Confirm your adjustment using", -30, 138, fontColorWhite, fontColorGray);
						mars_drawTextwShadow("the scroll tests.", -30, 146, fontColorWhite, fontColorGray);

						mars_drawTextwShadow("Designed by Keith Raney.", -30, 164, fontColorWhite, fontColorGray);
					break;
				}
				break;				
			case HELP_BLEED:
				mars_drawTextwShadow("COLOR BLEED", 58, 35, fontColorGreen, fontColorGray);
				mars_drawTextwShadow("This pattern helps diagnose", -30, 57, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("color bleed caused by", -30, 65, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("unneeded color upsampling.", -30, 73, fontColorWhite, fontColorGray);

				mars_drawTextwShadow("You can toggle between", -30, 90, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("vertical bars and", -30, 98, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("checkerboard with 'A'.", -30, 106, fontColorWhite, fontColorGray);
				break;
			case HELP_IRE:
				mars_drawTextwShadow("100 IRE", 60, 35, fontColorGreen, fontColorGray);
				mars_drawTextwShadow("You can vary IRE intensity", -30, 57, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("with A and B. Values are:", -30, 65, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("13, 25, 41, 53, 66, 82, 94", -30, 73, fontColorWhite, fontColorGray);
				break;
			case HELP_601CB:
				mars_drawTextwShadow("601 COLORBARS", 55, 35, fontColorGreen, fontColorGray);

				mars_drawTextwShadow("You can use color filters or the", -30, 57, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("blue only option in your display", -30, 65, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("-if available- to confirm color", -30, 73, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("balance.", -30, 81, fontColorWhite, fontColorGray);
				break;
			case HELP_SHARPNESS:
				mars_drawTextwShadow("SHARPNESS", 60, 35, fontColorGreen, fontColorGray);
				mars_drawTextwShadow("You should set the sharpness of", -30, 57, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("your CRT to a value that shows", -30, 65, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("clean black and gray transitions", -30, 73, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("with no white ghosting between.", -30, 81, fontColorWhite, fontColorGray);

				mars_drawTextwShadow("On most digitally controlled", -30, 98, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("displays, sharpness is an edge-", -30, 106, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("enhancement control and most", -30, 114, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("probably should be set to zero.", -30, 122, fontColorWhite, fontColorGray);

				mars_drawTextwShadow("Pressing B toggles a diagonal", -30, 139, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("brick tileset.", -30, 147, fontColorWhite, fontColorGray);
				break;
			case HELP_OVERSCAN:
				mars_drawTextwShadow("OVERSCAN", 60, 35, fontColorGreen, fontColorGray);
				mars_drawTextwShadow("With this pattern you can", -30, 57, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("interactively find out the", -30, 65, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("overscan in pixels of each edge", -30, 73, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("in a display.", -30, 81, fontColorWhite, fontColorGray);

				mars_drawTextwShadow("Use left and right to increment", -30, 98, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("the overscan until you see the", -30, 106, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("white border, then go back one", -30, 114, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("pixel. The resulting number is", -30, 122, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("the amount of overscan in pixels", -30, 130, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("in each direction.", -30, 138, fontColorWhite, fontColorGray);
				break;
			case HELP_SMPTE:
				mars_drawTextwShadow("SMPTE COLOR BARS", 25, 35, fontColorGreen, fontColorGray);

				mars_drawTextwShadow("This pattern can be used to", -30, 57, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("calibrate for NTSC levels", -30, 65, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("regarding contrast,", -30, 73, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("brightness and colors.", -30, 81, fontColorWhite, fontColorGray);

				mars_drawTextwShadow("You can toggle between 75% and", -30, 98, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("100% SMPTE color bars with A.", -30, 106, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("Of course the percentages are", -30, 114, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("relative to the console output.", -30, 122, fontColorWhite, fontColorGray);

				mars_drawTextwShadow("You can use color filters or the", -30, 139, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("blue only option in your display", -30, 147, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("-if available- to confirm color", -30, 155, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("balance.", -30, 163, fontColorWhite, fontColorGray);

				mars_drawTextwShadow("This HW lowest black is 6%.", -30, 180, fontColorWhite, fontColorGray);
				break;
			case HELP_GRAY:
				mars_drawTextwShadow("GRAY RAMP", 60, 35, fontColorGreen, fontColorGray);
				mars_drawTextwShadow("This gray ramp pattern can be", -30, 57, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("used to check color balance.", -30, 65, fontColorWhite, fontColorGray);

				mars_drawTextwShadow("You should make sure the bars", -30, 82, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("are gray, with no red or blue", -30, 90, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("bias. This can be adjusted with", -30, 98, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("the individual color settings,", -30, 106, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("or the tint control in NTSC", -30, 114, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("displays.", -30, 122, fontColorWhite, fontColorGray);
				break;
			case HELP_WHITE:
				mars_drawTextwShadow("WHITE SCREEN", 50, 35, fontColorGreen, fontColorGray);
				mars_drawTextwShadow("This pattern can be changed", -30, 57, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("between white, black, red,", -30, 65, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("green and blue screens with the", -30, 73, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("'A' and 'B' buttons.", -30, 81, fontColorWhite, fontColorGray);

				mars_drawTextwShadow("Some displays and scalers have", -30, 98, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("issues when changing between a", -30, 106, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("black 0 IRE and a white screen.", -30, 114, fontColorWhite, fontColorGray);

				mars_drawTextwShadow("A custom color mode is", -30, 122, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("available by pressing 'C'.", -30, 130, fontColorWhite, fontColorGray);
				break;
			case HELP_CONVERGENCE:
				mars_drawTextwShadow("CONVERGENCE TESTS", 25, 35, fontColorGreen, fontColorGray);
				mars_drawTextwShadow("These are used to adjust color", -30, 57, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("convergence in CRT displays", -30, 65, fontColorWhite, fontColorGray);

				mars_drawTextwShadow("The 'A' button changes the cross", -30, 82, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("hatch pattern between lines,", -30, 90, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("dots and crosses", -30, 98, fontColorWhite, fontColorGray);

				mars_drawTextwShadow("Resolution can be changed by", -30, 115, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("pressing 'c'", -30, 123, fontColorWhite, fontColorGray);

				mars_drawTextwShadow("The 'B' button changes to a", -30, 140, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("color pattern for transition", -30, 148, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("boundary check.", -30, 156, fontColorWhite, fontColorGray);

				mars_drawTextwShadow("In color modes, 'A' toggles", -30, 173, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("black borders.", -30, 181, fontColorWhite, fontColorGray);
				break;	
			case HELP_CHECK:
				mars_drawTextwShadow("CHECKERBOARD", 50, 35, fontColorGreen, fontColorGray);
				mars_drawTextwShadow("This pattern shows all the", -30, 57, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("visible pixels in an", -30, 65, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("alternating white and black", -30, 73, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("grid array.", -30, 81, fontColorWhite, fontColorGray);

				mars_drawTextwShadow("You can toggle the pattern with", -30, 98, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("button 'Up', or turn on", -30, 106, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("auto-toggle each frame with the", -30, 114, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("'A' button. A frame counter is", -30, 122, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("also available with 'B'.", -30, 130, fontColorWhite, fontColorGray);
				break;
			case HELP_STRIPES:
				switch (page)
				{
				case 1:
					mars_drawTextwShadow("HORIZONTAL STRIPES (1/2)", 10, 35, fontColorGreen, fontColorGray);
					mars_drawTextwShadow("You should see a pattern of", -30, 57, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("lines, each one pixel in height", -30, 65, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("starting with a white one.", -30, 73, fontColorWhite, fontColorGray);

					mars_drawTextwShadow("You can toggle the pattern with", -30, 90, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("button 'UP', or turn on", -30, 98, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("auto-toggle each frame with the", -30, 106, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("'A' button. A frame counter is", -30, 114, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("also available with 'B'.", -30, 122, fontColorWhite, fontColorGray);

					mars_drawTextwShadow("When auto-toggle is set, you", -30, 139, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("should see the lines", -30, 147, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("alternating rapidly. On some", -30, 155, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("setups, the pattern doesn't", -30, 163, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("change at all. This means that", -30, 171, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("(cont...)", 142, 178, fontColorWhite, fontColorGray);
					break;
				case 2:
					mars_drawTextwShadow("HORIZONTAL STRIPES (2/2)", 10, 35, fontColorGreen, fontColorGray);
					mars_drawTextwShadow("the signal is being treated as", -30, 57, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("480i/576i and odd or even frames", -30, 65, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("are being discarded completely.", -30, 73, fontColorWhite, fontColorGray);

					mars_drawTextwShadow("You can also display vertical", -30, 90, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("bars by pressing 'LEFT'. That", -30, 98, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("pattern will help you", -30, 106, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("evaluate if the signal is not", -30, 114, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("distorted horizontaly, since", -30, 122, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("all lines should be one pixel", -30, 130, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("wide.", -30, 138, fontColorWhite, fontColorGray);

					break;
				}
				break;
			case HELP_SHADOW:
				switch (page)
				{
				case 1:
					mars_drawTextwShadow("DROP SHADOW TEST (1/2)", 10, 35, fontColorGreen, fontColorGray);
					mars_drawTextwShadow("This is a crucial test for 240p", -30, 57, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("upscan converters. It displays a", -30, 65, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("simple sprite shadow against a", -30, 73, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("background, but the shadow is", -30, 81, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("shown only on each other frame.", -30, 89, fontColorWhite, fontColorGray);

					mars_drawTextwShadow("On a CRT this achieves a", -30, 106, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("transparency effect, since you", -30, 114, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("are watching a 30hz (25Hz)", -30, 122, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("shadow on a 60hz (50hz) signal.", -30, 130, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("No background detail should be", -30, 138, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("lost and the shadow should be", -30, 146, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("visible.", -30, 154, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("(cont...)", 142, 178, fontColorWhite, fontColorGray);
					break;
				case 2:
					mars_drawTextwShadow("DROP SHADOW TEST (2/2)", 15, 35, fontColorGreen, fontColorGray);
					mars_drawTextwShadow("The user can toggle the frame", -30, 57, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("used to draw the shadow with", -30, 65, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("button 'A'. Backgrounds can be", -30, 73, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("switched with the 'B' button and", -30, 81, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("button 'C' toggles sprites.", -30, 89, fontColorWhite, fontColorGray);
					break;
				}
				break;
			case HELP_STRIPED:
				mars_drawTextwShadow("STRIPED SPRITE TEST", 19, 35, fontColorGreen, fontColorGray);
				mars_drawTextwShadow("There are deinterlacers out", -30, 57, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("there that can display the drop", -30, 65, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("shadows correctly and still", -30, 73, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("interpret 240p as 480i. With a", -30, 81, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("striped sprite it should be easy", -30, 89, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("to tell if a processor tries to", -30, 97, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("deinterlace (plus interpolate).", -30, 105, fontColorWhite, fontColorGray);

				mars_drawTextwShadow("You can change backgrounds with", -30, 122, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("'A'.", -30, 130, fontColorWhite, fontColorGray);
				break;
			case HELP_MANUALLAG:
				switch (page)
				{
				case 1:
					mars_drawTextwShadow("MANUAL LAG TEST (1/3)", 10, 35, fontColorGreen, fontColorGray);
					mars_drawTextwShadow("The main intention is to show a", -30, 57, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("changing pattern on the screen,", -30, 65, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("which can be complemented with", -30, 73, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("audio. This should show to some", -30, 81, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("degree any lag when processing", -30, 89, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("the signal.", -30, 97, fontColorWhite, fontColorGray);

					mars_drawTextwShadow("As an added feature, the user", -30, 114, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("can click the 'A' button when", -30, 122, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("the sprite is aligned with the", -30, 130, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("one on the background, and the", -30, 138, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("offset in frames from the actual", -30, 146, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("intersection will be shown on", -30, 154, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("screen. A 1khz tone will be", -30, 162, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("played for 1 frame when pressed.", -30, 170, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("(cont...)", 142, 178, fontColorWhite, fontColorGray);
					break;
				case 2:
					mars_drawTextwShadow("MANUAL LAG TEST (2/3)", 10, 35, fontColorGreen, fontColorGray);
					mars_drawTextwShadow("This can be repeated ten", -30, 57, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("times and the software will", -30, 65, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("calculate the average. Whenever", -30, 73, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("the button was pressed before", -30, 81, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("the actual intersection frame,", -30, 89, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("the result will be ignored, but", -30, 97, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("still shown onscreen. Button 'B'", -30, 105, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("can be used to change the", -30, 113, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("direction of the sprite from", -30, 121, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("vertical to horizontal, or both", -30, 129, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("at the same time.", -30, 137, fontColorWhite, fontColorGray);

					mars_drawTextwShadow("Of course the evaluation is", -30, 154, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("dependent on reflexes and/or", -30, 162, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("rhythm more than anything. The", -30, 170, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("(cont...)", 142, 178, fontColorWhite, fontColorGray);
					break;
				case 3:
					mars_drawTextwShadow("MANUAL LAG TEST (3/3)", 10, 35, fontColorGreen, fontColorGray);
					mars_drawTextwShadow("visual and audio cues are the", -30, 57, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("more revealing aspects which", -30, 65, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("the user should consider, of", -30, 73, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("course the interactive factor", -30, 81, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("can give an experienced player", -30, 89, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("the hang of the system when", -30, 97, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("testing via different", -30, 105, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("connections.", -30, 113, fontColorWhite, fontColorGray);
					break;
				}
				break;
			case HELP_HSCROLL:
				switch (page)
				{
				case 1:
					mars_drawTextwShadow("SCROLL TEST (1/2)", 50, 35, fontColorGreen, fontColorGray);
					mars_drawTextwShadow("This test shows either an", -30, 57, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("horizontal 320x224 back from", -30, 65, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("Sonic or a vertical 256x224", -30, 73, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("back from Kiki Kaikai.", -30, 81, fontColorWhite, fontColorGray);

					mars_drawTextwShadow("Speed can be varied with Up &", -30, 98, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("Down and scroll direction with", -30, 106, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("Left. The 'A' button stops the", -30, 114, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("scroll and 'B' toggles between", -30, 122, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("vertical and horizontal.", -30, 130, fontColorWhite, fontColorGray);

					mars_drawTextwShadow("The vertical 256 scroll is also", -30, 147, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("used while in 320 mode but it", -30, 155, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("can be positioned with 'Right'.", -30, 163, fontColorWhite, fontColorGray);
					
					mars_drawTextwShadow("(cont...)", 142, 178, fontColorWhite, fontColorGray);
					break;					
				case 2:
					mars_drawTextwShadow("SCROLL TEST (2/2)", 50, 35, fontColorGreen, fontColorGray);
					mars_drawTextwShadow("This can be used to notice any", -30, 57, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("drops in framerate, or pixel", -30, 65, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("width inconsistencies.", -30, 73, fontColorWhite, fontColorGray);

					mars_drawTextwShadow("Sonic is a trademark of Sega", -30, 90, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("Enterprises Ltd. Kiki Kaikai", -30, 98, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("is a trademark of Taito.", -30, 106, fontColorWhite, fontColorGray);
					break;
				}
				break;
			case HELP_VSCROLL:
				mars_drawTextwShadow("GRID SCROLL TEST", 60, 35, fontColorGreen, fontColorGray);
				mars_drawTextwShadow("A grid is scrolled vertically or", -30, 57, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("horizontally, which can be used", -30, 65, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("to test linearity of the signal", -30, 73, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("and how well the display or", -30, 81, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("video processor copes with", -30, 89, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("scrolling and framerate.", -30, 97, fontColorWhite, fontColorGray);

				mars_drawTextwShadow("Button 'B' can be used to toggle", -30, 114, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("between horizontal and vertical,", -30, 122, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("while Up/Down regulate speed.", -30, 130, fontColorWhite, fontColorGray);

				mars_drawTextwShadow("Button 'A' stops the scroll and", -30, 147, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("'Left' changes direction.", -30, 155, fontColorWhite, fontColorGray);
				break;
		case HELP_LED:
				switch (page)
				{
				case 1:
					mars_drawTextwShadow("BACKLIT TEST (1/2)", 20, 35, fontColorGreen, fontColorGray);
					mars_drawTextwShadow("This test allows you to check", -30, 57, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("how the display's backlit works", -30, 65, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("when only a small array of", -30, 73, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("pixels is shown. This can be", -30, 81, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("very revealing and can be used", -30, 89, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("to detect the zones in LED", -30, 97, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("backlit displays, or to evaluate", -30, 105, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("global and local dimming.", -30, 113, fontColorWhite, fontColorGray);

					mars_drawTextwShadow("The user can move around the", -30, 130, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("white pixel arrays with the", -30, 138, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("d-pad, and change the size of", -30, 146, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("the pixel array with 'A'.", -30, 154, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("The 'B' button allows the user", -30, 162, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("to hide the pixel array in", -30, 170, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("(cont...)", 142, 178, fontColorWhite, fontColorGray);
					break;
				case 2:
					mars_drawTextwShadow("BACKLIT TEST (2/2)", 20, 35, fontColorGreen, fontColorGray);
					mars_drawTextwShadow("order to alternate a fully black", -30, 57, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("screen.", -30, 65, fontColorWhite, fontColorGray);

					mars_drawTextwShadow("It might be necessary to have a", -30, 82, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("fully-off panel (if possible)", -30, 90, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("when displaying the black", -30, 98, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("background,  in order to", -30, 106, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("evaluate full-backlit displays", -30, 114, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("by adjusting brightness", -30, 122, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("accordingly for this test.", -30, 130, fontColorWhite, fontColorGray);
					break;
				}
				break;
			case HELP_LAG:
				switch (page)
				{
				case 1:
					mars_drawTextwShadow("LAG TEST (1/2)", 60, 35, fontColorGreen, fontColorGray);
					mars_drawTextwShadow("This test is designed to be used", -30, 57, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("with two displays conected at", -30, 65, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("the same time. One being a CRT,", -30, 73, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("or a display with a known lag as", -30, 81, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("reference, and the other the", -30, 89, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("display to test.", -30, 97, fontColorWhite, fontColorGray);

					mars_drawTextwShadow("Using a camera, a picture should", -30, 114, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("be taken of both screens at the", -30, 122, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("same time. The picture will show", -30, 130, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("the frame discrepancy between", -30, 138, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("them.", -30, 146, fontColorWhite, fontColorGray);

					mars_drawTextwShadow("The circles in the bottom help", -30, 163, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("determine the frame even when", -30, 171, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("(cont...)", 142, 178, fontColorWhite, fontColorGray);
					break;
				case 2:
					mars_drawTextwShadow("LAG TEST (2/2)", 60, 35, fontColorGreen, fontColorGray);
					mars_drawTextwShadow("the numbers are blurry.", -30, 57, fontColorWhite, fontColorGray);

					mars_drawTextwShadow("This version of the suite can be", -30, 74, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("used with a Sega Nomad as the", -30, 82, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("reference display.", -30, 90, fontColorWhite, fontColorGray);

					mars_drawTextwShadow("You can also split the video", -30, 107, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("signal and feed both displays.", -30, 115, fontColorWhite, fontColorGray);

					mars_drawTextwShadow("The vertical bars on the sides", -30, 132, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("change color each frame to help", -30, 140, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("when using LCD photos.", -30, 148, fontColorWhite, fontColorGray);

					mars_drawTextwShadow("Press A to start/stop, B to", -30, 165, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("reset and C for B&W test.", -30, 173, fontColorWhite, fontColorGray);
					break;
				}
				break;
			case HELP_ALTERNATE:
				mars_drawTextwShadow("ALTERNATE 240p/480i", 60, 35, fontColorGreen, fontColorGray);
				mars_drawTextwShadow("Some devices have a delay when", -30, 57, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("the source changes between", -30, 65, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("resolutions, which happens in", -30, 73, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("some games. This test allows to", -30, 81, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("time that delay manually.", -30, 89, fontColorWhite, fontColorGray);

				mars_drawTextwShadow("Press A to switch the", -30, 106, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("resolution; press it again when", -30, 114, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("you are able to see the screen", -30, 122, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("back in your display.", -30, 130, fontColorWhite, fontColorGray);
				break;
			case HELP_VIDEO:
				mars_drawTextwShadow("VIDEO OPTIONS", 60, 35, fontColorGreen, fontColorGray);
				mars_drawTextwShadow("Here you can enable interlaced", -30, 57, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("(480i) mode for the whole suite", -30, 65, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("and disable horizontal 256 in", -30, 73, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("compatible screens.", -30, 81, fontColorWhite, fontColorGray);

				mars_drawTextwShadow("For PAL consoles, you can also", -30, 98, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("enable full 240p, instead of", -30, 106, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("224p.", -30, 114, fontColorWhite, fontColorGray);
				break;
			case HELP_AUDIOSYNC:
				mars_drawTextwShadow("AUDIO SYNC", 60, 35, fontColorGreen, fontColorGray);
				mars_drawTextwShadow("This test flashes the whole", -30, 57, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("screen white for 2 frames,", -30, 65, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("along with a 1khz test tone.", -30, 73, fontColorWhite, fontColorGray);

				mars_drawTextwShadow("You can verify the sync between", -30, 90, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("audio and video with recording", -30, 98, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("equipment or specialized", -30, 106, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("hardware.", -30, 114, fontColorWhite, fontColorGray);

				mars_drawTextwShadow("Press A to start/stop the test.", -30, 131, fontColorWhite, fontColorGray);
				break;
			case HELP_CONTRAST:
				mars_drawTextwShadow("PLUGE CONTRAST", 60, 35, fontColorGreen, fontColorGray);
				mars_drawTextwShadow("This sub test fills the screen", -30, 57, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("with a pattern. By default it", -30, 65, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("is drawn in color.", -30, 73, fontColorWhite, fontColorGray);

				mars_drawTextwShadow("You can change palettes with", -30, 90, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("A to display a dark or white", -30, 98, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("pattern in order to check", -30, 106, fontColorWhite, fontColorGray);
				mars_drawTextwShadow("contrast and brightness.", -30, 114, fontColorWhite, fontColorGray);

				mars_drawTextwShadow("The C button returns to PLUGE.", -30, 131, fontColorWhite, fontColorGray);
				break;
			case HELP_MEMVIEW:
				switch (page)
				{
				case 1:
					mars_drawTextwShadow("MEMORY VIEWER", 60, 35, fontColorGreen, fontColorGray);
					mars_drawTextwShadow("This shows selected regions of", -30, 57, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("the main CPU memory map.", -30, 65, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("The current address range is", -30, 73, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("shown in red at the right from", -30, 81, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("top to bottom.", -30, 89, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("The left part of the screen", -30, 97, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("shows 0x1C0 bytes in hex.", -30, 105, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("- Button A enabled CRC of of", -30, 113, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("current screen.", -30, 121, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("- Button B jumps to relevant", -30, 129, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("memory locations", -30, 137, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("(cont...)", 142, 178, fontColorWhite, fontColorGray);
					break;
				case 2:
					mars_drawTextwShadow("- Button C switches banks in", -30, 57, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("SCD Program RAM.", -30, 65, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("With a working SCD system", -30, 73, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("Program RAM banks should show", -30, 81, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("0xAA for bank 1 (0xBB for bank", -30, 89, fontColorWhite, fontColorGray);
					mars_drawTextwShadow("2 etc.) in all four corners.", -30, 97, fontColorWhite, fontColorGray);
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