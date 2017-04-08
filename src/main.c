#include <chip.h>
#include "board.h"

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "flash.h"
#include "flash_addresses.h"

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


#define SECTOR_ADDR         (0x1A070000)

static bool flash_demo(void)
{
    uint8_t src_data[512];
    for(size_t i=0;i<sizeof(src_data);i++) {
        src_data[i] = i;
    }

    // erase complete M4 program section
    if(!flash_erase(FLASH_PROGRAM_M4_ADDR, FLASH_PROGRAM_M4_END_ADDR)) {
        return false;
    }
    
    // write some dummy data to M4 program section
    if(!flash_append((uint8_t*)src_data, sizeof(src_data))) {
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

    if(!flash_demo()) {
        while(1) {
            blink(1, BLINK_FAST);
        }
    }

    while(1) {
        blink(1, BLINK_SLOW);
    }
    return 0;
}

