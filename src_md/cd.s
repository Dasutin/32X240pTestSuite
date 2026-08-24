#------------------------------
# SEGA 32X MegaCD Support Code
# By Chilly Willy
#------------------------------
        .text

        .global Sub_Start
Sub_Start:

# Standard MegaCD Sub-CPU Program Header (copied to 0x6000)

SPHeader:
        .asciz  "MAIN-SUBCPU"
        .word   0x0001,0x0000
        .long   0x00000000
        .long   0x00000000
        .long   SPHeaderOffsets-SPHeader
        .long   0x00000000

SPHeaderOffsets:
        .word   SPInit-SPHeaderOffsets
        .word   SPMain-SPHeaderOffsets
        .word   SPInt2-SPHeaderOffsets
        .word   SPNull-SPHeaderOffsets
        .word   0x0000

# Sub-CPU Program Initialization (VBlank not enabled yet)

SPInit:
        move.b  #'I,0x800F.w            /* Sub COMM Port = INITIALIZING */
        andi.b  #0xE2,0x8003.w          /* Priority Mode = off, 2M mode, Sub-CPU has access */
        rts

# Sub-CPU Program Main Entry Point (VBlank now enabled)

SPMain:
        bsr     InitPCM
        move.w  #0x0081,d0              /* CDBSTAT */
        jsr     0x5F22.w                /* Call CDBIOS function */
        move.w  0(a0),d0                /* BIOS status word */
        bmi.b   1f                      /* Not ready */
        lsr.w   #8,d0
        cmpi.b  #0x40,d0
        beq.b   9f                      /* Open */
        cmpi.b  #0x10,d0
        beq.b   9f                      /* No disc */
1:
# Initialize Drive
        lea     drive_init_parms(pc),a0
        move.w  #0x0010,d0              /* DRVINIT */
        jsr     0x5F22.w                /* Call CDBIOS function */
        bsr     WaitBIOS

        move.w  #0x0089,d0              /* CDCSTOP - stop reading data */
        jsr     0x5F22.w                /* Call CDBIOS function */
        bsr     WaitBIOS
        move.w  #0x008A,d0              /* CDCSTAT */
        jsr     0x5F22.w                /* Call CDBIOS function */
        bsr     WaitBIOS
9:
        move.b  #0,0x800F.w             /* Sub COMM Port = READY */

# Wait for command in Main COMM Port
WaitCmd:
        tst.b   0x800E.w
        beq.b   WaitCmd
        cmpi.b  #'D,0x800E.w
        beq     GetDiscInfo
        cmpi.b  #'T,0x800E.w
        beq     GetTrackInfo
        cmpi.b  #'P,0x800E.w
        beq     PlayTrack
        cmpi.b  #'S,0x800E.w
        beq     StopPlaying
        cmpi.b  #'Z,0x800E.w
        beq     PauseResume
        cmpi.b  #'C,0x800E.w
        beq     CheckDisc
        cmpi.b  #'O,0x800E.w
        beq     OpenTray
        cmpi.b  #'F,0x800E.w
        beq     FillPCMRAM
        cmpi.b  #'M,0x800E.w
        beq     CheckPCMRAM
        cmpi.b  #'A,0x800E.w
        beq     ControlPCM
        cmpi.b  #'L,0x800E.w
        beq     LoadPCM
        cmpi.b  #'G,0x800E.w
        beq     CenterPCM
        cmpi.b  #'R,0x800E.w
        beq     ReadSectors
        cmpi.b  #'W,0x800E.w
        beq     FillWordRAM
        cmpi.b  #'Q,0x800E.w
        beq     PingSubCPU
        move.b  #'E,0x800F.w            /* Sub COMM Port = ERROR */
WaitAck:
        tst.b   0x800E.w
        bne.b   WaitAck                 /* Wait for result acknowledged */
        move.b  #0,0x800F.w             /* Sub COMM Port = READY */
        bra     WaitCmd

GetDiscInfo:
        move.l  #0x00200000,d7
1:
        move.w  #0x0081,d0              /* CDBSTAT */
        jsr     0x5F22.w                /* Call CDBIOS function */
        move.w  0(a0),d2                /* BIOS status word */
        btst.l  #14,d2                  /* Tray open? */
        bne.b   DiscTrayOpen
        btst.l  #12,d2                  /* No disc? */
        bne.b   DiscNotFound
        btst.l  #15,d2                  /* Drive not ready? */
        beq.b   2f
        subq.l  #1,d7
        bne.b   1b
        bra.b   DiscInfoTimeout
