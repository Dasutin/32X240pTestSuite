OUTPUT_ARCH(sh)
EXTERN (_start)
ENTRY (_start)
__DYNAMIC = 0;

MEMORY
{
    rom_core (rx) : ORIGIN = 0x02000000, LENGTH = 0x000FFFFC
    marker0 (r) : ORIGIN = 0x020FFFFC, LENGTH = 0x00000004
    rom_bank1 (rx) : ORIGIN = 0x02100000, LENGTH = 0x000FFFFC
    marker1 (r) : ORIGIN = 0x021FFFFC, LENGTH = 0x00000004
    rom_bank2 (rx) : ORIGIN = 0x02200000, LENGTH = 0x000FFFFC
    marker2 (r) : ORIGIN = 0x022FFFFC, LENGTH = 0x00000004
    rom_bank3_assets (rx) : ORIGIN = 0x02340000, LENGTH = 0x000BFFFC
    marker3 (r) : ORIGIN = 0x023FFFFC, LENGTH = 0x00000004
    ram (wx) : ORIGIN = 0x06000000, LENGTH = 0x0003FC00
}

PROVIDE (__stack = 0x0603FC00);
PROVIDE (__diag_general_start = 0x06010000);
PROVIDE (__diag_general_end = 0x06020000);
PROVIDE (__diag_dma_start = 0x06020000);
PROVIDE (__diag_dma_end = 0x06030000);
PROVIDE (__diag_secondary_start = 0x06030000);
PROVIDE (__diag_secondary_end = 0x0603C000);

