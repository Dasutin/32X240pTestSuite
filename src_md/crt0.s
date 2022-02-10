#-------------------------------------
# SEGA 32X Support Code for the 68000
# by Chilly Willy
#-------------------------------------

        .text

# 0x880800 - Entry point for reset/cold-start

        .global _start

_start:
        moveq   #0,d0                   /* Clear Work RAM */
        move.w  #0x3FFF,d1
        suba.l  a1,a1
1:
        move.l  d0,-(a1)
        dbra    d1,1b

# Copy initialized variables from ROM to Work RAM
        lea     __text_end,a0
        move.w  #__data_size,d0
        lsr.w   #1,d0
        subq.w  #1,d0
2:
        move.w  (a0)+,(a1)+
        dbra    d0,2b

        lea     __stack,sp              /* Set stack pointer to top of Work RAM */

        bsr     init_hardware           /* Initialize the console hardware */

        jsr     main                    /* Call program main() */
3:
        stop    #0x2700
        bra.b   3b

        .align  64

# 0x880840 - 68000 General exception handler

        move.l  d0,-(sp)
        move.l  4(sp),d0            /* Jump table return address */
        sub.w   #0x206,d0           /* 0 = BusError, 6 = AddrError, etc */

/* Handle exception here */

        move.l  (sp)+,d0
        addq.l  #4,sp               /* Pop jump table return address */
        rte

        .align  64

# 0x880880 - 68000 Level 4 interrupt handler - HBlank IRQ

        rte

        .align  64

# 0x8808C0 - 68000 Level 6 interrupt handler - VBlank IRQ

        move.l  d0,-(sp)
        move.l  vblank,d0
        beq.b   1f
        move.l  a0,-(sp)
        movea.l d0,a0
        jmp     (a0)
1:
        move.l  (sp)+,d0
        rte

# Initialize the MD side to a known state for the game

init_hardware:
        lea     0xC00004,a0
        lea	0xC00000,a1
        move.w  #0x8114,(a0)            /* Display off, VBlank disabled */
        move.w  (a0),d0                 /* Read VDP Status reg */

# Initialize Joyports
        move.b  #0x40,0xA10009
        move.b  #0x40,0xA1000B
        move.b  #0x40,0xA10003
        move.b  #0x40,0xA10005

# Initialize Mega Drive VDP
        move.w  #0x8004,(a0) /* Reg  0 - Disable HBL INT */
        move.w  #0x8174,(a0) /* Reg  1 - Enable Display, VBL INT, DMA + 28 VCell Size */
        move.w  #0x8230,(a0) /* Reg  2 - Plane A =$30*$400=$C000 */
        move.w  #0x832C,(a0) /* Reg  3 - Window  =$2C*$400=$B000 */
        move.w  #0x8407,(a0) /* Reg  4 - Plane B =$7*$2000=$E000 */
        move.w  #0x8554,(a0) /* Reg  5 - Sprite Table begins at $BC00=$5E*$200 */
        move.w  #0x8600,(a0) /* Reg  6 - Not used */
        move.w  #0x8700,(a0) /* Reg  7 - Background Color number*/
        move.w  #0x8800,(a0) /* Reg  8 - Not used */
        move.w  #0x8900,(a0) /* Reg  9 - Not used */
        move.w  #0x8A00,(a0) /* Reg 10 - HInterrupt timing */
        move.w  #0x8B00,(a0) /* Reg 11 - $0000abcd A=Extr.int B=VSCR CD=HSCR */
        move.w  #0x8C81,(a0) /* Reg 12 - HCell Mode + Shadow/Highight + Interlaced Mode (40 Cell, No Shadow, No Interlace) */
        move.w  #0x8D2B,(a0) /* Reg 13 - HScroll Table = $B800 */
        move.w  #0x8E00,(a0) /* Reg 14 - Not used */
        move.w  #0x8F01,(a0) /* Reg 15 - Auto increment data */
        move.w  #0x9001,(a0) /* Reg 16 - Scroll Screen V&H Size (64x64) */
        move.w  #0x9100,(a0) /* Reg 17 - Window HPOS */
        move.w  #0x9200,(a0) /* Reg 18 - Window VPOS */

        move.w  #0,0xA15128             /* Controller Port 1 */
        move.w  #0,0xA1512A             /* Controller Port 2 */

        jsr     load_font

