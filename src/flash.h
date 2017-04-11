#ifndef FLASH_H
#define FLASH_H

/**
 * Simple flash IAP API.
 *
 * The workflow is very simple: 
 *
 * 1. Call flash_erase(begin, end) with the address range you want to use.
 *      Note that only a valid flash address range can be used that does not
 *      overlap the bootloader flash area and starts at a 512-byte boundary.
 *
 * 2. Call flash_append(data, sizeof(data)) successively untill all data is
 *      written to flash. sizeof(data) should be 512, 1024 or 4096.
 *
 * Note: this API is blocking, interrupts are temporarily disabled during the
 * API calls (which is a requirement for IAP, unless the IRQ vector is remapped
 * to RAM).
 */

/**
 * Erase all flash from start_addr up to end_addr,
 * or up to the next sector boundary after end_addr
 * if end_addr is not aligned on a sector boundary.
 *
 * NOTE: start_addr should be 512-byte aligned!
 *
 */
bool flash_erase(uint32_t start_addr, uint32_t end_addr);

/**
 * Append a block of data to the flash. Before writing the first block,
 * call flash_erase() with the appropriate address range.
 *
 * The first append() call saves data to the start_addr as passed
 * to the previous flash_erase(). Each successive append() call appends the
 * data just after the last block.
 */
bool flash_append(uint8_t *new_data, size_t sizeof_new_data);

/**
 * Utility function to find the start address of the flash bank
 * of the given flash address.
 * Returns 0 if the given address is not a valid flash address.
 */
uint32_t flash_start_from_addr(uint32_t address);
#endif

