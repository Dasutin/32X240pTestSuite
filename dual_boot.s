/*
 * Runs from cartridge ROM before the 32X adapter is enabled.
 *
 * 32X - 32X security startup at 0x3F0.
 * Genesis/Mega Drive with no 32X: boot the Genesis suite.
 * 32X - Controller 1 with B button held: boot the Genesis suite.
 */

        .text
        .global dual_boot

dual_boot:
        move.w  #0x2700,%sr
        lea     0x00A10000,%a0
        cmpi.l  #0x4D415253,0x30EC(%a0) /* "MARS" signature */
        bne.s   genesis_boot

        move.b  #0x40,0x0009(%a0)       /* Port 1 TH is an output */
        move.b  #0x40,0x0003(%a0)       /* TH high: B is data bit 4 */
        move.b  #0x40,0x000B(%a0)       /* Nonzero means normal 32X boot */
        nop
        nop
        btst    #4,0x0003(%a0)
        bne.s   boot_32x                 /* Controller inputs are active low */

        clr.b   0x000B(%a0)              /* Preserve B through security startup */

boot_32x:
        jmp     0x000003F0               /* Standard 32X startup/security code */

genesis_boot:
        jmp     0x000A0200               /* Genesis payload + _Entry_Point */
