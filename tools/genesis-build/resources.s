/* Equivalent to the upstream resources.res BIN declaration. */
        .section .rodata
        .align  2
        .global pcmcheck_scd
pcmcheck_scd:
        .incbin "res/pcmcheck.bin"