# Look for mouse
        lea     0xA10003,a0
0:
        jsr     get_mky
        cmpi.l  #-2,d0
        beq.b   0b                      /* Timeout */
        cmpi.l  #-1,d0
        beq.b   1f                      /* No mouse */
        move.w  #0xF001,0xA15128        /* Mouse in Port 1 */
1:
        lea     2(a0),a0
2:
        jsr     get_mky
        cmpi.l  #-2,d0
        beq.b   2b                      /* Timeout */
        cmpi.l  #-1,d0
        beq.b   3f                      /* No mouse */
        move.w  #0xF001,0xA1512A        /* Mouse in Port 2 */
3:

# Allow the 68k to access the FM chip
        move.w  #0x0100,0xA11100        /* Z80 assert bus request */
        move.w  #0x0100,0xA11200        /* Z80 deassert reset */

# Wait on Mars side
        move.b  #0,0xA15107             /* Clear RV - Allow SH2 to access ROM */
0:
        cmp.l   #0x4D5F4F4B,0xA15120    /* M_OK */
        bne.b   0b                      /* Wait for Primary SH2 OK */
1:
        cmp.l   #0x535F4F4B,0xA15124    /* S_OK */
        bne.b   1b                      /* Wait for Secondary OK */

        move.l  #vert_blank,vblank      /* Set Vertical Blank interrupt handler */
        move.w  #0x2000,sr              /* Enable interrupts */
        rts


# void write_byte(void *dst, unsigned char val)
        .global write_byte
write_byte:
        movea.l 4(sp),a0
        move.l  8(sp),d0
        move.b  d0,(a0)
        rts

# void write_word(void *dst, unsigned short val)
        .global write_word
write_word:
        movea.l 4(sp),a0
        move.l  8(sp),d0
        move.w  d0,(a0)
        rts

# void write_long(void *dst, unsigned int val)
        .global write_long
write_long:
        movea.l 4(sp),a0
        move.l  8(sp),d0
        move.l  d0,(a0)
        rts

# unsigned char read_byte(void *src)
        .global read_byte
read_byte:
        movea.l 4(sp),a0
        move.b  (a0),d0
        rts

# unsigned short read_word(void *src)
        .global read_word
read_word:
        movea.l 4(sp),a0
        move.w  (a0),d0
        rts

# unsigned int read_long(void *src)
        .global read_long
read_long:
        movea.l 4(sp),a0
        move.l  (a0),d0
        rts


        .data

# Put remaining code in data section to lower bus contention for the rom.

        .global do_main
do_main:
        move.b  #1,0xA15107         /* Set RV */
        move.b  #2,0xA130F1         /* SRAM disabled, write protected */
        move.b  #0,0xA15107         /* Clear RV */

        move.w  0xA15100,d0
        or.w    #0x8000,d0
        move.w  d0,0xA15100         /* Set FM - allow SH2 access to MARS hardware */
        move.l  #0,0xA15120         /* Let Primary SH2 run */

        lea     vert_blank(pc),a0
        move.l  a0,0xFF0FFC         /* Set vertical blank interrupt handler */
        move.w  #0x2000,sr          /* Enable interrupts */

main_loop:
        move.w  0xA15120,d0         /* Get COMM0 */
        bne.b   handle_req

        nop
        bra.b   main_loop

