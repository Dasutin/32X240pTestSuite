.text

| void clear_plane(int plane);
| clear the name table for plane A or B
        .global clear_plane
clear_plane:
        moveq   #0,d0
        move.l  #0x40000003,d1          /* VDP write VRAM at 0xC000 (scroll plane A) */
        movea.l 4(sp),a0                /* plane */
        mov.l   a0,d0
        cmpi.l  #1,d0
        bne.b   0f
        move.l  #0x60000003,d1          /* VDP write VRAM at 0xE000 (scroll plane B) */
0:
        moveq   #0,d0
        lea     0xC00000,a0
        move.w  #0x8F02,4(a0)           /* set INC to 2 */
        move.l  d1,4(a0)                /* write VRAM at plane start */
        move.w  #128*32-1,d1
1:
        move.w  d0,(a0)                 /* clear name pattern */
        dbra    d1,1b

        rts

| void hscroll_plane(int plane, int hscroll);
| set horizontall scroll for plane A or B
        .global hscroll_plane
hscroll_plane:
        move.l  4(sp),d0                /* plane number */

        move.l  #0x6C000002,d1          /* plane A HSCROLL */
        cmpi.l  #1,d0
        bne.b   0f
        move.l  #0x6C020002,d1          /* plane B HSCROLL */
0:
        move.l  8(sp),d0                /* hscroll */
        lea     0xC00000,a0
        move.w  #0x8F01,4(a0)           /* set INC to 1 */
        move.l  d1,4(a0)                /* write VRAM at hscroll table start */

        move.w	d0,(a0)	                /* write hscroll */
        rts

| void vscroll_plane(int plane, int hscroll);
| set vertical scroll for plane A or B
        .global vscroll_plane
vscroll_plane:
        move.l  4(sp),d0                /* plane number */

        move.l  #0x40000010,d1          /* plane A VSCROLL */
        cmpi.l  #1,d0
        bne.b   0f
        move.l  #0x40020010,d1          /* plane B VSCROLL */
0:
        move.l  8(sp),d0                /* vscroll amount */
        lea     0xC00000,a0
        move.l  d1,4(a0)                /* write to VSRAM */

        move.w  d0,(a0)                 /* write vscroll */
        rts

| void map_plane(int plane, int offset, int height, int ormask);
| set the name table and hscroll for plane B for bitmap mode
        .global map_plane
map_plane:
        moveq   #0,d0
        move.l  #0x40000003,d1          /* VDP write VRAM at 0xC000 (scroll plane A) */
        movea.l 4(sp),a0                /* plane */
        mov.l   a0,d0
        cmpi.l  #1,d0
        bne.b   0f
        move.l  #0x60000003,d1          /* VDP write VRAM at 0xE000 (scroll plane B) */
0:
        moveq   #0,d0
        movea.l 8(sp),a0
        move.w  a0,d0                   /* bitmap start */
        movea.l 12(sp),a0

        movem.l d2-d4,-(sp)
        moveq   #0,d3
        move.w  a0,d3                   /* bitmap height */

        moveq   #0,d4
        movea.l 28(sp),a0
        move.w  a0,d4                   /* OR mask for palette */

        lea     0xC00000,a0
        move.w  #0x8F02,4(a0)           /* set INC to 2 */
        move.l  d1,4(a0)                /* write VRAM at plane B start */
  
1:
        or      d4,d0                   /* palette */
        move.w  #39,d2
2:
        move.w  d0,(a0)                 /* name table entry set for next pattern */
        addq.w  #1,d0
        dbra    d2,2b

        subi.w	#40,d0
        or      d4,d0                   /* palette */
        move.w  #39,d2
3:
        move.w  d0,(a0)                 /* name table entry set for next pattern */
        addq.w  #1,d0
        dbra    d2,3b

        moveq	#23,d2
4:
        move.l  #0,(a0)                 /* clear entries to end of row */
        dbra	d2,4b
        dbra    d3,1b

        move.l  #0x6C000002,d1          /* VDP write VRAM at 0xAC00 (HSCROLL table) */
        move.l  d1,4(a0)                /* write VRAM at hscroll table start */
        move.w	#111,d1
5:
        move.l	#0x00000000,(a0)        /* scroll A = 0, scroll B = 0 */
        dbra	d1,5b

        move.w  #0x8B00,4(a0)           /* SCROLL the whole plane */
        move.w  #0x9003,4(a0)           /* scroll size 128x32 */

        movem.l (sp)+,d2-d4
        rts

| void set_vram(int offset, int val);
| store word to vram at offset
| entry: first arg = offset in vram
|        second arg = word to store
        .global set_vram
set_vram:
        lea     0xC00000,a1
        move.w  #0x8F02,4(a1)           /* set INC to 2 */
        move.l  4(sp),d1                /* vram offset */
        lsl.l   #2,d1
        lsr.w   #2,d1
        swap    d1
        ori.l   #0x40000000,d1          /* VDP write VRAM */
        move.l  d1,4(a1)                /* write VRAM at offset*/
        move.l  8(sp),d0                /* data word */
        move.w  d0,(a1)                 /* set vram word */
        rts

| void next_vram(int val);
| store word to vram at next offset
| entry: first arg = word to store
        .global next_vram
next_vram:
        move.l  4(sp),d0                /* data word */
        move.w  d0,0xC00000             /* set vram word */
        rts

| void set_palette(short *pal, int start, int count)
| copy count entries pointed to by pal into the palette starting at the index start
| entry: pal = pointer to an array of words holding the colors
|        start = index of the first color in the palette to set
|        count = number of colors to copy
        .global set_palette
set_palette:
        movea.l 4(sp),a0                /* pal */
        move.l  8(sp),d0                /* start */
        move.l  12(sp),d1               /* count */
        add.w   d0,d0                   /* start*2 */
        swap    d0                      /* high word holds address */
        ori.l   #0xC0000000,d0          /* write CRAM address (0 + index*2) */
        subq.w  #1,d1                   /* for dbra */

        lea     0xC00000,a1
        move.w  #0x8F02,4(a1)           /* set INC to 2 */
        move.l  d0,4(a1)                /* write CRAM */
0:
        move.w  (a0)+,(a1)              /* copy color to palette */
        dbra    d1,0b
        rts

