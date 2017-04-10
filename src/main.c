#include "board.h"

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "flash.h"
#include "flash_addresses.h"
#include <mbedtls/sha256.h>

#include <lpc_tools/boardconfig.h>
#include <lpc_tools/GPIO_HAL.h>
#include <mcu_timing/delay.h>

#include "sdcard.h"
#include "firmware_update.h"
// TODO rm
#include "hash.h"

unsigned int stack_value = 0xA5A55A5A;

#define BLINK_SLOW      (1500000)
#define BLINK_FAST      (500000)

static void blink(int count, int duration) {

    const GPIO *led = board_get_GPIO(GPIO_ID_LED_BLUE);
    for(int i=0;i<count;i++) {
        for(volatile int n=0;n<duration;n++){} 
        GPIO_HAL_set(led, HIGH);
        for(volatile int n=0;n<duration;n++){} 
        GPIO_HAL_set(led, LOW);
    }
}

static bool flash_demo(void)
{
    uint8_t src_data[512];
    for(size_t i=0;i<sizeof(src_data);i++) {
        src_data[i] = i;
    }
    
    uint8_t expected_hash[32];
    mbedtls_sha256(src_data, sizeof(src_data), expected_hash, 0);


    // erase complete M4 program section
    if(!flash_erase(FLASH_PROGRAM_M4_ADDR, FLASH_PROGRAM_M4_END_ADDR)) {
        return false;
    }
    
    // write some dummy data to M4 program section
    if(!flash_append((uint8_t*)src_data, sizeof(src_data))) {
        return false;
    }
    
    uint8_t hash[32];
    hash_flash(FLASH_PROGRAM_M4_ADDR, sizeof(src_data),
            hash, sizeof(hash));
     
    return hash_equal(expected_hash, sizeof(expected_hash),
            hash, sizeof(hash));
}


static bool sdcard_demo(void)
{
    return firmware_update("fw_m0.bin",
            FLASH_PROGRAM_M0_ADDR, FLASH_PROGRAM_M0_SIZE);
}

int main(void) {
    board_setup();

    const GPIO *led = board_get_GPIO(GPIO_ID_LED_RED);
    GPIO_HAL_set(led, HIGH);
    
    delay_init();
    sdcard_init(board_get_GPIO(GPIO_ID_SDCARD_POWER_ENABLE));
    sdcard_enable();

    fpuInit();
    
    blink(5, BLINK_SLOW);

    if(!sdcard_demo()) {
        while(1) {
            blink(1, BLINK_FAST);
        }
    }
/*
    if(!flash_demo()) {
        while(1) {
            blink(1, BLINK_FAST);
        }
    }
*/
    while(1) {
        blink(1, BLINK_SLOW);
    }
    return 0;
}