# Process request from Primary SH2
handle_req:
        cmpi.w  #0x01FF,d0
        bls     read_sram
        cmpi.w  #0x02FF,d0
        bls     write_sram
        cmpi.w  #0x03FF,d0
        bls     start_music
        cmpi.w  #0x04FF,d0
        bls     stop_music
        cmpi.w  #0x05FF,d0
        bls     read_mouse
        cmpi.w  #0x06FF,d0
        bls     clear_screen
        cmpi.w  #0x07FF,d0
        bls     set_offset
        cmpi.w  #0x08FF,d0
        bls     set_ntable
        cmpi.w  #0x09FF,d0
        bls     set_vram
        cmpi.w  #0x0AFF,d0
        bls     set_pal
        cmpi.w  #0x0BFF,d0
        bls     set_color

# Unknown command
        move.w  #0,0xA15120         /* Done */
        bra.b   main_loop

read_sram:
        move.w  #0x2700,sr          /* Disable ints */
        moveq   #0,d1
        moveq   #0,d0
        move.w  0xA15122,d0         /* COMM2 holds offset */
        lea     0x200000,a0
        move.b  #1,0xA15107         /* Set RV */
        move.b  #3,0xA130F1         /* SRAM enabled, write protected */
        move.b  1(a0,d0.l),d1       /* Read SRAM */
        move.b  #2,0xA130F1         /* SRAM disabled, write protected */
        move.b  #0,0xA15107         /* Clear RV */
        move.w  d1,0xA15122         /* COMM2 holds return byte */
        move.w  #0,0xA15120         /* Done */
        move.w  #0x2000,sr          /* Enable ints */
        bra     main_loop

write_sram:
        move.w  #0x2700,sr          /* Disable ints */
        moveq   #0,d1
        move.w  0xA15122,d1         /* COMM2 holds offset */
        lea     0x200000,a0
        move.b  #1,0xA15107         /* Set RV */
        move.b  #1,0xA130F1         /* SRAM enabled, write enabled */
        move.b  d0,1(a0,d1.l)       /* Write SRAM */
        move.b  #2,0xA130F1         /* SRAM disabled, write protected */
        move.b  #0,0xA15107         /* Clear RV */
        move.w  #0,0xA15120         /* Done */
        move.w  #0x2000,sr          /* Enable ints */
        bra     main_loop

set_rom_bank:
        move.l  a0,d3
        swap    d3
        lsr.w   #4,d3
        andi.w  #3,d3
        move.w  d3,0xA15104         /* Set ROM bank select */
        move.l  a0,d3
        andi.l  #0x0FFFFF,d3
        ori.l   #0x900000,d3
        movea.l d3,a1
        rts

start_music:
        tst.w   cd_ok
        beq.b   2f                  /* Could not init CD */
        tst.b   cd_ok
        bne.b   0f                  /* Disc found - Try to play track */
        /* check for CD */
10:
        move.b  0xA1200F,d1
        bne.b   10b                 /* Wait until Sub-CPU is ready to receive command */
        move.b  #'D,0xA1200E        /* Set main comm port to GetDiskInfo command */
11:
        move.b  0xA1200F,d0
        beq.b   11b                 /* Wait for acknowledge byte in sub comm port */
        move.b  #0x00,0xA1200E      /* Acknowledge receipt of command result */

        cmpi.b  #'D,d0
        bne.b   2f                  /* Could noy get disk info */
        move.w  0xA12020,d0         /* BIOS status */
        cmpi.w  #0x1000,d0
        bhs.b   2f                  /* Open, Busy, or No Disc */
        move.b  #1,cd_ok            /* Disc detected - Try to play track */
0:
        move.b  0xA1200F,d1
        bne.b   0b                  /* Wait until Sub-CPU is ready to receive command */

        move.b  d0,0xA12012         /* Repeat flag */
        move.w  0xA15122,d0
        addq.w  #1,d0
        move.w  d0,0xA12010         /* Track no. */
        move.b  #'P,0xA1200E        /* Set main comm port to PlayTrack command */
1:
        move.b  0xA1200F,d0
        beq.b   1b                  /* Wait for acknowledge byte in sub COMM port */
        move.b  #0x00,0xA1200E      /* Acknowledge receipt of command result */
2:
        move.w  #0,0xA15120         /* Done */
        bra     main_loop

stop_music:
        tst.w   cd_ok
        beq.b   2f
