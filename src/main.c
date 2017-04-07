#include <chip.h>
#include "board.h"

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <lpc_tools/irq.h>

unsigned int stack_value = 0xA5A55A5A;

#define BLINK_SLOW      (1500000)
#define BLINK_FAST      (500000)

static void blink(int count, int duration) {
    for(int i=0;i<count;i++) {
        for(volatile int n=0;n<duration;n++){} 
        Chip_GPIO_SetPinState(LPC_GPIO_PORT, 0, 13, true);
        for(volatile int n=0;n<duration;n++){} 
        Chip_GPIO_SetPinState(LPC_GPIO_PORT, 0, 13, false);
    }
}

#define SECTOR_FIRST        (14)
#define SECTOR_LAST         (14)
#define SECTOR_ADDR         (0x1A070000)
#define FLASH_BANK          (IAP_FLASH_BANK_A)
#define NUM_BYTES_TO_WRITE  (512)

static volatile uint32_t src_data[NUM_BYTES_TO_WRITE/sizeof(uint32_t)];
static bool flash_demo(void)
{
    const size_t num_elems = (sizeof(src_data)/sizeof(src_data[0]));
    for(size_t i=0;i<num_elems;i++) {
        src_data[i] = 0xC0DE0000 + i;
    }

    // init IAP    
    if(IAP_CMD_SUCCESS != Chip_IAP_Init()) {
       return false;
    }
    
    // prepare flash
    if(IAP_CMD_SUCCESS != Chip_IAP_PreSectorForReadWrite(SECTOR_FIRST,
                SECTOR_LAST, 
                FLASH_BANK)) {
        return false;
    }
   
    // erase flash
    if(IAP_CMD_SUCCESS != Chip_IAP_EraseSector(SECTOR_FIRST,
                SECTOR_LAST,
                FLASH_BANK)) {
       return false;
    }

    // prepare flash
    if(IAP_CMD_SUCCESS != Chip_IAP_PreSectorForReadWrite(SECTOR_FIRST,
                SECTOR_LAST, 
                FLASH_BANK)) {
        return false;
    }
    
    // write flash
    if(IAP_CMD_SUCCESS != Chip_IAP_CopyRamToFlash(SECTOR_ADDR,
                (uint32_t*)src_data,
                NUM_BYTES_TO_WRITE)) {
        return false;
    }

    // TODO verify flash is correct.
    // Maybe use internal signature generation (run from RAM!) or sha256?
    return true;
}

int main(void) {
    board_setup_pins();
    Chip_GPIO_SetPinState(LPC_GPIO_PORT, 0, 2, true);

    board_setup_clock();

    fpuInit();
    
    blink(5, BLINK_SLOW);

    // begin critical section: no interrupts allowed during flash programming
    const bool prev_irq_state = irq_disable();

    if(!flash_demo()) {
        while(1) {
            blink(1, BLINK_FAST);
        }
    }
    // end critical section
    irq_restore(prev_irq_state);


    while(1) {
        blink(1, BLINK_SLOW);
    }
    return 0;
}

