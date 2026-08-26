        .section .ramcode,"ax"

        .equ    DIAGNOSTIC_MD_DMA_TIMEOUT,0x00400000

        .global diagnosticMdRomBanks
diagnosticMdRomBanks:
        move.l  d2,-(sp)
        move.b  0xA15105,d2
        move.b  #0,0xA15105
        nop
        cmpi.l  #0x424B3030,0x9FFFFC
        bne.b   diagnostic_md_bank0_failed
        move.b  #1,0xA15105
        nop
        cmpi.l  #0x424B3031,0x9FFFFC
        bne.b   diagnostic_md_bank1_failed
        move.b  #2,0xA15105
        nop
        cmpi.l  #0x424B3130,0x9FFFFC
        bne.b   diagnostic_md_bank2_failed
        move.b  #3,0xA15105
        nop
        cmpi.l  #0x424B3131,0x9FFFFC
        bne.b   diagnostic_md_bank3_failed
        moveq   #0,d0
        bra.b   diagnostic_md_banks_done
diagnostic_md_bank0_failed:
        moveq   #1,d0
        bra.b   diagnostic_md_banks_done
diagnostic_md_bank1_failed:
        moveq   #2,d0
        bra.b   diagnostic_md_banks_done
diagnostic_md_bank2_failed:
        moveq   #3,d0
        bra.b   diagnostic_md_banks_done
diagnostic_md_bank3_failed:
        moveq   #4,d0
diagnostic_md_banks_done:
        move.b  d2,0xA15105
        move.l  (sp)+,d2
        rts

        .global diagnosticMdRamDreqProbe
diagnosticMdRamDreqProbe:
        move.l  d2,-(sp)
        move.l  d3,-(sp)
        move.l  d4,-(sp)
        move.w  sr,d4
        move.w  #0x2700,sr
        move.b  0xA15107,d2
        moveq   #7,d3
diagnostic_md_ram_dreq_next:
        move.b  d3,0xA15107
        move.b  0xA15107,d1
        andi.b  #7,d1
        cmp.b   d3,d1
        bne.b   diagnostic_md_ram_dreq_control_failed
        dbra    d3,diagnostic_md_ram_dreq_next
        move.b  #4,0xA15107
        move.w  #0x1357,d0
        moveq   #6,d3
diagnostic_md_ram_dreq_fill:
        move.w  d0,0xA15112
        dbra    d3,diagnostic_md_ram_dreq_fill
        move.b  0xA15107,d1
        andi.b  #0x80,d1
        bne.b   diagnostic_md_ram_dreq_not_empty
        move.w  d0,0xA15112
        move.b  0xA15107,d1
        andi.b  #0x80,d1
        beq.b   diagnostic_md_ram_dreq_not_full
        move.b  #0,0xA15107
        move.b  0xA15107,d1
        andi.b  #0x80,d1
        bne.b   diagnostic_md_ram_dreq_not_cleared
        moveq   #0,d0
        bra.b   diagnostic_md_ram_dreq_done
diagnostic_md_ram_dreq_control_failed:
        moveq   #1,d0
        lsl.l   #8,d0
        move.b  d3,d0
        lsl.l   #8,d0
        move.b  d1,d0
        bra.b   diagnostic_md_ram_dreq_done
diagnostic_md_ram_dreq_not_empty:
        move.l  #0x00020000,d0
        move.b  d1,d0
        bra.b   diagnostic_md_ram_dreq_done
diagnostic_md_ram_dreq_not_full:
        move.l  #0x00038000,d0
        move.b  d1,d0
        bra.b   diagnostic_md_ram_dreq_done
diagnostic_md_ram_dreq_not_cleared:
        move.l  #0x00040000,d0
        move.b  d1,d0
diagnostic_md_ram_dreq_done:
        move.b  d2,0xA15107
        move.w  d4,sr
        move.l  (sp)+,d4
        move.l  (sp)+,d3
        move.l  (sp)+,d2
        rts

        .global diagnosticMdCrossDreqControl
diagnosticMdCrossDreqControl:
        move.l  d2,-(sp)
        move.l  d3,-(sp)
        move.l  d4,-(sp)
        move.l  d5,-(sp)
        move.w  sr,d4
        move.w  #0x2700,sr
        moveq   #0,d5
        move.b  0xA15107,d5
        andi.w  #7,d5
        moveq   #7,d3
diagnostic_md_cross_dreq_next:
        move.b  d3,0xA15107
        move.w  #0xD400,d0
        or.b    d3,d0
        move.w  d0,0xA15122
        move.w  #0xFFFF,d2