0:
        move.b  0xA1200F,d1
        bne.b   0b                  /* Wait until Sub-CPU is ready to receive command */

        move.b  #'S,0xA1200E        /* Set main comm port to StopPlayback command */
1:
        move.b  0xA1200F,d0
        beq.b   1b                  /* Wait for acknowledge byte in Sub COMM port */
        move.b  #0x00,0xA1200E      /* Acknowledge receipt of command result */
2:
        move.w  #0,0xA15120         /* Done */
        bra     main_loop

read_mouse:
        tst.b   d0
        bne.b   1f                  /* Skip controller Port 1 */

        move.w  0xA15128,d0
        andi.w  #0xF001,d0
        cmpi.w  #0xF001,d0
        bne.b   1f                  /* No mouse in controller Port 1 */
        lea     0xA10003,a0
        bsr     get_mky
        bset    #31,d0
        move.w  d0,0xA15122
        swap    d0
        move.w  d0,0xA15120
0:
        move.w  0xA15120,d0
        bne.b   0b                  /* Wait for SH2 to read mouse value */
        bra     main_loop
1:
        move.w  0xA1512A,d0
        andi.w  #0xF001,d0
        cmpi.w  #0xF001,d0
        bne.b   3f                  /* No mouse in controller Port 2 */
        lea     0xA10005,a0
        bsr     get_mky
        bset    #31,d0
        move.w  d0,0xA15122
        swap    d0
        move.w  d0,0xA15120
2:
        move.w  0xA15120,d0
        bne.b   2b                  /* Wait for SH2 to read mouse value */
        bra     main_loop
3:
        move.l  #-1,d0              /* No mouse */
        move.w  d0,0xA15122
        swap    d0
        move.w  d0,0xA15120
4:
        move.w  0xA15120,d0
        bne.b   4b                  /* Wait for SH2 to read mouse value */
        bra     main_loop

clear_screen:
        moveq   #0,d0
        lea     0xC00000,a0
        move.w  #0x8F02,4(a0)       /* Set INC to 2 */
        move.l  #0x60000003,d1      /* VDP write VRAM at 0xE000 (Scroll Plane B) */
        move.l  d1,4(a0)            /* Write VRAM at Plane B start */
        move.w  #64*32-1,d1
1:
        move.w  d0,(a0)             /* Clear name pattern */
        dbra    d1,1b
        move.w  #0,0xA15120         /* Done */
        bra     main_loop

set_offset:
        moveq   #0,d7
        move.w  0xA15122,d7         /* Offset */
        move.w  #0,0xA15120         /* Done */
        bra     main_loop

set_ntable:
        lea     0xC00000,a1
        move.w  #0x8F02,4(a1)       /* Set INC to 2 */
        move.l  d7,d2               /* Name Table offset */
        swap    d2
        ori.l   #0x60000003,d2      /* OR cursor with VDP write VRAM at 0xE000 (Scroll Plane B) */
        move.l  d2,4(a1)            /* Write VRAM at location of cursor in Plane B */
        move.w  0xA15122,d0         /* Pattern name */
        move.w  d0,(a1)             /* Set pattern name for character */
        addq.l  #2,d7               /* Increment offset */
        move.w  #0,0xA15120         /* Done */
        bra     main_loop

set_vram:
        lea     0xC00000,a1
        move.w  #0x8F02,4(a1)       /* Set INC to 2 */
        move.l  d7,d2               /* VRAM offset */
        swap    d2
        rol.l   #2,d2
        lsl.w   #2,d2
        lsr.l   #2,d2
        ori.l   #0x40000000,d2      /* VDP write VRAM */
        move.l  d2,4(a1)            /* Write VRAM at location of cursor in Plane B */
        move.w  0xA15122,d0         /* Data word */
        move.w  d0,(a1)             /* Set VRAM word */
        addq.l  #2,d7               /* Increment offset */
        move.w  #0,0xA15120         /* Done */
        bra     main_loop

