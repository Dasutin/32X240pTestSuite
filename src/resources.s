.section .text

.align 2

			.global _GRID_PALETTE_DATA
_GRID_PALETTE_DATA:	
			.incbin "res/grid_pal.bin"

			.align 2

			.global _GRID_GRAY_PALETTE_DATA
_GRID_GRAY_PALETTE_DATA:
			.incbin "res/gridgray_pal.bin"

			.align 2

			.global _COLORREF_PALETTE_DATA
_COLORREF_PALETTE_DATA:
			.incbin "res/colorref_pal.bin"

			.align 2

			.global _SHARPNESS_PALETTE_DATA
_SHARPNESS_PALETTE_DATA:
			.incbin "res/sharpness_pal.bin"

			.align 2

			.global _SHARPNESS_BRICK_PALETTE_DATA
_SHARPNESS_BRICK_PALETTE_DATA:
			.incbin "res/sharpbrick_pal.bin"

			.align 2

			.global _COLORBLEED_PALETTE_DATA
_COLORBLEED_PALETTE_DATA:
			.incbin "res/colorbleed_pal_a.bin"

			.align 2

			.global _GRAYRAMP_PALETTE_DATA
_GRAYRAMP_PALETTE_DATA:
			.incbin "res/grayramp_pal.bin"

			.align 2

			.global _PLUGE_NTSC_PALETTE_DATA
_PLUGE_NTSC_PALETTE_DATA:
			.incbin "res/plugentsc_pal.bin"

			.align 2

			.global _PLUGE_RGB_PALETTE_DATA
_PLUGE_RGB_PALETTE_DATA:
			.incbin "res/plugergb_pal.bin"

			.align 2

			.global _HSTRIPES_PALETTE_DATA
_HSTRIPES_PALETTE_DATA:
			.incbin "res/horizontalstripes_pal.bin"

			.align 2

			.global _SMPTE100IRE_PALETTE_DATA
_SMPTE100IRE_PALETTE_DATA:
			.incbin "res/smpte_100_ire_pal.bin"

			.align 2

			.global _SMPTE75IRE_PALETTE_DATA
_SMPTE75IRE_PALETTE_DATA:
			.incbin "res/smpte_75_ire_pal.bin"

			.align 2

			.global _BACKGROUND_PALETTE_DATA
_BACKGROUND_PALETTE_DATA:
			.incbin "res/background_pals.bin"

			.align 2

			.global _FONT_HIGHLIGHT_FONT_PALETTE_DATA
_FONT_HIGHLIGHT_FONT_PALETTE_DATA:
			.incbin "res/240pfont_highlight_pal.bin"

			.align 2

			.global _CONVERGENCE_GRID_PALETTE_DATA
_CONVERGENCE_GRID_PALETTE_DATA:
			.incbin "res/convergence_grid_pal.bin"

			.align 2

			.global _MOTOKO_PALETTE_DATA
_MOTOKO_PALETTE_DATA:
			.incbin "res/motoko_pal.bin"

			.align 2

			.global _BEE_PALETTE_DATA
_BEE_PALETTE_DATA:
			.incbin "res/bee_pal.bin"

			.align 2

			.global _TEST_PALETTE
_TEST_PALETTE:
			.incbin "res/motoko_pal.bin"
			.incbin "res/bee_pal.bin"

			.align 2

			.global _SCREENSCROLL_COMBO_PALETTE
_SCREENSCROLL_COMBO_PALETTE:
			.incbin "res/screenscroll_pal.bin"
			.incbin "res/screenscroll_background_1_pal.bin"

			.align 2

			.global _BACKGROUND_W_GILLIAN_PALETTE
_BACKGROUND_W_GILLIAN_PALETTE:
			.incbin "res/background_w_gillian_pal.bin"

			.align 2

			.global _MARKER_PALETTE
_MARKER_PALETTE:
			.incbin "res/marker_pal.bin"

			.align 2

			.global _MARKER_STRIPED_PALETTE