2:
        move.l  #0x00200000,d7
3:
        move.w  #0x0081,d0              /* Refresh CDBSTAT/TOC table */
        jsr     0x5F22.w                /* Call CDBIOS function */
        move.w  0(a0),d2
        btst.l  #14,d2
        bne.b   DiscTrayOpen
        btst.l  #12,d2
        bne.b   DiscNotFound
        btst.l  #13,d2                  /* TOC still being read? */
        beq.b   4f
        subq.l  #1,d7
        bne.b   3b
        bra.b   DiscInfoTimeout

4:
        cmpi.b  #0xFF,17(a0)            /* Invalid/unread track list? */
        beq.b   DiscInvalidTOC
        move.w  #1,0x8020.w             /* Ready */
        move.w  16(a0),0x8022.w         /* First song number, Last song number */
        moveq   #0,d2
        move.b  18(a0),d2               /* Zero-extended drive version */
        move.w  d2,0x8024.w

        move.b  #'D,0x800F.w            /* Sub COMM Port = DONE */
        bra     WaitAck

DiscInvalidTOC:
        move.w  #0x000A,0x8020.w
        bra.b   DiscInfoError
DiscTrayOpen:
        move.w  #0x000B,0x8020.w
        bra.b   DiscInfoError
DiscNotFound:
        move.w  #0x000C,0x8020.w
        bra.b   DiscInfoError
DiscInfoTimeout:
        move.w  #0x000D,0x8020.w
DiscInfoError:
        clr.w   0x8022.w
        clr.w   0x8024.w

        move.b  #'D,0x800F.w            /* Sub COMM Port = DONE */
        bra     WaitAck

GetTrackInfo:
        move.w  0x8010.w,d1             /* Track number */
        move.w  #0x0083,d0              /* CDBTOCREAD */
        jsr     0x5F22.w                /* Call CDBIOS function */
        bsr     WaitBIOS
        tst.l   d7
        beq     TrackInfoFail
        move.l  d0,0x8020.w             /* MMSSFFTN */
        move.w  d1,0x8024.w             /* $0000 CD-DA, $00FF data */

        move.b  #'D,0x800F.w            /* Sub COMM Port = DONE */
        bra     WaitAck

PlayTrack:
        move.w  #0x0002,d0              /* MSCSTOP - stop playing */
        jsr     0x5F22.w                /* Call CDBIOS function */
        bsr     WaitBIOS

        move.w  0x8010.w,d1             /* Track number */
        move.w  #0x0011,d0              /* MSCPLAY - play from track on */
        move.b  0x8012.w,d2             /* Flag */
        bmi.b   2f
        beq.b   1f
        move.w  #0x0013,d0              /* MSCPLAYR - play with repeat */
        bra.b   2f
1:
        move.w  #0x0012,d0              /* MSCPLAY1 - play once */
2:
        lea     track_number(pc),a0
        move.w  d1,(a0)
        jsr     0x5F22.w                /* Call CDBIOS function */
        bsr     WaitBIOS

        move.b  #'D,0x800F.w            /* Sub COMM Port = DONE */
        bra     WaitAck

StopPlaying:
        move.w  #0x0002,d0              /* MSCSTOP - stop playing */
        jsr     0x5F22.w                /* Call CDBIOS function */
        bsr     WaitBIOS

        move.b  #'D,0x800F.w            /* Sub COMM Port = DONE */
        bra     WaitAck

PauseResume:
        move.w  #0x0081,d0              /* CDBSTAT */
        jsr     0x5F22.w                /* Call CDBIOS function */
        move.b  (a0),d0
        cmpi.b  #1,d0
        beq.b   1f                      /* Playing - pause playback */
        cmpi.b  #5,d0
        beq.b   2f                      /* Paused - resume playback */

        move.b  #'E,0x800F.w            /* Sub COMM Port = ERROR */
        bra     WaitAck
1:
        move.w  #0x0003,d0              /* MSCPAUSEON - pause playback */
        jsr     0x5F22.w                /* Call CDBIOS function */

        move.b  #'D,0x800F.w            /* Sub COMM Port = DONE */
        bra     WaitAck
2:
        move.w  #0x0004,d0              /* MSCPAUSEOFF - resume playback */
        jsr     0x5F22.w                /* Call CDBIOS function */

        move.b  #'D,0x800F.w            /* Sub COMM Port = DONE */
        bra     WaitAck

