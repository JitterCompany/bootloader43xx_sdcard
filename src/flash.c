#include <chip.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <c_utils/align.h>
#include <lpc_tools/irq.h>

#include "flash_addresses.h"


// start address of each flash bank
static const uint32_t flash_banks[] = {
    0x1A000000,
    0x1B000000
};

// sizes of the sectors: not all sectors are the same size!
static const size_t sector_sizes[] = {
0x2000,
0x2000,
0x2000,
0x2000,
0x2000,
0x2000,
0x2000,
0x2000,
0x10000,
0x10000,
0x10000,
0x10000,
0x10000,
0x10000,
0x10000,
};

#define NUM_SECTORS (sizeof(sector_sizes)/sizeof(sector_sizes[0]))

static struct {
    uint32_t current_addr;
    uint32_t end_addr;
    bool valid;
} ctx;

static bool IAP_prepare_and_erase(uint32_t start_sector,
        uint32_t end_sector, uint8_t flash_bank);
static bool IAP_prepare_and_write(uint32_t start_sector,
        uint32_t end_sector, uint8_t flash_bank,
        uint8_t *new_data, size_t sizeof_new_data);

static uint8_t flash_bank_from_addr(uint32_t address);
static bool address_overlaps_bootloader(uint32_t addr);
static bool sector_lookup(const uint32_t address, uint32_t *result_sector_id);
static bool valid_flash_range(uint32_t start_addr, uint32_t end_addr);

bool flash_erase(uint32_t start_addr, uint32_t end_addr)
{
    ctx.valid = false;

    // start address should be 512-byte aligned
    if(start_addr != (uint32_t)align((void*)start_addr, 512)) {
        return false;
    }

    if(!valid_flash_range(start_addr, end_addr)) {
        return false;
    }

    uint32_t start_sector, end_sector;
    if(!sector_lookup(start_addr, &start_sector)) {
        return false;
    }
    if(!sector_lookup(end_addr-1, &end_sector)) {
        return false;
    }

    const uint8_t flash_bank = flash_bank_from_addr(start_addr);

    // try to init IAP    
    if(IAP_CMD_SUCCESS != Chip_IAP_Init()) {
       return false;
    }
    
    if(!IAP_prepare_and_erase(start_sector, end_sector, flash_bank)) {
        return false;
    }
    
    ctx.current_addr = start_addr; 
    ctx.end_addr = end_addr;
    ctx.valid = true;

    return true;
}

bool flash_append(uint8_t *new_data, size_t sizeof_new_data)
{
    // flash should be erased first
    if(!ctx.valid) {
        return false;
    }

    // data should be word-aligned
    if(new_data != align(new_data, 4)) {
        return false;
    }
    
    // only these sizes are supported by IAP
    switch(sizeof_new_data) {
        case 512:
        case 1024:
        case 4096:
            break;
        default:
            return false;
    }
    
    const uint32_t end_addr = ctx.current_addr + sizeof_new_data;

    uint32_t start_sector, end_sector;
    if(!sector_lookup(ctx.current_addr, &start_sector)) {
        return false;
    }
    if(!sector_lookup(end_addr-1, &end_sector)) {
        return false;
    }

    const uint8_t flash_bank = flash_bank_from_addr(ctx.current_addr);

    if(!IAP_prepare_and_write(start_sector, end_sector, flash_bank,
                new_data, sizeof_new_data)) {
        return false;
    }

    return true;
}

static bool IAP_prepare_and_erase(uint32_t start_sector,
        uint32_t end_sector, uint8_t flash_bank)
{
    bool success = false;

    // begin critical section: no interrupts allowed during flash programming
    const bool prev_irq_state = irq_disable();

    if(IAP_CMD_SUCCESS == Chip_IAP_PreSectorForReadWrite(start_sector,
                end_sector,
                flash_bank)) {

        if(IAP_CMD_SUCCESS == Chip_IAP_EraseSector(start_sector,
                    end_sector,
                    flash_bank)) {
            success = true;
        }
    }

    // end critical section
    irq_restore(prev_irq_state);

    return success;
}

static bool IAP_prepare_and_write(uint32_t start_sector,
        uint32_t end_sector, uint8_t flash_bank,
        uint8_t *new_data, size_t sizeof_new_data)
{
    bool success = false;

    // begin critical section: no interrupts allowed during flash programming
    const bool prev_irq_state = irq_disable();

    if(IAP_CMD_SUCCESS == Chip_IAP_PreSectorForReadWrite(start_sector,
                end_sector, 
                flash_bank)) {
        if(IAP_CMD_SUCCESS == Chip_IAP_CopyRamToFlash(ctx.current_addr,
                    (uint32_t*)new_data,
                    sizeof_new_data)) {
            ctx.current_addr+= sizeof_new_data;
            success = true;
        }
    }
    
    // end critical section
    irq_restore(prev_irq_state);

    return success;
}

static uint8_t flash_bank_from_addr(uint32_t address) {
    return ((address >> 24) == 0x1A) ? IAP_FLASH_BANK_A : IAP_FLASH_BANK_B;
}

/**
 * Find the start address of a given flash address.
 * Returns 0 if the given address is not a valid flash address
 */
uint32_t flash_start_from_addr(uint32_t address) {
    switch((address >> 24)) {
        case 0x1A:
            return 0x1A000000;
        case 0x1B:
            return 0x1B000000;
        default:
          return 0;
    }
}

static bool address_overlaps_bootloader(uint32_t addr)
{
    return ((addr >= FLASH_BOOTLOADER_ADDR)
            && (addr < FLASH_BOOTLOADER_END_ADDR));
}

static bool sector_lookup(const uint32_t address, uint32_t *result_sector_id) {
    uint32_t sector_id = 0;
    
    uint32_t offset = flash_start_from_addr(address);
    for(;sector_id<NUM_SECTORS;sector_id++) {

        offset+= sector_sizes[sector_id];
        if(offset > address) {
            break;
        }
    }

    // address is out of the flash range
    if(sector_id >= NUM_SECTORS) {
        return false;
    }

    // adress is in the same sector as the bootloader
    if(address_overlaps_bootloader(offset)) {
        return false;
    }

    *result_sector_id = sector_id;
    return true;
}

static bool valid_flash_range(uint32_t start_addr, uint32_t end_addr)
{
    // end should be > start
    if(end_addr < start_addr) {
        return false;
    }

    // both addresses should be in flash range, same flash bank
    const uint32_t flash = flash_start_from_addr(start_addr);
    if(!flash || (flash != flash_start_from_addr(end_addr))) {
        return false;
    }
    
    // range should not overlap bootloader space
    if(address_overlaps_bootloader(start_addr)) {
       return false;
    } 

    return true;
}