SECTIONS
{
  .text 0x02000000 :
  AT (0x00000000)
  {
    __text_start = .;
    *(.text)
    *(.text.*)
    *(.gnu.linkonce.t.*)
    . = ALIGN(16);
    __INIT_SECTION__ = .;
    KEEP (*(.init))
    SHORT (0x000B)
    SHORT (0x0009)
    . = ALIGN(16);
    __FINI_SECTION__ = .;
    KEEP (*(.fini))
    SHORT (0x000B)
    SHORT (0x0009)
    *(.eh_frame_hdr)
    KEEP (*(.eh_frame))
    *(.gcc_except_table)
    *(.gcc_except_table.*)
    KEEP (*(.jcr))
    . = ALIGN(16);
    __CTOR_LIST__ = .;
    ___CTOR_LIST__ = .;
    LONG((__CTOR_END__ - __CTOR_LIST__) / 4 - 2)
    KEEP (*(SORT(.ctors.*)))
    KEEP (*(.ctors))
    LONG(0)
    __CTOR_END__ = .;
    . = ALIGN(16);
    __DTOR_LIST__ = .;
    ___DTOR_LIST__ = .;
    LONG((__DTOR_END__ - __DTOR_LIST__) / 4 - 2)
    KEEP (*(SORT(.dtors.*)))
    KEEP (*(.dtors))
    LONG(0)
    __DTOR_END__ = .;
    *(.rdata)
    *(.rodata)
    *(.rodata.*)
    *(.gnu.linkonce.r.*)
    . = ALIGN(16);
    __text_end = .;
  } > rom_core
  __text_size = __text_end - __text_start;

  .data 0x06000000 :
  AT (LOADADDR(.text) + SIZEOF(.text))
  {
    __data_start = .;
    *(.ramcode)
    *(.ramcode.*)
    *(.data)
    *(.data.*)
    *(.gnu.linkonce.d.*)
    CONSTRUCTORS
    *(.lit8)
    *(.lit4)
    *(.sdata)
    *(.sdata.*)
    *(.gnu.linkonce.s.*)
    . = ALIGN(16);
    __data_end = .;
  } > ram
  __data_size = __data_end - __data_start;
  __rom_core_end = LOADADDR(.data) + SIZEOF(.data);

  .bss :
  {
    __bss_start = .;
    *(.bss)
    *(.bss.*)
    *(.gnu.linkonce.b.*)
    *(.sbss)
    *(.sbss.*)
    *(.gnu.linkonce.sb.*)
    *(.scommon)
    *(COMMON)
    . = ALIGN(16);
    end = .;
    _end = end;
    __end = _end;
    __bss_end = .;
  } > ram
  __bss_size = __bss_end - __bss_start;

  .marker0 0x020FFFFC :
  AT (0x000FFFFC)
  {
    __marker0_start = .;
    LONG(0x424B3030)
    __marker0_end = .;
  } > marker0

  .rom_bank1 0x02100000 :
  AT (0x00100000)
  {
    __rom_bank1_start = .;
    __diagnostic_wram_pattern_start = .;
    KEEP(*(.rom_bank1.wram))
    __diagnostic_wram_pattern_end = .;
    __diagnostic_z80_payload_start = .;
    KEEP(*(.rom_bank1.z80))
    __diagnostic_z80_payload_end = .;
    *(.rom_bank1)
    *(.rom_bank1.*)
    __rom_bank1_end = .;
  } > rom_bank1

  .marker1 0x021FFFFC :
  AT (0x001FFFFC)
  {
    __marker1_start = .;
    LONG(0x424B3031)
    __marker1_end = .;
  } > marker1

  .rom_bank2 0x02200000 :
  AT (0x00200000)
  {
    __rom_bank2_start = .;
    *(.rom_bank2)
    *(.rom_bank2.*)
    __rom_bank2_end = .;
  } > rom_bank2

  .marker2 0x022FFFFC :
  AT (0x002FFFFC)
  {
    __marker2_start = .;
    LONG(0x424B3130)
    __marker2_end = .;
  } > marker2

  .rom_bank3_assets 0x02340000 :
  AT (0x00340000)
  {
    __rom_bank3_assets_start = .;
    *(.rom_bank3_assets)
    *(.rom_bank3_assets.*)
    __rom_bank3_assets_end = .;
  } > rom_bank3_assets

  .marker3 0x023FFFFC :
  AT (0x003FFFFC)
  {
    __marker3_start = .;
    LONG(0x424B3131)
    __marker3_end = .;
  } > marker3

  ASSERT(__rom_core_end <= 0x000FFFFC,
    "Core ROM overlaps the bank-0 marker")
  ASSERT((__marker0_end - __marker0_start) == 4,
    "Bank-0 marker must occupy four bytes")
  ASSERT((__marker1_end - __marker1_start) == 4,
    "Bank-1 marker must occupy four bytes")
  ASSERT((__marker2_end - __marker2_start) == 4,
    "Bank-2 marker must occupy four bytes")
  ASSERT((__marker3_end - __marker3_start) == 4,
    "Bank-3 marker must occupy four bytes")
  ASSERT(__rom_bank1_end <= 0x021FFFFC,
    "Bank-1 assets overlap the marker")
  ASSERT(__diagnostic_wram_pattern_start == 0x02100000,
    "Word-RAM pattern must begin at bank 1")
  ASSERT((__diagnostic_wram_pattern_end -
    __diagnostic_wram_pattern_start) == 0x00010000,
    "Word-RAM pattern must occupy 64 KiB")
  ASSERT(__diagnostic_z80_payload_start == 0x02110000,
    "Z80 payload must follow the Word-RAM pattern")
  ASSERT((__diagnostic_z80_payload_end -
    __diagnostic_z80_payload_start) == 0x00000200,
    "Z80 payload must occupy 512 bytes")
  ASSERT(__rom_bank2_end <= 0x022FFFFC,
    "Bank-2 assets overlap the marker")
  ASSERT(__rom_bank3_assets_start >= 0x02340000,
    "A section overlaps the Genesis reservation")
  ASSERT(__rom_bank3_assets_end <= 0x023FFFFC,
    "Bank-3 assets overlap the marker")
  ASSERT(__bss_end <= 0x0600F000,
    "Writable SDRAM overlaps the reserved SDRAM test window")
  ASSERT(__diag_secondary_end <= __stack,
    "Diagnostic buffers overlap the SH-2 stack reservation")
}