CheckDisc:
        lea     drive_init_parms(pc),a0
        move.w  #0x0010,d0              /* DRVINIT */
        jsr     0x5F22.w                /* Call CDBIOS function */
        bsr     WaitBIOS

        move.w  #0x0089,d0              /* CDCSTOP - stop reading data */
        jsr     0x5F22.w                /* Call CDBIOS function */
        bsr     WaitBIOS
        move.w  #0x008A,d0              /* CDCSTAT */
        jsr     0x5F22.w                /* Call CDBIOS function */
        bsr     WaitBIOS

        move.b  #'D,0x800F.w            /* Sub COMM Port = DONE */
        bra     WaitAck

TrackInfoFail:
        clr.l   0x8020.w
        move.w  #2,0x8024.w
        move.b  #'E,0x800F.w
        bra     WaitAck

OpenTray:
        move.w  #0x000A,d0              /* DRVOPEN - open loading tray */
        jsr     0x5F22.w                /* Call CDBIOS function */

        move.b  #'D,0x800F.w            /* Sub COMM Port = DONE */
        bra     WaitAck

WaitBIOS:
        move.l  #0x00200000,d7
1:
        move.w  #0x0080,d0              /* CDBCHK */
        jsr     0x5F22.w                /* Call CDBIOS function */
        bcc.b   2f
        subq.l  #1,d7
        bne.b   1b
        moveq   #0,d7                    /* Timed out */
        rts
2:
        moveq   #1,d7                    /* Completed */
        rts

# RF5C164 register addresses in Sub-CPU space
ENVdat  = 0xFF0001
PANdat  = 0xFF0003
FDLdat  = 0xFF0005
FDHdat  = 0xFF0007
LSLdat  = 0xFF0009
LSHdat  = 0xFF000B
STdat   = 0xFF000D
CTRLdat = 0xFF000F
ONOFFdat = 0xFF0011
WAVEdat = 0xFF2001

PCMWait:
        move.l  d0,-(a7)
        move.w  #5,d0
1:
        dbra    d0,1b
        move.l  (a7)+,d0
        rts

InitPCM:
        move.b  #0xFF,ONOFFdat
        bsr     PCMWait
        move.b  #0x80,CTRLdat             /* Write bank 0 */
        bsr     PCMWait
        lea     pcm_sample(pc),a1
        movea.l #WAVEdat,a0
        move.w  #4094,d0
        moveq   #0,d1
1:
        move.b  0(a1,d1.w),(a0)
        addq.l  #2,a0
        addq.w  #1,d1
        andi.w  #31,d1
        dbra    d0,1b
        move.b  #0xFF,(a0)                 /* Loop marker */

        move.b  #0xC0,CTRLdat              /* Enable PCM, channel 1 */
        bsr     PCMWait
        move.b  #0xFF,PANdat
        bsr     PCMWait
        move.b  #0x00,ENVdat
        bsr     PCMWait
        move.b  #0x00,STdat
        bsr     PCMWait
        move.b  #0x00,LSHdat
        bsr     PCMWait
        move.b  #0x00,LSLdat
        bsr     PCMWait
        move.b  #0x08,FDHdat
        bsr     PCMWait
        move.b  #0x00,FDLdat
        bsr     PCMWait
        rts

ControlPCM:
        move.w  0x8010.w,d0
        tst.w   d0
        beq     StopPCM
        cmpi.w  #1,d0
        bne.b   1f
        move.b  #0x0F,PANdat               /* Left */
        bra.b   3f
1:
        cmpi.w  #3,d0
        bne.b   2f
        move.b  #0xF0,PANdat               /* Right */
        bra.b   3f
2:
        move.b  #0xFF,PANdat               /* Center */
3:
        bsr     PCMWait
        move.b  #0x08,FDHdat
        bsr     PCMWait
        move.b  #0x00,FDLdat
        bsr     PCMWait
        move.b  #0xFF,ENVdat
        bsr     PCMWait
        move.b  #0x00,STdat
        bsr     PCMWait
        move.b  #0xFE,ONOFFdat
        bsr     PCMWait
        clr.l   0x8020.w
        clr.w   0x8024.w
        move.b  #'D,0x800F.w
        bra     WaitAck

