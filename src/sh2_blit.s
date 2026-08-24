
    .section .data
    .align 4

/* r4 = dst, r5 = src, r6 = byte count */
    .global _sh2_blit8_copy_row
_sh2_blit8_copy_row:
    tst     r6,r6
    bt      .Lcopy_done

    /* Different source/destination parity cannot use aligned word accesses. */
    mov     r4,r0
    xor     r5,r0
    tst     #1,r0
    bf      .Lcopy_bytes

    /* If both pointers are odd, peel one byte and make both even. */
    mov     r4,r0
    tst     #1,r0
    bt      .Lcopy_longs
    mov.b   @r5+,r0
    add     #-1,r6
    mov.b   r0,@r4
    tst     r6,r6
    bt/s    .Lcopy_done
    add     #1,r4

.Lcopy_longs:
    mov     r6,r1
    shlr2   r1
    tst     r1,r1
    bt      .Lcopy_tail
.Lcopy_long_loop:
    mov.l   @r5+,r0
    add     #-4,r6
    mov.l   r0,@r4
    dt      r1
    bf/s    .Lcopy_long_loop
    add     #4,r4

.Lcopy_tail:
    mov     r6,r0
    tst     #2,r0
    bt      .Lcopy_last_byte
    mov.w   @r5+,r1
    add     #-2,r6
    mov.w   r1,@r4
    add     #2,r4
.Lcopy_last_byte:
    tst     r6,r6
    bt      .Lcopy_done
    mov.b   @r5,r0
    mov.b   r0,@r4
.Lcopy_done:
    rts
    nop

.Lcopy_bytes:
    mov.b   @r5+,r0
    dt      r6
    mov.b   r0,@r4
    bf/s    .Lcopy_bytes
    add     #1,r4
    rts
    nop

    .align 4
    .global _sh2_blit8_overwrite_row
_sh2_blit8_overwrite_row:
    tst     r6,r6
    bt      .Lodd_done
    mov     r4,r0
    tst     #1,r0
    bt      .Lodd_words
    mov.b   @r5+,r0
    add     #-1,r6
    mov.b   r0,@r4
    tst     r6,r6
    bt/s    .Lodd_done
    add     #1,r4

.Lodd_words:
    mov     r6,r7
    shlr    r7
    tst     r7,r7
    bt      .Lodd_tail
.Lodd_word_loop:
    mov.b   @r5+,r0
    extu.b  r0,r0
    shll8   r0
    mov.b   @r5+,r1
    extu.b  r1,r1
    or      r1,r0
    mov.w   r0,@r4
    dt      r7
    bf/s    .Lodd_word_loop
    add     #2,r4
.Lodd_tail:
    mov     r6,r0
    tst     #1,r0
    bt      .Lodd_done
    mov.b   @r5,r0
    mov.b   r0,@r4
.Lodd_done:
    rts
    nop

    .align 4
    .global _sh2_blit8_reverse_row
_sh2_blit8_reverse_row:
    tst     r6,r6
    bt      .Lreverse_done
    mov     r4,r0
    tst     #1,r0
    bf      .Lreverse_pairs

    /* Even right edge would make the preceding word odd; peel one byte. */
    mov.b   @r5+,r0
    add     #-1,r6
    mov.b   r0,@r4
    tst     r6,r6
    bt/s    .Lreverse_done
    add     #-1,r4

.Lreverse_pairs:
    mov     r6,r7
    shlr    r7
    tst     r7,r7
    bt      .Lreverse_tail
.Lreverse_word_loop:
    mov.b   @r5+,r0
    extu.b  r0,r0
    mov.b   @r5+,r1
    extu.b  r1,r1
    shll8   r1
    or      r1,r0
    add     #-1,r4
    mov.w   r0,@r4
    dt      r7
    bf/s    .Lreverse_word_loop
    add     #-1,r4
.Lreverse_tail:
    mov     r6,r0
    tst     #1,r0
    bt      .Lreverse_done
    mov.b   @r5,r0
    mov.b   r0,@r4
.Lreverse_done:
    rts
    nop

    .align 4
    .global _sh2_scale8_forward_row
_sh2_scale8_forward_row:
    mov.l   @r4,r5
    mov.l   @(4,r4),r6
    mov.l   @(8,r4),r7
    mov.l   @(12,r4),r1
    mov.l   @(16,r4),r2
    mov.l   @(20,r4),r3
    tst     r3,r3
    bt      .Lscale_forward_done
.Lscale_forward_loop:
    mov     r7,r0
    shlr16  r0
    and     r2,r0
    mov.b   @(r0,r6),r0
    add     r1,r7
    mov.b   r0,@r5
    dt      r3
    bf/s    .Lscale_forward_loop
    add     #1,r5
.Lscale_forward_done:
    rts
    nop

    .align 4
    .global _sh2_scale8_reverse_row
_sh2_scale8_reverse_row:
    mov.l   @r4,r5
    mov.l   @(4,r4),r6
    mov.l   @(8,r4),r7
    mov.l   @(12,r4),r1
    mov.l   @(16,r4),r2
    mov.l   @(20,r4),r3
    tst     r3,r3
    bt      .Lscale_reverse_done