set_pal:
        lea     0xC00004,a0
        lea     0xC00000,a1
        moveq   #0,d0
        moveq   #0,d4
        move.w  0xA15122,d4         /* Grab data word */
        cmpi.w  #0x0100,d4
        bls     set_pal0
        cmpi.w  #0x0200,d4
        bls     set_pal1
        cmpi.w  #0x0300,d4
        bls     set_pal2
        cmpi.w  #0x0400,d4
        bls     set_pal3
set_pal0:
        move.l  #0xC0000000,(a0)
        move.w  #0,0xA15120         /* Done */
        bra     main_loop    
set_pal1:
        move.l  #0xC0200000,(a0)
        move.w  #0,0xA15120         /* Done */
        bra     main_loop
set_pal2:
        move.l  #0xC0400000,(a0)
        move.w  #0,0xA15120         /* Done */
        bra     main_loop
set_pal3:
        move.l  #0xC0600000,(a0)
        move.w  #0,0xA15120         /* Done */
        bra     main_loop
set_color:
        moveq   #0,d0
        moveq   #0,d6
        moveq   #0,d5
        move.w  0xA15122,d6         /* Grab data word */
        cmpi.w  #0x0100,d6
        bls     set_color_gray
        cmpi.w  #0x0200,d6
        bls     set_color_red
        cmpi.w  #0x0300,d6
        bls     set_color_green
        cmpi.w  #0x0400,d6
        bls     set_color_blue
set_color_gray:
        lea     pal_gray_data,a2
        bra     setting_color
set_color_red:
        lea     pal_red_data,a2
        bra     setting_color
set_color_green:
        lea     pal_green_data,a2
        bra     setting_color
set_color_blue:
        lea     pal_blue_data,a2
        bra     setting_color
setting_color:
        move.w  #15,d5
        bra     loop_color
loop_color:
        move.w  (a2)+,(a1)
        dbra    d5,loop_color
        move.w  #0,0xA15120         /* Done */
        bra     main_loop  

vert_blank:
        move.l  d1,-(sp)
        move.l  d2,-(sp)

        /* read controllers */
        move.w  0xA15128,d0
        andi.w  #0xF000,d0
        cmpi.w  #0xF000,d0
        beq.b   0f                  /* No controller or mouse in Port 1 */
        lea     0xA10003,a0
        bsr.b   get_pad
        move.w  d2,0xA15128         /* Controller 1 current value */
0:
        move.w  0xA1512A,d0
        andi.w  #0xF000,d0
        cmpi.w  #0xF000,d0
        beq.b   1f                  /* No controller or mouse in Port 2 */
        lea     0xA10005,a0
        bsr.b   get_pad
        move.w  d2,0xA1512A         /* Controller 2 current value */
1:
        tst.w   gen_lvl2
        beq.b   2f
        lea     0xA12000,a0
        move.w  (a0),d0
        ori.w   #0x0100,d0
        move.w  d0,(a0)             

2:
        move.l  (sp)+,d2
        move.l  (sp)+,d1
        movea.l (sp)+,a0
        move.l  (sp)+,d0
        rte

# get current pad value
# entry: a0 = pad control port
# exit:  d2 = pad value (0 0 0 1 M X Y Z S A C B R L D U) or (0 0 0 0 0 0 0 0 S A C B R L D U)
get_pad:
        bsr.b   get_input       /* - 0 s a 0 0 d u - 1 c b r l d u */
        move.w  d0,d1
        andi.w  #0x0C00,d0
        bne.b   no_pad
        bsr.b   get_input       /* - 0 s a 0 0 d u - 1 c b r l d u */
        bsr.b   get_input       /* - 0 s a 0 0 0 0 - 1 c b m x y z */
        move.w  d0,d2
        bsr.b   get_input       /* - 0 s a 1 1 1 1 - 1 c b r l d u */
        andi.w  #0x0F00,d0      /* 0 0 0 0 1 1 1 1 0 0 0 0 0 0 0 0 */
        cmpi.w  #0x0F00,d0
        beq.b   common          /* Six button controller */
        move.w  #0x010F,d2      /* Three button controller */
