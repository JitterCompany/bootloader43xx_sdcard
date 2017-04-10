#include <chip.h>
#include "board.h"

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "flash.h"
#include "flash_addresses.h"

#include <mbedtls/sha256.h>

#include <lpc_tools/boardconfig.h>

unsigned int stack_value = 0xA5A55A5A;

#define BLINK_SLOW      (1500000)
#define BLINK_FAST      (500000)

static void blink(int count, int duration) {

    const GPIO *led = board_get_GPIO(GPIO_ID_LED_BLUE);
    for(int i=0;i<count;i++) {
        for(volatile int n=0;n<duration;n++){} 
        Chip_GPIO_SetPinState(LPC_GPIO_PORT, led->port, led->pin, true);
        for(volatile int n=0;n<duration;n++){} 
        Chip_GPIO_SetPinState(LPC_GPIO_PORT, led->port, led->pin, false);
    }
}


static bool flash_demo(void)
{
    uint8_t src_data[512];
    for(size_t i=0;i<sizeof(src_data);i++) {
        src_data[i] = i;
    }
    
    uint8_t expected_hash[32];
    mbedtls_sha256(src_data, sizeof(src_data),
            expected_hash, 0);


    // erase complete M4 program section
    if(!flash_erase(FLASH_PROGRAM_M4_ADDR, FLASH_PROGRAM_M4_END_ADDR)) {
        return false;
    }
    
    // write some dummy data to M4 program section
    if(!flash_append((uint8_t*)src_data, sizeof(src_data))) {
        return false;
    }
    
    uint8_t hash[32];
    mbedtls_sha256((uint8_t*)FLASH_PROGRAM_M4_ADDR, sizeof(src_data),
            hash, 0);
     
    // Constant-time compare.
    // Usefull in case this hash is used in a security context.
    uint8_t diff = 0;
    for(size_t i=0;i<sizeof(hash);i++) {
        diff|= (expected_hash[i] ^ hash[i]);
    }
    
    return (diff == 0);
}

int main(void) {
    board_setup();
    const GPIO *led = board_get_GPIO(GPIO_ID_LED_RED);
    Chip_GPIO_SetPinState(LPC_GPIO_PORT, led->port, led->pin, true);

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

