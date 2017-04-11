
MEMORY
{
  /* Flash where the bootloader itself is stored
   * Note: the bootloader end address should be aligned to a sector boundary!
   * See User manual 6.5, table 31 for the sector start addresses.
   */
  Flash_M4 (rx) : 	ORIGIN = 0x1a000000, LENGTH = 0x8000

  /* flash space for the user program (M0 and M4 core)
   * */
  flash_program_M0 (rx) : 	ORIGIN = 0x1b000000, LENGTH = 0x80000
  flash_program_M4 (rx) : 	ORIGIN = 0x1a000000 + LENGTH(Flash_M4), LENGTH = 0x80000 - LENGTH(Flash_M4)

  /* RAM used by the bootloader */
  RAM_M4 (rwx) : 	ORIGIN = 0x10080000, LENGTH = 0x9FE0
  reserved_IAP(r) : ORIGIN = 0x10089FE0, LENGTH = 0x20
}


/* Symbols for use in code */

__flash_bootloader = ORIGIN(Flash_M4);
__top_flash_bootloader = ORIGIN(Flash_M4) + LENGTH(Flash_M4);

__flash_program_M0 = ORIGIN(flash_program_M0);
__top_flash_program_M0 = ORIGIN(flash_program_M0) + LENGTH(flash_program_M0);

__flash_program_M4 = ORIGIN(flash_program_M4);
__top_flash_program_M4 = ORIGIN(flash_program_M4) + LENGTH(flash_program_M4);

__top_RAM_M4 = ORIGIN(RAM_M4) + LENGTH(RAM_M4);