common:
        lsl.b   #4,d2           /* - 0 s a 0 0 0 0 m x y z 0 0 0 0 */
        lsl.w   #4,d2           /* 0 0 0 0 m x y z 0 0 0 0 0 0 0 0 */
        andi.w  #0x303F,d1      /* 0 0 s a 0 0 0 0 0 0 c b r l d u */
        move.b  d1,d2           /* 0 0 0 0 m x y z 0 0 c b r l d u */
        lsr.w   #6,d1           /* 0 0 0 0 0 0 0 0 s a 0 0 0 0 0 0 */
        or.w    d1,d2           /* 0 0 0 0 m x y z s a c b r l d u */
        eori.w  #0x1FFF,d2      /* 0 0 0 1 M X Y Z S A C B R L D U */
        rts

no_pad:
        move.w  #0xF000,d2
        rts

# Read single phase from controller
get_input:
        move.b  #0x00,(a0)
        nop
        nop
        move.b  (a0),d0
        move.b  #0x40,(a0)
        lsl.w   #8,d0
        move.b  (a0),d0
        rts

# get current mouse value
# entry: a0 = mouse control port
# exit:  d0 = mouse value (0  0  0  0  0  0  0  0  YO XO YS XS S  M  R  L  X7 X6 X5 X4 X3 X2 X1 X0 Y7 Y6 Y5 Y4 Y3 Y2 Y1 Y0) or -2 (timeout) or -1 (no mouse)
get_mky:
        move.w  sr,d2
        move.w  #0x2700,sr      /* Disable INTs */

        move.b	#0x60,6(a0)     /* Set direction bits */
        nop
        nop
        move.b  #0x60,(a0)      /* First phase of mouse packet */
        nop
        nop
0:
        btst    #4,(a0)
        beq.b   0b              /* Wait on handshake */
        move.b  (a0),d0
        andi.b  #15,d0
        bne     mky_err         /* Not 0 means not mouse */

        move.b  #0x20,(a0)      /* Next phase */
        move.w  #254,d1         /* Number retries before timeout */
1:
        btst    #4,(a0)
        bne.b   2f              /* Handshake */
        dbra    d1,1b
        bra     timeout_err
2:
        move.b  (a0),d0
        andi.b  #15,d0
        move.b  #0,(a0)         /* Next phase */
        cmpi.b  #11,d0
        bne     mky_err         /* Not 11 means not mouse */
3:
        btst    #4,(a0)
        beq.b   4f              /* Handshake */
        dbra    d1,3b
        bra     timeout_err
4:
        move.b  (a0),d0         /* Specs say should be 15 */
        nop
        nop
        move.b  #0x20,(a0)      /* Next phase */
        nop
        nop
5:
        btst    #4,(a0)
        bne.b   6f
        dbra    d1,5b
        bra     timeout_err
6:
        move.b  (a0),d0         /* Specs say should be 15 */
        nop
        nop
        move.b  #0,(a0)         /* Next phase */
        moveq   #0,d0           /* Clear reg to hold packet */
        nop
7:
        btst    #4,(a0)
        beq.b   8f              /* Handshake */
        dbra    d1,7b
        bra     timeout_err
8:
        move.b  (a0),d0         /* YO XO YS XS */
        move.b  #0x20,(a0)      /* Next phase */
        lsl.w   #8,d0           /* Save nibble */
9:
        btst    #4,(a0)
        bne.b   10f             /* Handshake */
        dbra    d1,9b
        bra     timeout_err
10:
        move.b  (a0),d0         /* S  M  R  L */
        move.b  #0,(a0)         /* Next phase */
        lsl.b   #4,d0           /* YO XO YS XS S  M  R  L  0  0  0  0 */
        lsl.l   #4,d0           /* YO XO YS XS S  M  R  L  0  0  0  0  0  0  0  0 */
11:
        btst    #4,(a0)
        beq.b   12f             /* Handshake */
        dbra    d1,11b
        bra     timeout_err