LoadPCM:
        bsr     InitPCM
        clr.l   0x8020.w
        clr.w   0x8024.w
        move.b  #'D,0x800F.w
        bra     WaitAck

CenterPCM:
        move.b  #0xFF,PANdat
        bsr     PCMWait
        clr.l   0x8020.w
        clr.w   0x8024.w
        move.b  #'D,0x800F.w
        bra     WaitAck

StopPCM:
        move.b  #0xFF,ONOFFdat
        bsr     PCMWait
        move.b  #0x00,ENVdat
        bsr     PCMWait
        clr.l   0x8020.w
        clr.w   0x8024.w
        move.b  #'D,0x800F.w
        bra     WaitAck

FillPCMRAM:
        move.b  #0xFF,ONOFFdat
        bsr     PCMWait
        move.b  #0x40,CTRLdat             /* Disable PCM RAM playback access */
        bsr     PCMWait
        move.b  0x8011.w,d4                /* Test value */
        move.w  0x8012.w,d6                /* Bank, or 0xFF for all */
        cmpi.w  #0x00FF,d6
        beq.b   1f
        move.w  d6,d5
        bra.b   2f
1:
        moveq   #0,d5
2:
        move.b  d5,CTRLdat                 /* Select write bank 0-15 */
        bsr     PCMWait
        movea.l #WAVEdat,a0
        move.w  #4095,d1
3:
        move.b  d4,(a0)
        addq.l  #2,a0
        dbra    d1,3b

        cmpi.w  #0x00FF,d6
        bne.b   PCMFillDone
        addq.w  #1,d5
        cmpi.w  #16,d5
        bne.b   2b

PCMFillDone:
        move.b  #0xC0,CTRLdat              /* Restore PCM access, channel 1 */
        bsr     PCMWait
        clr.l   0x8020.w
        clr.w   0x8024.w
        move.b  #'D,0x800F.w
        bra     WaitAck

CheckPCMRAM:
        move.b  #0xFF,ONOFFdat
        bsr     PCMWait
        move.b  #0x40,CTRLdat             /* Disable PCM RAM playback access */
        bsr     PCMWait
        move.b  0x8011.w,d4                /* Test value */
        move.w  0x8012.w,d6                /* Bank, or 0xFF for all */
        cmpi.w  #0x00FF,d6
        beq.b   4f
        move.w  d6,d5
        bra.b   5f
4:
        moveq   #0,d5
5:
        move.b  d5,CTRLdat                 /* Select read bank 0-15 */
        bsr     PCMWait

        movea.l #WAVEdat,a0
        move.w  #4095,d1
        moveq   #0,d2
6:
        move.b  (a0),d3
        cmp.b   d4,d3
        bne.b   PCMFail
        addq.l  #2,a0
        addq.w  #1,d2
        dbra    d1,6b

        cmpi.w  #0x00FF,d6
        bne.b   PCMCompareDone
        addq.w  #1,d5
        cmpi.w  #16,d5
        bne.b   5b

PCMCompareDone:
        move.b  #0xC0,CTRLdat              /* Restore PCM access, channel 1 */
        bsr     PCMWait
        clr.l   0x8020.w
        clr.w   0x8024.w
        move.b  #'D,0x800F.w
        bra     WaitAck

PCMFail:
        move.w  d5,0x8020.w                /* Failed bank */
        move.w  d2,0x8022.w                /* Failed offset */
        moveq   #0,d0
        move.b  d3,d0
        move.w  d0,0x8024.w                /* Read value */
        move.b  #0xC0,CTRLdat              /* Restore PCM access, channel 1 */
        bsr     PCMWait
        move.b  #'E,0x800F.w
        bra     WaitAck

# Read one or more Mode 1 sectors into 2M Word RAM and calculate a CRC32.
ReadSectors:
        bclr    #0,0x8003.w                 /* Sub-CPU owns Word RAM */
        moveq   #0,d0
        move.w  0x8010.w,d0                /* Starting LBA */
        moveq   #0,d1
        move.w  0x8012.w,d1                /* Sector count */
        beq     SectorReadFail
        lea     bios_packet(pc),a5
        move.l  d0,(a5)
        move.l  d1,4(a5)
        move.l  #0x00080000,8(a5)
        movea.l a5,a0
        move.w  #0x0089,d0                 /* CDCSTOP */
        jsr     0x5F22.w
        bsr     WaitBIOS
        tst.l   d7
        beq     SectorReadFail
        moveq   #0,d1                      /* Mode 1, 2048-byte sectors */
        move.w  #0x0096,d0                 /* CDCSETMODE */
        jsr     0x5F22.w
        bsr     WaitBIOS
        tst.l   d7
        beq     SectorReadFail
        andi.w  #0xF8FF,0x8004.w
        ori.w   #0x0300,0x8004.w            /* CDC destination: Sub-CPU read */
        movea.l a5,a0
        move.w  #0x0020,d0                 /* ROMREADN */
        jsr     0x5F22.w