.Lscale_reverse_loop:
    mov     r7,r0
    shlr16  r0
    and     r2,r0
    mov.b   @(r0,r6),r0
    add     r1,r7
    mov.b   r0,@r5
    dt      r3
    bf/s    .Lscale_reverse_loop
    add     #-1,r5
.Lscale_reverse_done:
    rts
    nop

    .align 4
    .global _sh2_font_glyph
_sh2_font_glyph:
    mov.l   @r4,r5
    mov.l   @(4,r4),r6
    mov.l   @(8,r4),r7
    mov.l   @(12,r4),r1
    mov.l   @(16,r4),r2
    mov.l   @(20,r4),r0
    tst     r0,r0
    bf      .Lfont_odd

    mov     #8,r3
.Lfont_even_row:
    mov.b   @r6+,r4
    extu.b  r4,r4

    mov     r4,r0
    shlr2   r0
    shlr2   r0
    shlr2   r0
    shll    r0
    mov.w   @(r0,r7),r0
    mov.w   r0,@r5

    mov     r4,r0
    shlr2   r0
    shlr2   r0
    and     #3,r0
    shll    r0
    mov.w   @(r0,r7),r0
    mov.w   r0,@(2,r5)

    mov     r4,r0
    shlr2   r0
    and     #3,r0
    shll    r0
    mov.w   @(r0,r7),r0
    mov.w   r0,@(4,r5)

    mov     r4,r0
    and     #3,r0
    shll    r0
    mov.w   @(r0,r7),r0
    mov.w   r0,@(6,r5)

    dt      r3
    bf/s    .Lfont_even_row
    add     r2,r5
    rts
    nop

.Lfont_odd:
    mov     #8,r3
.Lfont_odd_row:
    mov.b   @r6+,r4
    extu.b  r4,r4

    mov     r4,r0
    shlr2   r0
    shlr2   r0
    shlr2   r0
    shlr    r0
    mov.b   @(r0,r1),r0
    mov.b   r0,@r5
    add     #1,r5

    mov     r4,r0
    shlr2   r0
    shlr2   r0
    shlr    r0
    and     #3,r0
    shll    r0
    mov.w   @(r0,r7),r0
    mov.w   r0,@r5

    mov     r4,r0
    shlr2   r0
    shlr    r0
    and     #3,r0
    shll    r0
    mov.w   @(r0,r7),r0
    mov.w   r0,@(2,r5)

    mov     r4,r0
    shlr    r0
    and     #3,r0
    shll    r0
    mov.w   @(r0,r7),r0
    mov.w   r0,@(4,r5)

    mov     r4,r0
    and     #1,r0
    mov.b   @(r0,r1),r0
    mov.b   r0,@(6,r5)

    add     #-1,r5
    dt      r3
    bf/s    .Lfont_odd_row
    add     r2,r5
    rts
    nop

/* Audio accumulator helpers. */
    .align 4
    .global _sh2_mix_clear
_sh2_mix_clear:
    tst     r5,r5
    bt      .Lmix_clear_done
    mov     #0,r0
.Lmix_clear_loop:
    mov.l   r0,@r4
    dt      r5
    bf/s    .Lmix_clear_loop
    add     #4,r4
.Lmix_clear_done:
    rts
    nop

    .align 4
    .global _sh2_mix_channel
_sh2_mix_channel:
    tst     r6,r6
    bt      .Lmix_channel_done
.Lmix_channel_loop:
    mov.b   @r5+,r0
    extu.b  r0,r0
    add     #-128,r0
    muls.w  r7,r0
    sts     macl,r1
    mov.l   @r4,r2
    add     r1,r2
    mov.l   r2,@r4
    dt      r6
    bf/s    .Lmix_channel_loop
    add     #4,r4
.Lmix_channel_done:
    rts
    nop

    .align 4
    .global _sh2_mix_finalize
_sh2_mix_finalize:
    tst     r6,r6
    bt      .Lmix_finalize_done
    mov     #2,r7
    shll8   r7                  /* +512 */
    mov     r7,r3
    neg     r3,r3               /* -512 */
.Lmix_finalize_loop:
    mov.l   @r5+,r0
    shar    r0
    shar    r0
    shar    r0
    shar    r0
    cmp/gt  r7,r0
    bt      .Lmix_clamp_high
    cmp/gt  r0,r3
    bt      .Lmix_clamp_low
    bra     .Lmix_pack
    nop
.Lmix_clamp_high:
    bra     .Lmix_pack
    mov     r7,r0
.Lmix_clamp_low:
    mov     r3,r0
.Lmix_pack:
    add     r7,r0
    add     #4,r0               /* PWM center is 516 */
    mov     r0,r1
    shll16  r1
    or      r0,r1
    mov.l   r1,@r4
    dt      r6
    bf/s    .Lmix_finalize_loop
    add     #4,r4
.Lmix_finalize_done:
    rts
    nop

    .align 4
    .global _sh2_mix_silence
