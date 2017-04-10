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

static struct {
    unsigned int error_count;
    bool updated; 
} update_state;

static void check_fw(const char *filename,
        const uint32_t flash_addr_begin, const size_t flash_max_size)
{
    enum FirmwareResult result = firmware_update("fw_m0.bin",
            FLASH_PROGRAM_M0_ADDR, FLASH_PROGRAM_M0_SIZE);

    if(result == FIRMWARE_RESULT_ERROR) {
        update_state.error_count+= 1;

    } else if(result == FIRMWARE_RESULT_UPDATED) {
        update_state.updated = true;
    }
}


int main(void) {
    board_setup();

    const GPIO *led = board_get_GPIO(GPIO_ID_LED_RED);
    GPIO_HAL_set(led, HIGH);

    delay_init();
    sdcard_init(board_get_GPIO(GPIO_ID_SDCARD_POWER_ENABLE));
    sdcard_enable();

    fpuInit();

    blink(2, BLINK_SLOW);

    memset(&update_state, 0, sizeof(update_state));

    check_fw("fw_m4.bin", FLASH_PROGRAM_M4_ADDR, FLASH_PROGRAM_M4_SIZE);
    check_fw("fw_m0.bin", FLASH_PROGRAM_M0_ADDR, FLASH_PROGRAM_M0_SIZE);

    blink(10*update_state.error_count, BLINK_FAST);

    if(update_state.updated) {
        blink(10, BLINK_SLOW);
        // TODO reset mcu
    }
    
    GPIO_HAL_set(led, LOW);
    // TODO boot main firmware
    return 0;
}