12:
        move.b  (a0),d0         /* X7 X6 X5 X4 */
        move.b  #0x20,(a0)      /* Next phase */
        lsl.b   #4,d0           /* YO XO YS XS S  M  R  L  X7 X6 X5 X4 0  0  0  0 */
        lsl.l   #4,d0           /* YO XO YS XS S  M  R  L  X7 X6 X5 X4 0  0  0  0  0  0  0  0 */
13:
        btst    #4,(a0)
        bne.b   14f             /* Handshake */
        dbra    d1,13b
        bra     timeout_err
14:
        move.b  (a0),d0         /* X3 X2 X1 X0 */
        move.b  #0,(a0)         /* Next phase */
        lsl.b   #4,d0           /* YO XO YS XS S  M  R  L  X7 X6 X5 X4 X3 X2 X1 X0 0  0  0  0 */
        lsl.l   #4,d0           /* YO XO YS XS S  M  R  L  X7 X6 X5 X4 X3 X2 X1 X0 0  0  0  0  0  0  0  0 */
15:
        btst    #4,(a0)
        beq.b   16f             /* Handshake */
        dbra    d1,15b
        bra     timeout_err
16:
        move.b  (a0),d0         /* Y7 Y6 Y5 Y4 */
        move.b  #0x20,(a0)      /* Next phase */
        lsl.b   #4,d0           /* YO XO YS XS S  M  R  L  X7 X6 X5 X4 X3 X2 X1 X0 Y7 Y6 Y5 Y4 0  0  0  0 */
        lsl.l   #4,d0           /* YO XO YS XS S  M  R  L  X7 X6 X5 X4 X3 X2 X1 X0 Y7 Y6 Y5 Y4 0  0  0  0  0  0  0  0*/
17:
        btst    #4,(a0)
        beq.b   18f             /* Handshake */
        dbra    d1,17b
        bra     timeout_err
18:
        move.b  (a0),d0         /* Y3 Y2 Y1 Y0 */
        move.b  #0x60,(a0)      /* First phase */
        lsl.b   #4,d0           /* YO XO YS XS S  M  R  L  X7 X6 X5 X4 X3 X2 X1 X0 Y7 Y6 Y5 Y4 Y3 Y2 Y1 Y0 0  0  0  0 */
        lsr.l   #4,d0           /* YO XO YS XS S  M  R  L  X7 X6 X5 X4 X3 X2 X1 X0 Y7 Y6 Y5 Y4 Y3 Y2 Y1 Y0 */
19:
        btst    #4,(a0)
        beq.b   19b             /* Wait on handshake */

        move.w  d2,sr           /* Restore int status */
        rts

timeout_err:
        move.b  #0x60,(a0)      /* First phase */
        nop
        nop
0:
        btst    #4,(a0)
        beq.b   0b              /* Wait on handshake */

        move.w  d2,sr           /* Restore int status */
        moveq   #-2,d0
        rts

mky_err:
        move.b  #0x40,6(a0)             /* Set direction bits */
        nop
        nop
        move.b  #0x40,(a0)

        move.w  d2,sr                   /* Restore int status */
        moveq   #-1,d0
        rts

# Load font tile data
reload_font:
        lea     0xC00004,a0             /* VDP cmd/sts reg */
        lea     0xC00000,a1             /* VDP data reg */
load_font:
        lea     0xC00004,a0             /* VDP cmd/sts reg */
        lea     0xC00000,a1             /* VDP data reg */
        move.w  #0x8F02,(a0)            /* INC = 2 */
        move.l  #0x40000000,(a0)        /* Write VRAM address 0 */
        lea     font_data,a2
        move.w  #0x6B*8-1,d2
0:
        move.l  (a2)+,d0                /* Dont FG mask */
        move.l  d0,d1
        not.l   d1                      /* Font BG mask */
        andi.l  #0xFFFFFFFF,d0          /* Set font FG color */
        andi.l  #0x00000000,d1          /* Set font BG color */
        or.l    d1,d0
        move.l  d0,(a1)                 /* Set tile line */
        dbra    d2,0b

