OUTPUT_ARCH(m68k)
SEARCH_DIR(.)
__DYNAMIC = 0;

MEMORY
{
    rom (rx) : ORIGIN = 0x000A0000, LENGTH = 0x00040000
    ram (wx) : ORIGIN = 0x00FF0000, LENGTH = 0x00010000
}

PROVIDE (__stack = 0x01000000);

SECTIONS
{
  .text ORIGIN(rom) :
  {
    *(.text .text.*)
    . = ALIGN(0x4);
    __CTOR_LIST__ = .;
    LONG((__CTOR_END__ - __CTOR_LIST__) / 4 - 2)
    *(.ctors)
    LONG(0)
    __CTOR_END__ = .;
    __DTOR_LIST__ = .;
    LONG((__DTOR_END__ - __DTOR_LIST__) / 4 - 2)
    *(.dtors)
    LONG(0)
    __DTOR_END__ = .;
    *(.rodata .rodata.*)
    *(.gcc_except_table .gcc_except_table.*)

    . = ALIGN(0x4);
    __INIT_SECTION__ = .;
    *(.init)
    SHORT(0x4E75)
    __FINI_SECTION__ = .;
    *(.fini)
    SHORT(0x4E75)

    _etext = .;
    *(.lit)
  } > rom

  .data 0x00FF0000 :
  AT (LOADADDR(.text) + SIZEOF(.text))
  {
    *(.got.plt) *(.got)
    *(.shdata)
    *(.data .data.*)
    _edata = .;
  } > ram
  _stext = LOADADDR(.data);
  _sdata = SIZEOF(.data);

  .bss 0x00FF0000 + SIZEOF(.data) :
  {
    _start = .;
    *(.shbss)
    *(.bss .bss.*)
    *(COMMON)
    _bend = .;
  } > ram

  .stab 0 (NOLOAD) : { *(.stab) }
  .stabstr 0 (NOLOAD) : { *(.stabstr) }
  .eh_frame 0 (NOLOAD) : { *(.eh_frame) }

  ASSERT((SIZEOF(.text) + SIZEOF(.data)) <= LENGTH(rom),
         "Genesis payload exceeds reserved ROM region")
}