diagnostic_md_cross_dreq_wait:
        move.w  0xA15122,d1
        move.w  d1,d0
        andi.w  #0xFF00,d0
        cmpi.w  #0xD500,d0
        beq.b   diagnostic_md_cross_dreq_ack
        dbra    d2,diagnostic_md_cross_dreq_wait
        move.l  #0x00020000,d0
        moveq   #0,d2
        move.b  d3,d2
        lsl.w   #8,d2
        or.l    d2,d0
        move.w  0xA15122,d1
        andi.w  #0x00FF,d1
        or.l    d1,d0
        bra.b   diagnostic_md_cross_dreq_done
diagnostic_md_cross_dreq_ack:
        andi.w  #0x0087,d1
        cmp.b   d3,d1
        bne.b   diagnostic_md_cross_dreq_failed
        dbra    d3,diagnostic_md_cross_dreq_next
        moveq   #0,d0
        bra.b   diagnostic_md_cross_dreq_done
diagnostic_md_cross_dreq_failed:
        move.l  #0x00010000,d0
        moveq   #0,d2
        move.b  d3,d2
        lsl.w   #8,d2
        or.l    d2,d0
        andi.w  #0x00FF,d1
        or.l    d1,d0
diagnostic_md_cross_dreq_done:
        move.b  d5,0xA15107
        move.w  d4,sr
        move.l  (sp)+,d5
        move.l  (sp)+,d4
        move.l  (sp)+,d3
        move.l  (sp)+,d2
        rts

        .global diagnosticMdVdpDma
diagnosticMdVdpDma:
	move.l  d2,-(sp)
	moveq   #0,d2
	move.b  0xA15107,d2
	move.b  #1,0xA15107
        lea     0xC00004,a0
        move.w  #0x8F02,(a0)
	move.l  16(sp),d0
        move.w  d0,d1
        andi.w  #0x00FF,d1
        ori.w   #0x9300,d1
        move.w  d1,(a0)
        lsr.l   #8,d0
        andi.w  #0x00FF,d0
        ori.w   #0x9400,d0
        move.w  d0,(a0)
	move.l  8(sp),d0
        lsr.l   #1,d0
        move.w  d0,d1
        andi.w  #0x00FF,d1
        ori.w   #0x9500,d1
        move.w  d1,(a0)
        lsr.l   #8,d0
        move.w  d0,d1
        andi.w  #0x00FF,d1
        ori.w   #0x9600,d1
        move.w  d1,(a0)
        lsr.l   #8,d0
        andi.w  #0x007F,d0
        ori.w   #0x9700,d0
        move.w  d0,(a0)
	move.l  12(sp),d0
        move.l  d0,d1
        andi.l  #0x00003FFF,d0
        swap    d0
        andi.l  #0x3FFF0000,d0
        andi.l  #0x0000C000,d1
        lsr.l   #8,d1
        lsr.l   #6,d1
        or.l    d1,d0
        ori.l   #0x40000080,d0
        move.l  d0,(a0)
        move.l  #DIAGNOSTIC_MD_DMA_TIMEOUT,d1
diagnostic_md_vdp_dma_wait:
        move.w  (a0),d0
        btst    #1,d0
        beq.b   diagnostic_md_vdp_dma_done
        subq.l  #1,d1
        bne.b   diagnostic_md_vdp_dma_wait
	moveq   #1,d0
	move.b  d2,0xA15107
	move.l  (sp)+,d2
        rts
diagnostic_md_vdp_dma_done:
	move.b  d2,0xA15107
	move.l  (sp)+,d2
        moveq   #0,d0
        rts

        .global diagnosticMdWordRamVdpDma
diagnosticMdWordRamVdpDma:
        move.l  d2,-(sp)
        move.w  sr,d2
        move.w  #0x2700,sr
        lea     0xC00004,a0
        move.w  #0x8F00,(a0)
        move.w  #0x8114,(a0)
        move.w  #0x9300,(a0)
        move.w  #0x9480,(a0)
        move.w  #0x9500,(a0)
        move.w  #0x9600,(a0)
        move.w  #0x9730,(a0)
        move.l  #0x50000081,-(sp)
        move.w  (sp)+,(a0)
        move.w  (sp)+,(a0)
        nop
        move.w  #0x8F02,(a0)
        move.w  #0x8164,(a0)
        move.w  d2,sr
        move.l  diagnosticMdVblankCount,d1
diagnostic_md_word_ram_wait:
        move.b  0xA15107,d0
        btst    #2,d0
        beq.b   diagnostic_md_word_ram_done
        move.l  diagnosticMdVblankCount,d0
        sub.l   d1,d0
        cmpi.l  #30,d0
        bcs.b   diagnostic_md_word_ram_wait
        moveq   #1,d0
        move.l  (sp)+,d2
        rts
diagnostic_md_word_ram_done:
        move.l  (sp)+,d2
        moveq   #0,d0
        rts

        .section .text
        .extern reload_font
        .global diagnosticMdReloadFont
diagnosticMdReloadFont:
        move.l  d2,-(sp)
        move.l  a2,-(sp)
        jsr     reload_font
        move.l  (sp)+,a2
        move.l  (sp)+,d2
        rts