_MARKER_STRIPED_PALETTE:
			.incbin "res/marker_striped_pal.bin"

			.align 2

			.global _CONVERGENCE_COLOR_PALETTE
_CONVERGENCE_COLOR_PALETTE:
			.incbin "res/convergence_color_pal.bin"

			.align 2

			.global _CONVERGENCE_COLOR_BORDER_PALETTE
_CONVERGENCE_COLOR_BORDER_PALETTE:
			.incbin "res/convergence_color_border_pal.bin"

			.align 2

			.global _SCREENSCROLL_PALETTE
_SCREENSCROLL_PALETTE:
			.incbin "res/screenscroll_pal.bin"

			.align 2

			.global _SCREENSCROLL_TREES_TEST_PALETTE
_SCREENSCROLL_TREES_TEST_PALETTE:
			.incbin "res/screenscroll_trees_test_pal.bin"

			.align 2

			.global _SCREENSCROLL_BACKGROUND_PALETTE
_SCREENSCROLL_BACKGROUND_PALETTE:
			.incbin "res/screenscroll_background_1_pal.bin"

			.align 2

			.global _COLORCHART_PALETTE_DATA
_COLORCHART_PALETTE_DATA:
			.incbin "res/colorchart_pal.bin"

			.align 2

			.global _EBU_COLORBARS_PALETTE_DATA
 _EBU_COLORBARS_PALETTE_DATA:
			.incbin "res/ebu_colorbars_pal.bin"

			.align 2

			.global _GRID_PATTERN
_GRID_PATTERN:
			.incbin "res/grid_tile.bin"

			.align 2

			.global _COLORREF_PATTERN
_COLORREF_PATTERN:
			.incbin "res/colorref_tile.bin"

			.align 2

			.global _SHARPNESS_PATTERN
_SHARPNESS_PATTERN:
			.incbin "res/sharpness_tile.bin"

			.align 2

			.global _SHARPNESS_BRICK_PATTERN
_SHARPNESS_BRICK_PATTERN:
			.incbin "res/sharpbrick_tile.bin"

			.align 2

			.global _COLORBLEED_PATTERN
_COLORBLEED_PATTERN:
			.incbin "res/colorbleed_tile.bin"

			.align 2

			.global _COLORBLEED_CHECKERBOARD_TILE
_COLORBLEED_CHECKERBOARD_TILE:
			.incbin "res/colorbleed_checkerboard_tile.bin"

			.align 2

			.global _GRAYRAMP_PATTERN
_GRAYRAMP_PATTERN:
			.incbin "res/grayramp_tile.bin"

			.align 2

			.global _PLUGE_PATTERN
_PLUGE_PATTERN:
			.incbin "res/pluge_tile.bin"

			.align 2

			.global _HSTRIPES_PATTERN
_HSTRIPES_PATTERN:
			.incbin "res/horizontalstripes_tile.bin"

			.align 2

			.global _SMPTE100IRE_PATTERN
_SMPTE100IRE_PATTERN:
			.incbin "res/smpte_100_ire_tile.bin"

			.align 2

.global _SMALL_FONT

			.global _BACKGROUND_TILE
_BACKGROUND_TILE:
			.incbin "res/background_tiles.bin"

			.align 2

			.global _FONT_HIGHLIGHT_FONT
_FONT_HIGHLIGHT_FONT:
			.incbin "res/240pfont_highlight_tile.bin"

			.align 2

			.global _CONVERGENCE_GRID_TILE
_CONVERGENCE_GRID_TILE:
			.incbin "res/convergence_grid_tile.bin"

			.align 2

			.global _CONVERGENCE_STARS_TILE
_CONVERGENCE_STARS_TILE:
			.incbin "res/convergence_stars_tile.bin"

			.align 2

			.global _CONVERGENCE_DOTS_TILE
