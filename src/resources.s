.section .text

.align 2

soundFileName1:
.asciz ""

.align 4

soundFile1:
soundFileEnd1:
.align 4

.global _soundFileName
_soundFileName:
.long soundFileName1

.global _soundFileSize
_soundFileSize:
.long soundFileEnd1 - soundFile1

.global _soundFilePtr
_soundFilePtr:
.long soundFile1

.align 4
