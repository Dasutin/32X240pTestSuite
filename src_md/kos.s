# ---------------------------------------------
# Kosinski decompression subroutine
# void Kos_Decomp(uint8_t *src, uint8_t *dst)
# Inputs:
# 4(sp) = compressed data location
# 8(sp) = destination
# ---------------------------------------------

        .text

        .global Kos_Decomp
Kos_Decomp:
        movea.l 4(sp),a0
        movea.l 8(sp),a1
        movem.l d2-d6,-(sp)
        subq.l  #2,sp                   /* Make space for two bytes on the stack */
        move.b  (a0)+,1(sp)
        move.b  (a0)+,(sp)
        move.w  (sp),d5                 /* Copy first description field */
        moveq   #15,d4                  /* 16 bits in a byte */

Kos_Decomp_Loop:
        lsr.w   #1,d5                   /* Bit which is shifted out goes into C flag */
        move    sr,d6
        dbra    d4,Kos_Decomp_ChkBit
        move.b  (a0)+,1(sp)
        move.b  (a0)+,(sp)
        move.w  (sp),d5                 /* Get next description field if needed */
        moveq   #15,d4                  /* Reset bit counter */

Kos_Decomp_ChkBit:
        move    d6,ccr                  /* Was the bit set? */
        bcc.b   Kos_Decomp_RLE          /* If not, branch (C flag clear means bit was clear) */
        move.b  (a0)+,(a1)+             /* Otherwise, copy byte as-is */
        bra.b   Kos_Decomp_Loop

Kos_Decomp_RLE:
        moveq   #0,d3
        lsr.w   #1,d5                   /* Get next bit */
        move    sr,d6
        dbra    d4,Kos_Decomp_ChkBit2
        move.b  (a0)+,1(sp)
        move.b  (a0)+,(sp)
        move.w  (sp),d5
        moveq   #15,d4

Kos_Decomp_ChkBit2:
        move    d6,ccr                  /* Was the bit set? */
        bcs.b   Kos_Decomp_SeparateRLE  /* If it was, branch */
        lsr.w   #1,d5                   /* Bit which is shifted out goes into X flag */
        dbra    d4,1f
        move.b  (a0)+,1(sp)
        move.b  (a0)+,(sp)
        move.w  (sp),d5
        moveq   #15,d4
1:
        roxl.w  #1,d3                   /* Get high repeat count bit (shift X flag in) */
        lsr.w   #1,d5
        dbra    d4,2f
        move.b  (a0)+,1(sp)
        move.b  (a0)+,(sp)
        move.w  (sp),d5
        moveq   #15,d4
2:
        roxl.w  #1,d3                   /* Get low repeat count bit */
        addq.w  #1,d3                   /* Increment repeat count */
        moveq   #-1,d2
        move.b  (a0)+,d2                /* Calculate offset */
        bra.b   Kos_Decomp_RLELoop

Kos_Decomp_SeparateRLE:
        move.b  (a0)+,d0                /* Get first byte */
        move.b  (a0)+,d1                /* Get second byte */
        moveq   #-1,d2
        move.b  d1,d2
        lsl.w   #5,d2
        move.b  d0,d2                   /* Calculate offset */
        andi.w  #7,d1                   /* Does a third byte need to be read? */
        beq.b   Kos_Decomp_SeparateRLE2 /* If it does, branch */
        move.b  d1,d3                   /* Copy repeat count */
        addq.w  #1,d3                   /* And increment it */

Kos_Decomp_RLELoop:
        move.b  (a1,d2.w),d0
        move.b  d0,(a1)+                /* Copy appropriate byte */
        dbra    d3,Kos_Decomp_RLELoop   /* And repeat the copying */
        bra.b   Kos_Decomp_Loop

Kos_Decomp_SeparateRLE2:
        move.b  (a0)+,d1
        beq.b   Kos_Decomp_Done         /* 0 indicates end of compressed data */
        cmpi.b  #1,d1
        beq.w   Kos_Decomp_Loop         /* 1 indicates a new description needs to be read */
        move.b  d1,d3                   /* Otherwise, copy repeat count */
        bra.b   Kos_Decomp_RLELoop

Kos_Decomp_Done:
        addq.l  #2,sp                   /* Restore stack pointer to original state */
        movem.l (sp)+,d2-d6
        rts

# End of function Kos_Decomp