SectorWaitStat:
        move.l  #0x00200000,d7
1:
        move.w  #0x008A,d0                 /* CDCSTAT */
        jsr     0x5F22.w
        bcc.b   2f
        subq.l  #1,d7
        bne.b   1b
        bra     SectorReadFail
2:
        move.l  #0x00200000,d7
3:
        move.w  #0x008B,d0                 /* CDCREAD */
        jsr     0x5F22.w
        bcc.b   4f
        subq.l  #1,d7
        bne.b   3b
        bra     SectorReadFail
4:
        movea.l 8(a5),a0
        lea     12(a5),a1
        move.l  #0x00200000,d7
5:
        move.w  #0x008C,d0                 /* CDCTRN */
        jsr     0x5F22.w
        bcc.b   6f
        subq.l  #1,d7
        bne.b   5b
        bra     SectorReadFail
6:
        move.w  #0x008D,d0                 /* CDCACK */
        jsr     0x5F22.w
        addq.l  #1,(a5)
        addi.l  #0x0800,8(a5)
        subq.l  #1,4(a5)
        bne     SectorWaitStat

        moveq   #0,d1
        move.w  0x8012.w,d1
        lsl.l   #8,d1
        lsl.l   #3,d1                      /* Sectors * 2048 */
        movea.l #0x00080000,a0
        bsr     CRC32
        move.l  d0,0x8020.w
        move.w  0x8012.w,0x8024.w
        bset    #0,0x8003.w                 /* Give Word RAM to Main CPU */
        move.b  #'D,0x800F.w
        bra     WaitAck

SectorReadFail:
        clr.l   0x8020.w
        move.w  #1,0x8024.w
        bset    #0,0x8003.w
        move.b  #'E,0x800F.w
        bra     WaitAck

# d1.l bytes at a0, result in d0.l
CRC32:
        move.l  #0xFFFFFFFF,d0
        tst.l   d1
        beq.b   4f
1:
        moveq   #0,d2
        move.b  (a0)+,d2
        eor.l   d2,d0
        moveq   #7,d3
2:
        lsr.l   #1,d0
        bcc.b   3f
        eori.l  #0xEDB88320,d0
3:
        dbra    d3,2b
        subq.l  #1,d1
        bne.b   1b
4:
        not.l   d0
        rts

FillWordRAM:
        bclr    #0,0x8003.w
        move.w  0x8010.w,d4
        movea.l #0x00080000,a0
        move.w  #4095,d0
        moveq   #0,d1
1:
        move.w  d4,d2
        eor.w   d1,d2
        move.w  d2,(a0)+
        addq.w  #1,d1
        dbra    d0,1b
        bset    #0,0x8003.w
        clr.l   0x8020.w
        clr.w   0x8024.w
        move.b  #'D,0x800F.w
        bra     WaitAck

PingSubCPU:
        move.w  0x8010.w,0x8020.w
        move.w  #0xE715,0x8022.w
        clr.w   0x8024.w
        move.b  #'D,0x800F.w
        bra     WaitAck


| Sub-CPU Program VBlank (INT02) Service Handler

SPInt2:
        rts

| Sub-CPU program Reserved Function

SPNull:
        rts


| Sub-CPU variables

        .align  2
drive_init_parms:
        .byte   0x01, 0xFF              /* First track (1), last track (all) */

track_number:
        .word   0

        .align  2
pcm_sample:
        .byte   0x80,0x98,0xB0,0xC6,0xD9,0xE9,0xF4,0xFC
        .byte   0xFE,0xFC,0xF4,0xE9,0xD9,0xC6,0xB0,0x98
        .byte   0x80,0x18,0x30,0x46,0x59,0x69,0x74,0x7C
        .byte   0x7E,0x7C,0x74,0x69,0x59,0x46,0x30,0x18

        .align  2
bios_packet:
        .long   0,0,0,0,0


        .global Sub_End
Sub_End:
