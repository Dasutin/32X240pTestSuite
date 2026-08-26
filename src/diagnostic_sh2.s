        .text
        .align  2

        .global _diagnosticProbeSr
_diagnosticProbeSr:
        stc     sr,r0
        mov     r0,r1
        ldc     r4,sr
        nop
        ldc     r1,sr
        rts
        nop

        .global _diagnosticReadSr
_diagnosticReadSr:
        stc     sr,r0
        rts
        nop

        .global _diagnosticWriteSr
_diagnosticWriteSr:
        ldc     r4,sr
        rts
        nop

        .global _diagnosticCacheSuspend
_diagnosticCacheSuspend:
        mov.l   diagnostic_cctl,r1
        mov.b   @r1,r0
        extu.b  r0,r0
        mov     #0,r2
        mov.b   r2,@r1
        rts
        nop

        .global _diagnosticCacheRestore
_diagnosticCacheRestore:
        mov.l   diagnostic_cctl,r1
        mov.b   r4,@r1
        rts
        nop

        .global _diagnosticReadVbr
_diagnosticReadVbr:
        stc     vbr,r0
        rts
        nop

        .global _diagnosticWriteVbr
_diagnosticWriteVbr:
        ldc     r4,vbr
        rts
        nop

        .global _diagnosticHblankModes
_diagnosticHblankModes:
        tst     r5,r5
        bt      diagnostic_hblank_done
diagnostic_hblank_next:
        mov.l   diagnostic_hblank_timeout,r2
diagnostic_hblank_wait_high:
        mov.l   diagnostic_fbctl,r1
        mov.w   @r1,r0
        mov.l   diagnostic_hblank_mask,r3
        and     r3,r0
        cmp/eq  r3,r0
        bt      diagnostic_hblank_write
        dt      r2
        bf      diagnostic_hblank_wait_high
        mov.l   r5,@r6
        mov     #-1,r0
        rts
        nop
diagnostic_hblank_write:
        mov.w   @r4+,r0
        mov.l   diagnostic_dispmode,r1
        mov.w   r0,@r1
        mov.l   diagnostic_hblank_timeout,r2
diagnostic_hblank_wait_low:
        mov.l   diagnostic_fbctl,r1
        mov.w   @r1,r0
        mov.l   diagnostic_hblank_mask,r3
        and     r3,r0
        tst     r0,r0
        bt      diagnostic_hblank_advance
        dt      r2
        bf      diagnostic_hblank_wait_low
        mov.l   r5,@r6
        mov     #-1,r0
        rts
        nop
diagnostic_hblank_advance:
        dt      r5
        bf      diagnostic_hblank_next
diagnostic_hblank_done:
        mov     #0,r0
        mov.l   r0,@r6
        rts
        nop

        .align  2
diagnostic_cctl:
        .long   0xFFFFFE92
diagnostic_fbctl:
        .long   0x2000410A
diagnostic_dispmode:
        .long   0x20004100
diagnostic_hblank_mask:
        .long   0x00004000
diagnostic_hblank_timeout:
        .long   0x00100000

        .section .rom_bank1.wram,"a"
        .align  4
        .global _diagnosticWramPattern
        .global _diagnostic_wram_pattern_end
_diagnosticWramPattern:
        .rept   2048
        .word   0x0000,0xFFFF,0xAAAA,0x5555
        .word   0x1357,0x2468,0x89AB,0xCDEF
        .word   0x0F0F,0xF0F0,0x33CC,0xCC33
        .word   0x8001,0x4002,0x2004,0x1008
        .endr
_diagnostic_wram_pattern_end:
