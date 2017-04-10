#ifndef FLASH_ADDRESSES_H
#define FLASH_ADDRESSES_H


// import linker symbols.
// Don't use these directly, see the defines below instead.
extern uint32_t * __flash_bootloader;
extern uint32_t * __top_flash_bootloader;

extern uint32_t * __flash_program_M0;
extern uint32_t * __top_flash_program_M0;

extern uint32_t * __flash_program_M4;
extern uint32_t * __top_flash_program_M4;

/*
 * linker symbols as constants
 *
 * Instead of using the linker symbols above directly, simply use these
 * constants. This way you don't need to to these weird casts inline.
 */
#define FLASH_BOOTLOADER_ADDR (((uint32_t)&__flash_bootloader))
#define FLASH_BOOTLOADER_END_ADDR (((uint32_t)&__top_flash_bootloader))

#define FLASH_PROGRAM_M0_ADDR       (((uint32_t)&__flash_program_M0))
#define FLASH_PROGRAM_M0_END_ADDR   (((uint32_t)&__top_flash_program_M0))
#define FLASH_PROGRAM_M0_SIZE       ((size_t)(FLASH_PROGRAM_M0_END_ADDR \
                                    - FLASH_PROGRAM_M0_ADDR))

#define FLASH_PROGRAM_M4_ADDR       (((uint32_t)&__flash_program_M4))
#define FLASH_PROGRAM_M4_END_ADDR   (((uint32_t)&__top_flash_program_M4))
#define FLASH_PROGRAM_M4_SIZE       ((size_t)(FLASH_PROGRAM_M4_END_ADDR \
                                    - FLASH_PROGRAM_M4_ADDR))

#endif