# Set the default palette for text
        move.l  #0xC0000000,0xC00004        /* Write CRAM address 0 PAL0 */
        move.w  #0x0000,0xC00000            /* PAL GRAY */
        move.w  #0x0222,0xC00000
        move.w  #0x0444,0xC00000
        move.w  #0x0666,0xC00000
        move.w  #0x0888,0xC00000
        move.w  #0x0AAA,0xC00000
        move.w  #0x0CCC,0xC00000
        move.w  #0x0EEE,0xC00000
        move.w  #0x0EEE,0xC00000
        move.w  #0x0EEE,0xC00000
        move.w  #0x0EEE,0xC00000
        move.w  #0x0EEE,0xC00000
        move.w  #0x0EEE,0xC00000
        move.w  #0x0EEE,0xC00000
        move.w  #0x0EEE,0xC00000
        move.w  #0x0EEE,0xC00000

        move.l  #0xC0200000,0xC00004       /* Write CRAM address 32 PAL1 */
        move.w  #0x0000,0xC00000           /* PAL RED */
        move.w  #0x0002,0xC00000
        move.w  #0x0004,0xC00000
        move.w  #0x0008,0xC00000
        move.w  #0x000A,0xC00000
        move.w  #0x000C,0xC00000
        move.w  #0x000E,0xC00000
        move.w  #0x000E,0xC00000
        move.w  #0x000E,0xC00000
        move.w  #0x000E,0xC00000
        move.w  #0x000E,0xC00000
        move.w  #0x000E,0xC00000
        move.w  #0x000E,0xC00000
        move.w  #0x000E,0xC00000
        move.w  #0x000E,0xC00000
        move.w  #0x000E,0xC00000       

        move.l  #0xC0400000,0xC00004        /* Write CRAM address 64 PAL2 */
        move.w  #0x0000,0xC00000            /* PAL GREEN */
        move.w  #0x0020,0xC00000
        move.w  #0x0040,0xC00000
        move.w  #0x0060,0xC00000
        move.w  #0x0080,0xC00000
        move.w  #0x00A0,0xC00000
        move.w  #0x00C0,0xC00000
        move.w  #0x00E0,0xC00000
        move.w  #0x00E0,0xC00000
        move.w  #0x00E0,0xC00000
        move.w  #0x00E0,0xC00000
        move.w  #0x00E0,0xC00000
        move.w  #0x00E0,0xC00000
        move.w  #0x00E0,0xC00000
        move.w  #0x00E0,0xC00000
        move.w  #0x00E0,0xC00000

        move.l  #0xC0600000,0xC00004        /* Write CRAM address 64 PAL3 */
        move.w  #0x0000,0xC00000            /* PAL BLUE */
        move.w  #0x0200,0xC00000
        move.w  #0x0400,0xC00000
        move.w  #0x0600,0xC00000
        move.w  #0x0800,0xC00000
        move.w  #0x0A00,0xC00000
        move.w  #0x0C00,0xC00000
        move.w  #0x0E00,0xC00000
        move.w  #0x0E00,0xC00000
        move.w  #0x0E00,0xC00000
        move.w  #0x0E00,0xC00000
        move.w  #0x0E00,0xC00000
        move.w  #0x0E00,0xC00000
        move.w  #0x0E00,0xC00000
        move.w  #0x0E00,0xC00000
        move.w  #0x0E00,0xC00000

        move.w  #0x8174,(a4)            /* Display on, VBlank enabled */
        rts

# Global variables for 68000

        .align  4

vblank:
        dc.l    0

        .global gen_lvl2
gen_lvl2:
        dc.w    0

        .global cd_ok
cd_ok:
        dc.w    0

        .global pal_1
pal_1:
        dc.l    0xC0000000

        .global pal_2
pal_2:
        dc.l    0xC0200000

        .global pal_3
pal_3:
        dc.l    0xC0400000

        .global pal_4
pal_4:
        dc.l    0xC0600000

        .align  4
