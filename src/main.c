#include <chip.h>
#include "board.h"

unsigned int stack_value = 0xA5A55A5A;

int main(void) {
    board_setup_pins();
    Chip_GPIO_SetPinState(LPC_GPIO_PORT, 0, 2, true);
    fpuInit();
    
    while(1){
        for(volatile int n=0;n<10000000;n++){} 
        Chip_GPIO_SetPinState(LPC_GPIO_PORT, 0, 13, true);
        for(volatile int n=0;n<10000000;n++){} 
        Chip_GPIO_SetPinState(LPC_GPIO_PORT, 0, 13, false);
    }
    return 0;
}