_CONVERGENCE_DOTS_TILE:
			.incbin "res/convergence_dots_tile.bin"

			.align 2

			.global _MOTOKO_PATTERN
_MOTOKO_PATTERN:
			.incbin "res/motoko_tile.bin"

			.align 2

			.global _BEE_PATTERN
_BEE_PATTERN:
			.incbin "res/bee_tile.bin"

			.align 2

			.global _BACKGROUND_W_GILLIAN_TILE
_BACKGROUND_W_GILLIAN_TILE:
			.incbin "res/background_w_gillian_tile.bin"

			.align 2

			.global _MARKER_TILE
_MARKER_TILE:
			.incbin "res/marker_tile.bin"

			.align 2

			.global _MARKER_STRIPED_TILE
_MARKER_STRIPED_TILE:
			.incbin "res/marker_striped_tile.bin"

			.align 2

			.global _BEE_SHADOW_TILE
_BEE_SHADOW_TILE:
			.incbin "res/bee_shadow_tile.bin"

			.align 2

			.global	_MARKER_SHADOW_TILE
_MARKER_SHADOW_TILE:
			.incbin "res/marker_shadow.bin"

			.align 2

			.global	_H_STRIPES_SHADOW_TILE
_H_STRIPES_SHADOW_TILE:
			.incbin "res/h_stripes_shadow_test_tile.bin"

			.align 2

			.global	_CHECKERBOARD_SHADOW_TILE
_CHECKERBOARD_SHADOW_TILE:
			.incbin "res/checkerboard_shadow_test_tile.bin"

			.align 2

			.global	_CONVERGENCE_COLOR_TILE
_CONVERGENCE_COLOR_TILE:
			.incbin "res/convergence_color_tile.bin"

			.align 2

			.global	_CONVERGENCE_COLOR_BORDER_TILE
_CONVERGENCE_COLOR_BORDER_TILE:
			.incbin "res/convergence_color_border_tile.bin"

			.align 2

			.global _SCREENSCROLL_TILE
_SCREENSCROLL_TILE:
			.incbin "res/screenscroll_tile.bin"

			.align 2

			.global _SCREENSCROLL_TREES_TEST
_SCREENSCROLL_TREES_TEST_TILE:
			.incbin "res/screenscroll_trees_test_tile.bin"

			.align 2

			.global _SCREENSCROLL_BACKGROUND_TILE
_SCREENSCROLL_BACKGROUND_TILE:
			.incbin "res/screenscroll_background_1_tile.bin"

			.align 2

			.global _IRE_PATTERN
_IRE_PATTERN:
			.incbin "res/ire.bin"

			.align 2

			.global _TEST_SMTPE_PATTERN
_TEST_SMTPE_PATTERN:
			.incbin "res/test.bin"

			.align 2

			.global _COLORCHART_PATTERN
_COLORCHART_PATTERN:
			.incbin "res/colorchart_tile.bin"

			.align 2

			.global _MONOSCOPE_PATTERN
_MONOSCOPE_PATTERN:
			.incbin "res/monoscope_tile.bin"

			.align 2

			.global _EBU_COLORBARS_PATTERN
_EBU_COLORBARS_PATTERN:
			.incbin "res/ebu_colorbars_tile.bin"

			.align 2

audioFileName1:
			.asciz  "jump"
audioFileName2:
			.asciz  "beep"

			.align  4

audioFile1:
			.incbin "res/jump.wav"
audioFileEnd1:
			.align  4
audioFile2:
			.incbin "res/beep.wav"
audioFileEnd2:

			.align  4

			.global _audioFileName
_audioFileName:
			.long   audioFileName1
			.long   audioFileName2

			.global _audioFileSize
_audioFileSize:
			.long   audioFileEnd1 - audioFile1
			.long   audioFileEnd2 - audioFile2

			.global _audioFilePtr
_audioFilePtr:        
			.long   audioFile1
			.long   audioFile2

			.align  4
