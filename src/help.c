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

#include "types.h"
#include "32x.h"
#include "hw_32x.h"
#include "32x_images.h"
#include "shared_objects.h"
#include "help.h"

void DrawHelp(int option)
{
	u16 button, pressedButton, oldButton = 0xFFFF;
	u16 exit = 0, totalpages = 1, page = 1;

	marsVDP256Start();

	initMainBG();

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
			button = MARS_SYS_COMM10;

		pressedButton = button & ~oldButton;
		oldButton = button & 0x0FFF;

		drawMainBG();
		loadTextPalette();

		switch (option)
		{
			case HELP_GENERAL:
				switch (page)
				{
					case 1:
						drawTextwHighlight("HELP (1/2)", 125, 35, fontColorGreen, fontColorGreenHighlight);

						drawTextwHighlight("The 240p Test Suite was designed", 35, 57, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("with two goals in mind:", 35, 65, fontColorWhite, fontColorWhiteHighlight);

						drawTextwHighlight("1) Evaluate 240p signals on TV", 35, 81, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("sets and video processors; and", 35, 90, fontColorWhite, fontColorWhiteHighlight);

						drawTextwHighlight("2) Provide calibration patterns", 35, 106, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("from a game console to help in", 35, 114, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("properly calibrating the display", 35, 122, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("black, white, and color levels.", 35, 130, fontColorWhite, fontColorWhiteHighlight);

						drawTextwHighlight("Help can be shown with 'Z' in", 35, 146, fontColorGreen, fontColorGreenHighlight);
						drawTextwHighlight("any test.", 35, 154, fontColorGreen, fontColorGreenHighlight);

						drawTextwHighlight("(cont...)", 207, 178, fontColorWhite, fontColorWhiteHighlight);
						if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_THREE)
							drawTextwHighlight("Press C to exit help", 91, 193, fontColorWhite, fontColorWhiteHighlight);
						else
							drawTextwHighlight("Press Z to exit help", 91, 193, fontColorWhite, fontColorWhiteHighlight);
						break;

					case 2:
						drawTextwHighlight("HELP (2/2)", 125, 35, fontColorGreen, fontColorGreenHighlight);

						drawTextwHighlight("The Sega 32X can output 224", 35, 50, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("active video lines. In PAL", 35, 58, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("consoles, it can display either", 35, 66, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("224 or 240 lines.", 35, 74, fontColorWhite, fontColorWhiteHighlight);

						drawTextwHighlight("Visit:", 35, 90, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("http://junkerhq.net/240p", 35, 98, fontColorGreen, fontColorGreenHighlight);
						drawTextwHighlight("for more information.", 35, 106, fontColorWhite, fontColorWhiteHighlight);

						drawQRCode(256, 122, 32, 32);

						if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_THREE)
							drawTextwHighlight("Press C to exit help", 91, 193, fontColorWhite, fontColorWhiteHighlight);
						else
							drawTextwHighlight("Press Z to exit help", 91, 193, fontColorWhite, fontColorWhiteHighlight);
						break;
				}
				break;

			case HELP_PLUGE:
				switch (page)
				{
					case 1:
						drawTextwHighlight("PLUGE (1/3)", 115, 35, fontColorGreen, fontColorGreenHighlight);

						drawTextwHighlight("NTSC levels require black to be", 32, 57, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("at 7.5 IRE for video. This HW", 32, 65, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("lowest is 6 IRE (6%), so using", 32, 73, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("this value for general 240p use", 32, 81, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("is not recommended.", 32, 89, fontColorWhite, fontColorWhiteHighlight);

						drawTextwHighlight("Of course using it as reference", 32, 106, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("will work perfectly for games", 32, 114, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("in this platform.", 32, 122, fontColorWhite, fontColorWhiteHighlight);

						drawTextwHighlight("In PAL - and console gaming in", 32, 139, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("general - it is adviced to use", 32, 147, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("a value of 2 IRE as black.", 32, 155, fontColorWhite, fontColorWhiteHighlight);

						drawTextwHighlight("(cont...)", 207, 178, fontColorWhite, fontColorWhiteHighlight);
						break;

					case 2:
						drawTextwHighlight("PLUGE (2/3)", 115, 35, fontColorGreen, fontColorGreenHighlight);

						drawTextwHighlight("The PLUGE pattern is used to", 32, 57, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("help adjust the black level to", 32, 65, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("a correct value.", 32, 73, fontColorWhite, fontColorWhiteHighlight);

						drawTextwHighlight("The inner bars on the sides are", 32, 90, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("black at 6%, the outer at 12%.", 32, 98, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("If these bars are not visible,", 32, 106, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("adjust the \"brightness\" control", 32, 114, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("until they are.", 32, 122, fontColorWhite, fontColorWhiteHighlight);

						drawTextwHighlight("You should lower it until they", 32, 139, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("are not visible, and raise it", 32, 147, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("until they show.", 32, 155, fontColorWhite, fontColorWhiteHighlight);

						drawTextwHighlight("(cont...)", 207, 178, fontColorWhite, fontColorWhiteHighlight);
						break;

					case 3:
						drawTextwHighlight("PLUGE (3/3)", 115, 35, fontColorGreen, fontColorGreenHighlight);

						drawTextwHighlight("Within it A button changes", 32, 56, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("palettes between the original,", 32, 64, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("highest and lowest values the", 32, 72, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("hardware can display.", 32, 80, fontColorWhite, fontColorWhiteHighlight);
						break;
				}
				break;

			case HELP_COLORS:
				drawTextwHighlight("COLORBARS", 125, 35, fontColorGreen, fontColorGreenHighlight);

				drawTextwHighlight("This pattern allows you to", 32, 57, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("calibrate each color: Red, Green", 32, 65, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("and Blue; as well as white.", 32, 73, fontColorWhite, fontColorWhiteHighlight);

				drawTextwHighlight("Adjust the white level first,", 32, 90, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("using the \"Contrast\" control", 32, 98, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("on your TV set. Raise it until", 32, 106, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("you cannot distinguish between", 32, 114, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("the blocks under \"E\" and \"F\",", 32, 122, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("and lower it slowly until you", 32, 130, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("can clearly tell them appart.", 32, 138, fontColorWhite, fontColorWhiteHighlight);

				drawTextwHighlight("Do the same for each color.", 32, 155, fontColorWhite, fontColorWhiteHighlight);
				break;

			case HELP_GRID:
				drawTextwHighlight("GRID", 140, 35, fontColorGreen, fontColorGreenHighlight);

				drawTextwHighlight("This grid uses the full 320x224", 32, 57, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("resolution.", 32, 65, fontColorWhite, fontColorWhiteHighlight);

				drawTextwHighlight("You can use it to verify that", 32, 82, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("all the visible area is being", 32, 90, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("displayed, and that there is no", 32, 98, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("distortion present.", 32, 106, fontColorWhite, fontColorWhiteHighlight);

				drawTextwHighlight("The full active video signal can", 32, 123, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("be filled with gray by pressing", 32, 131, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("the 'A' button.", 32, 139, fontColorWhite, fontColorWhiteHighlight);
				break;

			case HELP_MONOSCOPE:
				switch (page)
				{
					case 1:
						drawTextwHighlight("MONOSCOPE (1/3)", 95, 35, fontColorGreen, fontColorGreenHighlight);

						drawTextwHighlight("This pattern contains elements", 32, 57, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("to calibrate multiple aspects", 32, 65, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("of a CRT.", 32, 73, fontColorWhite, fontColorWhiteHighlight);

						drawTextwHighlight("Read your monitor's service", 32, 90, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("manual to learn how, and use", 32, 98, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("'A' button to change IRE.", 32, 106, fontColorWhite, fontColorWhiteHighlight);

						drawTextwHighlight("Brightness adjustment: Adjust", 32, 123, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("convergence at low brightness", 32, 131, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("(13/25 IRE). An overly bright", 32, 139, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("pattern can mask convergence.", 32, 147, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("issues.", 32, 155, fontColorWhite, fontColorWhiteHighlight);

						drawTextwHighlight("(cont...)", 207, 178, fontColorWhite, fontColorWhiteHighlight);
						break;

					case 2:
						drawTextwHighlight("MONOSCOPE (2/3)", 95, 35, fontColorGreen, fontColorGreenHighlight);

						drawTextwHighlight("Convergence: Use the center", 32, 57, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("crosshair to check static", 32, 65, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("(center of screen) convergence.", 32, 73, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("Use the patterns at the sides", 32, 81, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("to check dynamic (edge)", 32, 89, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("convergence.", 32, 97, fontColorWhite, fontColorWhiteHighlight);

						drawTextwHighlight("Corners: After setting center", 32, 114, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("and edge convergence, use", 32, 122, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("magnets to adjust corner purity", 32, 130, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("and geometry.", 32, 138, fontColorWhite, fontColorWhiteHighlight);

						drawTextwHighlight("(cont...)", 207, 178, fontColorWhite, fontColorWhiteHighlight);
						break;

					case 3:
						drawTextwHighlight("MONOSCOPE (3/3)", 95, 35, fontColorGreen, fontColorGreenHighlight);

						drawTextwHighlight("Size and aspect ratio: If", 32, 57, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("vertical and horizontal size are", 32, 65, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("correct, the red squares in the", 32, 73, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("pattern will be perfect squares.", 32, 81, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("After setting H size, use a tape", 32, 89, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("measure to adjust V size to", 32, 97, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("match it.", 32, 105, fontColorWhite, fontColorWhiteHighlight);

						drawTextwHighlight("Linearity: The squares in each", 32, 122, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("corner should get you started.", 32, 130, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("Confirm your adjustment using", 32, 138, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("the scroll tests.", 32, 146, fontColorWhite, fontColorWhiteHighlight);

						drawTextwHighlight("Designed by Keith Raney.", 32, 164, fontColorWhite, fontColorWhiteHighlight);
						break;
				}
				break;

			case HELP_BLEED:
				drawTextwHighlight("COLOR BLEED", 123, 35, fontColorGreen, fontColorGreenHighlight);
				drawTextwHighlight("This pattern helps diagnose", 32, 56, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("color bleed caused by", 32, 64, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("unneeded color upsampling.", 32, 72, fontColorWhite, fontColorWhiteHighlight);

				drawTextwHighlight("You can toggle between", 32, 90, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("vertical bars and", 32, 98, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("checkerboard with 'A'.", 32, 106, fontColorWhite, fontColorWhiteHighlight);
				break;

			case HELP_IRE:
				drawTextwHighlight("100 IRE", 125, 35, fontColorGreen, fontColorGreenHighlight);
				drawTextwHighlight("You can vary IRE intensity", 32, 56, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("with 'A' and 'B'. Values are:", 32, 64, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("13, 25, 41, 53, 66, 82, 94", 32, 72, fontColorWhite, fontColorWhiteHighlight);
				break;

			case HELP_601CB:
				drawTextwHighlight("601 COLORBARS", 120, 35, fontColorGreen, fontColorGreenHighlight);
				drawTextwHighlight("You can use color filters or the", 32, 56, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("blue only option in your display", 32, 64, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("to confirm color balance.", 32, 72, fontColorWhite, fontColorWhiteHighlight);
				break;

			case HELP_SHARPNESS:
				drawTextwHighlight("SHARPNESS", 125, 35, fontColorGreen, fontColorGreenHighlight);
				drawTextwHighlight("You should set the sharpness of", 32, 56, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("your CRT to a value that shows", 32, 64, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("clean black and gray transitions", 32, 72, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("with no white ghosting between.", 32, 80, fontColorWhite, fontColorWhiteHighlight);

				drawTextwHighlight("Pressing 'A' toggles a diagonal", 32, 96, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("brick tileset.", 32, 104, fontColorWhite, fontColorWhiteHighlight);
				break;

			case HELP_OVERSCAN:
				drawTextwHighlight("OVERSCAN", 125, 35, fontColorGreen, fontColorGreenHighlight);
				drawTextwHighlight("With this pattern you can", 32, 56, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("interactively find out the", 32, 64, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("overscan in pixels of each edge", 32, 72, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("in a display.", 32, 80, fontColorWhite, fontColorWhiteHighlight);

				drawTextwHighlight("Use left and right to increment", 32, 96, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("the overscan until you see the", 32, 104, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("white border, then go back one", 32, 112, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("pixel. The resulting number is", 32, 120, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("the amount of overscan in pixels", 32, 128, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("in each direction.", 32, 136, fontColorWhite, fontColorWhiteHighlight);
				break;

			case HELP_SMPTE:
				drawTextwHighlight("SMPTE COLOR BARS", 90, 35, fontColorGreen, fontColorGreenHighlight);

				drawTextwHighlight("This pattern can be used to", 32, 56, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("approximate for NTSC levels", 32, 64, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("regarding contrast,", 32, 72, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("brightness and colors.", 32, 80, fontColorWhite, fontColorWhiteHighlight);

				drawTextwHighlight("You can toggle between 75% and", 32, 96, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("100% SMPTE color bars with A.", 32, 104, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("Of course the percentages are", 32, 112, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("relative to the console output.", 32, 120, fontColorWhite, fontColorWhiteHighlight);

				drawTextwHighlight("You can use color filters or the", 32, 136, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("blue only option in your display", 32, 144, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("to confirm color balance", 32, 152, fontColorWhite, fontColorWhiteHighlight);
				break;

			case HELP_GRAY:
				drawTextwHighlight("GRAY RAMP", 125, 35, fontColorGreen, fontColorGreenHighlight);
				drawTextwHighlight("This gray ramp pattern can be", 32, 56, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("used to check color balance.", 32, 64, fontColorWhite, fontColorWhiteHighlight);

				drawTextwHighlight("You should make sure the bars", 32, 80, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("are gray, with no color bias.", 32, 88, fontColorWhite, fontColorWhiteHighlight);
				break;

			case HELP_WHITE:
				drawTextwHighlight("WHITE SCREEN", 115, 35, fontColorGreen, fontColorGreenHighlight);
				drawTextwHighlight("This pattern can be changed", 32, 56, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("between white, black, red,", 32, 64, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("green and blue screens with the", 32, 72, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("'A' and 'B' buttons.", 32, 80, fontColorWhite, fontColorWhiteHighlight);

				drawTextwHighlight("Some displays and scalers have", 32, 96, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("issues when changing between a", 32, 104, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("black 0 IRE and a white screen.", 32, 112, fontColorWhite, fontColorWhiteHighlight);

				drawTextwHighlight("A custom color mode is", 32, 128, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("available by pressing 'C'.", 32, 136, fontColorWhite, fontColorWhiteHighlight);
				break;

			case HELP_CONVERGENCE:
				drawTextwHighlight("CONVERGENCE TESTS", 90, 35, fontColorGreen, fontColorGreenHighlight);
				drawTextwHighlight("These are used to adjust color", 32, 56, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("convergence in CRT displays", 32, 64, fontColorWhite, fontColorWhiteHighlight);

				drawTextwHighlight("The 'A' button changes the cross", 32, 80, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("hatch pattern between lines,", 32, 88, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("dots and crosses", 32, 96, fontColorWhite, fontColorWhiteHighlight);

				drawTextwHighlight("The 'B' button changes to a", 32, 112, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("color pattern for transition", 32, 120, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("boundary check.", 32, 128, fontColorWhite, fontColorWhiteHighlight);
				break;

			case HELP_DISAPPEAR:
				drawTextwHighlight("DISAPPEARING LOGO", 90, 35, fontColorGreen, fontColorGreenHighlight);
				drawTextwHighlight("This test allows you to use a", 32, 56, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("slow motion camera to measure", 32, 64, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("how long it takes from a button", 32, 72, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("press to when the logo", 32, 80, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("disappears.", 32, 88, fontColorWhite, fontColorWhiteHighlight);

				drawTextwHighlight("You can also use a photodiode", 32, 104, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("and press 'C', to turn the", 32, 112, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("whole background white for", 32, 120, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("2 frames.", 32, 128, fontColorWhite, fontColorWhiteHighlight);
				break;

			case HELP_CHECK:
				drawTextwHighlight("CHECKERBOARD", 115, 35, fontColorGreen, fontColorGreenHighlight);
				drawTextwHighlight("This pattern shows all the", 32, 57, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("visible pixels in an", 32, 65, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("alternating white and black", 32, 73, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("grid array.", 32, 81, fontColorWhite, fontColorWhiteHighlight);

				drawTextwHighlight("You can toggle the pattern with", 32, 98, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("button 'Up', or turn on", 32, 106, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("auto-toggle each frame with the", 32, 114, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("'A' button. A frame counter is", 32, 122, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("also available with 'B'.", 32, 130, fontColorWhite, fontColorWhiteHighlight);
				break;

			case HELP_STRIPES:
				switch (page)
				{
					case 1:
						drawTextwHighlight("HOR/VER STRIPES (1/2)", 75, 35, fontColorGreen, fontColorGreenHighlight);
						drawTextwHighlight("You should see a pattern of", 32, 57, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("lines, each one pixel in height", 32, 65, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("starting with a white one.", 32, 73, fontColorWhite, fontColorWhiteHighlight);

						drawTextwHighlight("You can toggle the pattern with", 32, 90, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("button 'UP', or turn on", 32, 98, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("auto-toggle each frame with the", 32, 106, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("'A' button. A frame counter is", 32, 114, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("also available with 'B'.", 32, 122, fontColorWhite, fontColorWhiteHighlight);

						drawTextwHighlight("When auto-toggle is set, you", 32, 139, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("should see the lines", 32, 147, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("alternating rapidly.", 32, 155, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("(cont...)", 207, 178, fontColorWhite, fontColorWhiteHighlight);
						break;

					case 2:
						drawTextwHighlight("HOR/VER STRIPES (2/2)", 75, 35, fontColorGreen, fontColorGreenHighlight);
						drawTextwHighlight("You can also display vertical", 32, 57, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("bars by pressing 'LEFT'. That", 32, 65, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("pattern will help you", 32, 73, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("evaluate if the signal is not", 32, 81, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("distorted horizontaly, since", 32, 89, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("all lines should be one pixel", 32, 97, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("wide.", 32, 105, fontColorWhite, fontColorWhiteHighlight);
						break;
				}
				break;

			case HELP_SHADOW:
				drawTextwHighlight("DROP SHADOW TEST", 96, 35, fontColorGreen, fontColorGreenHighlight);
				drawTextwHighlight("This is a crucial test for 240p", 32, 56, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("It displays a simple sprite", 32, 64, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("shadow against a background,", 32, 72, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("but the shadow is shown only", 32, 80, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("on each other frame. Achieving", 32, 88, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("a transparency effect.", 32, 96, fontColorWhite, fontColorWhiteHighlight);

				drawTextwHighlight("The user can toggle the frame", 32, 112, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("used to draw the shadow with", 32, 120, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("button 'A'. Backgrounds can be", 32, 128, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("switched with the 'B' button,", 32, 136, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("and button 'C' toggles sprites.", 32, 144, fontColorWhite, fontColorWhiteHighlight);
				break;

			case HELP_STRIPED:
				drawTextwHighlight("STRIPED SPRITE TEST", 84, 35, fontColorGreen, fontColorGreenHighlight);
				drawTextwHighlight("There are deinterlacers out", 32, 57, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("there that can display the drop", 32, 65, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("shadows correctly and still", 32, 73, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("interpret 240p as 480i. With a", 32, 81, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("striped sprite it should be easy", 32, 89, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("to tell if a processor tries to", 32, 97, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("deinterlace (plus interpolate).", 32, 105, fontColorWhite, fontColorWhiteHighlight);

				drawTextwHighlight("You can change backgrounds with", 32, 122, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("'A'.", 32, 130, fontColorWhite, fontColorWhiteHighlight);
				break;

			case HELP_MANUALLAG:
				switch (page)
				{
					case 1:
						drawTextwHighlight("TIMING & REFLEX (1/2)", 75, 35, fontColorGreen, fontColorGreenHighlight);
						drawTextwHighlight("The main intention is to show a", 32, 56, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("changing pattern on the screen,", 32, 64, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("which can be complemented with", 32, 72, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("audio. This should show to some", 32, 80, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("degree any lag when processing", 32, 88, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("the signal.", 32, 96, fontColorWhite, fontColorWhiteHighlight);

						drawTextwHighlight("As an added feature, the user", 32, 112, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("can click the 'A' button when", 32, 120, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("the sprite is aligned with the", 32, 128, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("one on the background, and the", 32, 136, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("offset in frames from the actual", 32, 144, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("intersection will be shown on", 32, 152, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("screen. A 1khz tone will be", 32, 160, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("played for 1 frame when pressed.", 32, 168, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("(cont...)", 207, 178, fontColorWhite, fontColorWhiteHighlight);
						break;

					case 2:
						drawTextwHighlight("TIMING & REFLEX (2/2)", 75, 35, fontColorGreen, fontColorGreenHighlight);
						drawTextwHighlight("Button 'B' can be used to", 32, 56, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("change the direction of the", 32, 64, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("sprite.", 32, 72, fontColorWhite, fontColorWhiteHighlight);

						drawTextwHighlight("Of course the evaluation is", 32, 88, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("dependent on reflexes and/or", 32, 96, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("rhythm more than anything. The", 32, 104, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("visual and audio cues are the", 32, 112, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("more revealing aspects which", 32, 120, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("the user should consider, of", 32, 128, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("course the interactive factor", 32, 136, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("can give an experienced player", 32, 144, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("the hang of the system when", 32, 152, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("testing via different", 32, 160, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("connections.", 32, 168, fontColorWhite, fontColorWhiteHighlight);
						break;
				}
				break;

			case HELP_HSCROLL:
				drawTextwHighlight("SCROLL TEST", 115, 35, fontColorGreen, fontColorGreenHighlight);
				drawTextwHighlight("This test shows either an", 32, 57, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("horizontal 320x224 back from", 32, 65, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("Sonic or a vertical 256x224", 32, 73, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("back from Kiki Kaikai.", 32, 81, fontColorWhite, fontColorWhiteHighlight);

				drawTextwHighlight("Speed can be varied with Up &", 32, 98, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("Down and scroll direction with", 32, 106, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("Left. The 'A' button stops the", 32, 114, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("scroll and 'B' toggles between", 32, 122, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("vertical and horizontal.", 32, 130, fontColorWhite, fontColorWhiteHighlight);

				drawTextwHighlight("This can be used to notice any", 32, 147, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("drops in framerate, or pixel", 32, 155, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("width inconsistencies.", 32, 163, fontColorWhite, fontColorWhiteHighlight);
				break;

			case HELP_VSCROLL:
				drawTextwHighlight("GRID SCROLL TEST", 104, 35, fontColorGreen, fontColorGreenHighlight);
				drawTextwHighlight("A grid is scrolled vertically or", 32, 57, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("horizontally, which can be used", 32, 65, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("to test linearity of the signal", 32, 73, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("and how well the display or", 32, 81, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("video processor copes with", 32, 89, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("scrolling and framerate.", 32, 97, fontColorWhite, fontColorWhiteHighlight);

				drawTextwHighlight("Button 'B' can be used to toggle", 32, 114, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("between horizontal and vertical,", 32, 122, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("while Up/Down regulate speed.", 32, 130, fontColorWhite, fontColorWhiteHighlight);

				drawTextwHighlight("Button 'A' stops the scroll and", 32, 147, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("'Left' changes direction.", 32, 155, fontColorWhite, fontColorWhiteHighlight);
				break;

			case HELP_SOUND:
				drawTextwHighlight("SOUND TEST", 125, 35, fontColorGreen, fontColorGreenHighlight);
				drawTextwHighlight("You can test the sound from the", 32, 56, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("32X 2-channel PWM and", 32, 64, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("Genesis/Mega Drive PSG here.", 32, 72, fontColorWhite, fontColorWhiteHighlight);

				drawTextwHighlight("Panning can be changed when", 32, 88, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("possible. This can help you", 32, 96, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("identify stereo cabling issues.", 32, 104, fontColorWhite, fontColorWhiteHighlight);

				drawTextwHighlight("PSG has 200hz, 2khz and 4khz", 32, 120, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("tones on its channels and white", 32, 128, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("noise at 500hz.", 32, 136, fontColorWhite, fontColorWhiteHighlight);
				break;

		case HELP_LED:
			switch (page)
			{
				case 1:
					drawTextwHighlight("BACKLIT TEST", 106, 35, fontColorGreen, fontColorGreenHighlight);
					drawTextwHighlight("This test allows you to check", 32, 57, fontColorWhite, fontColorWhiteHighlight);
					drawTextwHighlight("how the display's backlit works", 32, 65, fontColorWhite, fontColorWhiteHighlight);
					drawTextwHighlight("when only a small array of", 32, 73, fontColorWhite, fontColorWhiteHighlight);
					drawTextwHighlight("pixels is shown.", 32, 81, fontColorWhite, fontColorWhiteHighlight);

					drawTextwHighlight("The user can move around the", 32, 97, fontColorWhite, fontColorWhiteHighlight);
					drawTextwHighlight("white pixel arrays with the", 32, 105, fontColorWhite, fontColorWhiteHighlight);
					drawTextwHighlight("d-pad, and change the size of", 32, 113, fontColorWhite, fontColorWhiteHighlight);
					drawTextwHighlight("the pixel array with 'A'.", 32, 121, fontColorWhite, fontColorWhiteHighlight);
					drawTextwHighlight("The 'B' button allows the user", 32, 129, fontColorWhite, fontColorWhiteHighlight);
					drawTextwHighlight("to hide the pixel array in", 32, 137, fontColorWhite, fontColorWhiteHighlight);
					drawTextwHighlight("order to alternate a fully black", 32, 145, fontColorWhite, fontColorWhiteHighlight);
					drawTextwHighlight("screen.", 32, 153, fontColorWhite, fontColorWhiteHighlight);
					break;
			}
			break;

			case HELP_LAG:
				switch (page)
				{
					case 1:
						drawTextwHighlight("LAG TEST (1/2)", 110, 35, fontColorGreen, fontColorGreenHighlight);
						drawTextwHighlight("This test is designed to be used", 32, 57, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("with two displays conected at", 32, 65, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("the same time. One being a CRT,", 32, 73, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("or a display with a known lag as", 32, 81, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("reference, and the other the", 32, 89, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("display to test.", 32, 97, fontColorWhite, fontColorWhiteHighlight);

						drawTextwHighlight("Using a camera, a picture should", 32, 114, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("be taken of both screens at the", 32, 122, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("same time. The picture will show", 32, 130, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("the frame discrepancy between", 32, 138, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("them.", 32, 146, fontColorWhite, fontColorWhiteHighlight);

						drawTextwHighlight("The circles in the bottom help", 32, 163, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("determine the frame even when", 32, 171, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("(cont...)", 207, 178, fontColorWhite, fontColorWhiteHighlight);
						break;

					case 2:
						drawTextwHighlight("LAG TEST (2/2)", 110, 35, fontColorGreen, fontColorGreenHighlight);
						drawTextwHighlight("the numbers are blurry.", 32, 57, fontColorWhite, fontColorWhiteHighlight);

						drawTextwHighlight("This version of the suite can be", 32, 74, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("used with a Sega Nomad as the", 32, 82, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("reference display.", 32, 90, fontColorWhite, fontColorWhiteHighlight);

						drawTextwHighlight("You can also split the video", 32, 107, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("signal and feed both displays.", 32, 115, fontColorWhite, fontColorWhiteHighlight);

						drawTextwHighlight("The vertical bars on the sides", 32, 132, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("change color each frame to help", 32, 140, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("when using LCD photos.", 32, 148, fontColorWhite, fontColorWhiteHighlight);

						drawTextwHighlight("Press A to start/stop, B to", 32, 165, fontColorWhite, fontColorWhiteHighlight);
						drawTextwHighlight("reset and C for B&W test.", 32, 173, fontColorWhite, fontColorWhiteHighlight);
						break;
				}
				break;

			case HELP_ALTERNATE:
				drawTextwHighlight("ALTERNATE 240p/480i", 125, 35, fontColorGreen, fontColorGreenHighlight);
				drawTextwHighlight("Some devices have a delay when", 32, 57, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("the source changes between", 32, 65, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("resolutions, which happens in", 32, 73, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("some games. This test allows to", 32, 81, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("time that delay manually.", 32, 89, fontColorWhite, fontColorWhiteHighlight);

				drawTextwHighlight("Press A to switch the", 32, 106, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("resolution; press it again when", 32, 114, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("you are able to see the screen", 32, 122, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("back in your display.", 32, 130, fontColorWhite, fontColorWhiteHighlight);
				break;

			case HELP_VIDEO:
				drawTextwHighlight("VIDEO OPTIONS", 125, 35, fontColorGreen, fontColorGreenHighlight);
				drawTextwHighlight("Here you can enable interlaced", 32, 57, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("(480i) mode for the whole suite", 32, 65, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("and disable horizontal 256 in", 32, 73, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("compatible screens.", 32, 81, fontColorWhite, fontColorWhiteHighlight);

				drawTextwHighlight("For PAL consoles, you can also", 32, 98, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("enable full 240p, instead of", 32, 106, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("224p.", 32, 114, fontColorWhite, fontColorWhiteHighlight);
				break;

			case HELP_AUDIOSYNC:
				drawTextwHighlight("AUDIO SYNC", 125, 35, fontColorGreen, fontColorGreenHighlight);
				drawTextwHighlight("This test flashes the whole", 32, 57, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("screen white for 2 frames,", 32, 65, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("along with a 1khz test tone.", 32, 73, fontColorWhite, fontColorWhiteHighlight);

				drawTextwHighlight("You can verify the sync between", 32, 90, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("audio and video with recording", 32, 98, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("equipment or specialized", 32, 106, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("hardware.", 32, 114, fontColorWhite, fontColorWhiteHighlight);

				drawTextwHighlight("Press A to start/stop the test.", 32, 131, fontColorWhite, fontColorWhiteHighlight);
				break;

			case HELP_CONTRAST:
				drawTextwHighlight("PLUGE CONTRAST", 125, 35, fontColorGreen, fontColorGreenHighlight);
				drawTextwHighlight("This sub test fills the screen", 32, 57, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("with a pattern. By default it", 32, 65, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("is drawn in color.", 32, 73, fontColorWhite, fontColorWhiteHighlight);

				drawTextwHighlight("You can change palettes with", 32, 90, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("A to display a dark or white", 32, 98, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("pattern in order to check", 32, 106, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("contrast and brightness.", 32, 114, fontColorWhite, fontColorWhiteHighlight);

				drawTextwHighlight("The C button returns to PLUGE.", 32, 131, fontColorWhite, fontColorWhiteHighlight);
				break;

			case HELP_MEMVIEW:
				drawTextwHighlight("MEMORY VIEWER", 110, 35, fontColorGreen, fontColorGreenHighlight);
				drawTextwHighlight("This shows selected regions of", 32, 56, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("the main CPU memory map.", 32, 64, fontColorWhite, fontColorWhiteHighlight);

				drawTextwHighlight("The current address range is", 32, 80, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("shown in red at the right from", 32, 88, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("top to bottom.", 32, 96, fontColorWhite, fontColorWhiteHighlight);

				drawTextwHighlight("The left part of the screen", 32, 112, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("shows 0x1C0 bytes in hex.", 32, 120, fontColorWhite, fontColorWhiteHighlight);

				drawTextwHighlight("- Button A enabled CRC of the", 32, 136, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("current screen.", 32, 144, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("- Button B jumps to relevant", 32, 152, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("memory locations.", 32, 160, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("- Button C switches to ASCII", 32, 168, fontColorWhite, fontColorWhiteHighlight);
				drawTextwHighlight("mode.", 32, 176, fontColorWhite, fontColorWhiteHighlight);
				break;
		default:
			screenFadeOut(1);
			exit = 1;
			break;
	}
		if (pressedButton & SEGA_CTRL_RIGHT)
		{
			if (page + 1 <= totalpages)
				page++;
		}

		if (pressedButton & SEGA_CTRL_LEFT)
		{
			if (page - 1 > 0)
				page--;
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