_sh2_mix_silence:
    tst     r5,r5
    bt      .Lmix_silence_done
    mov.l   .Lmix_center,r0
.Lmix_silence_loop:
    mov.l   r0,@r4
    dt      r5
    bf/s    .Lmix_silence_loop
    add     #4,r4
.Lmix_silence_done:
    rts
    nop
    .align 2
.Lmix_center:
    .long   0x02040204

/* Line-table context: table, count, current offset, pitch, wrap, hscroll. */
    .align 4
    .global _sh2_update_line_table
_sh2_update_line_table:
    mov.l   @r4,r5
    mov.l   @(4,r4),r6
    mov.l   @(8,r4),r7
    mov.l   @(12,r4),r1
    mov.l   @(16,r4),r2
    mov.l   @(20,r4),r3
    tst     r6,r6
    bt      .Lline_table_done
.Lline_table_loop:
    cmp/hs  r2,r7
    bf      .Lline_no_wrap
    sub     r2,r7
.Lline_no_wrap:
    mov     r7,r0
    add     r3,r0
    mov.w   r0,@r5
    add     r1,r7
    dt      r6
    bf/s    .Lline_table_loop
    add     #2,r5
.Lline_table_done:
    rts
    nop

/* Context: destination, 256-byte map, width, height, pitch. */
    .align 4
    .global _sh2_remap8_rect
_sh2_remap8_rect:
    mov.l   @r4,r5
    mov.l   @(4,r4),r1
    mov.l   @(8,r4),r6
    mov.l   @(12,r4),r7
    mov.l   @(16,r4),r2
    sub     r6,r2
    tst     r7,r7
    bt      .Lremap_done
.Lremap_row:
    mov     r6,r3
    tst     r3,r3
    bt      .Lremap_next_row
.Lremap_pixel:
    mov.b   @r5,r0
    extu.b  r0,r0
    mov.b   @(r0,r1),r0
    mov.b   r0,@r5
    dt      r3
    bf/s    .Lremap_pixel
    add     #1,r5
.Lremap_next_row:
    add     r2,r5
    dt      r7
    bf      .Lremap_row
.Lremap_done:
    rts
    nop

/* r4=uint16 destination, r5=entry count, r6=uint16 value */
    .align 4
    .global _sh2_fill16
_sh2_fill16:
    tst     r5,r5
    bt      .Lfill16_done
    extu.w  r6,r0
    mov     r0,r1
    shll16  r1
    or      r1,r0
	 mov     r0,r6

    mov     r4,r0
    tst     #2,r0
    bt      .Lfill16_pairs
    mov.w   r6,@r4
    add     #-1,r5
    tst     r5,r5
    bt/s    .Lfill16_done
    add     #2,r4

.Lfill16_pairs:
    mov     r5,r7
    shlr    r7
    tst     r7,r7
    bt      .Lfill16_tail
.Lfill16_pair_loop:
    mov.l   r6,@r4
    dt      r7
    bf/s    .Lfill16_pair_loop
    add     #4,r4
.Lfill16_tail:
    mov     r5,r0
    tst     #1,r0
    bt      .Lfill16_done
    mov.w   r6,@r4
.Lfill16_done:
    rts
    nop

/* Fixed-size, aligned, opaque tile copies. r4=dst, r5=src, r6=pitch. */
    .align 4
    .global _sh2_blit_tile8
_sh2_blit_tile8:
    mov     #8,r7
.Ltile8_row:
    mov.l   @r5+,r0
    mov.l   @r5+,r1
    mov.l   r0,@r4
    mov.l   r1,@(4,r4)
    dt      r7
    bf/s    .Ltile8_row
    add     r6,r4
    rts
    nop

    .align 4
    .global _sh2_blit_tile16
_sh2_blit_tile16:
    mov     #16,r7
.Ltile16_row:
    mov.l   @r5+,r0
    mov.l   @r5+,r1
    mov.l   @r5+,r2
    mov.l   @r5+,r3
    mov.l   r0,@r4
    mov.l   r1,@(4,r4)
    mov.l   r2,@(8,r4)
    mov.l   r3,@(12,r4)
    dt      r7
    bf/s    .Ltile16_row
    add     r6,r4
    rts
    nop

    .align 4
    .global _sh2_blit_tile32
_sh2_blit_tile32:
    mov     #32,r7
.Ltile32_row:
    mov.l   @r5+,r0
    mov.l   @r5+,r1
    mov.l   r0,@r4
    mov.l   r1,@(4,r4)
    mov.l   @r5+,r0
    mov.l   @r5+,r1
    mov.l   r0,@(8,r4)
    mov.l   r1,@(12,r4)
    mov.l   @r5+,r0
    mov.l   @r5+,r1
    mov.l   r0,@(16,r4)
    mov.l   r1,@(20,r4)
    mov.l   @r5+,r0
    mov.l   @r5+,r1
    mov.l   r0,@(24,r4)
    mov.l   r1,@(28,r4)
    dt      r7
    bf/s    .Ltile32_row
    add     r6,r4
    rts
    nop
