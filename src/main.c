#include "board.h"

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <lpc_tools/boardconfig.h>
#include <lpc_tools/GPIO_HAL.h>
#include <mcu_timing/delay.h>

#include "flash.h"
#include "flash_addresses.h"

#include "string.h"
#include "sdcard.h"
#include "firmware_update.h"

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
    enum FirmwareResult result = firmware_update(filename,
            flash_addr_begin, flash_max_size);

    if(result == FIRMWARE_RESULT_ERROR) {
        update_state.error_count+= 1;

    } else if(result == FIRMWARE_RESULT_UPDATED) {
        update_state.updated = true;
    }
}

typedef void (*UserProgram)(void);

static void run_user_program(void)
{
    // stack pointer is at offset 0 from vector table
    const uint32_t user_stack = *((volatile uint32_t*)FLASH_PROGRAM_M4_ADDR);

    // reset function is at offset 4 from vector table
    const UserProgram user_program = 
        *((volatile UserProgram*)(FLASH_PROGRAM_M4_ADDR+4));

    // Use the vector table from the user program
    uint32_t flash_offset = (FLASH_PROGRAM_M4_ADDR
        - flash_start_from_addr(FLASH_PROGRAM_M4_ADDR));
    SCB->VTOR = flash_offset;

    // set stack pointer to program stack
    __set_MSP(user_stack);

    // boot user program
    user_program();
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
        // reset mcu
        delay_us(200000);
        Chip_RGU_TriggerReset(RGU_CORE_RST);
    }
    
    GPIO_HAL_set(led, LOW);
    
    // NOTE: disable all interrupts that this bootloader has enabled
    // to avoid unexpected calling of user IRQHandlers
    sdcard_disable();
    delay_deinit();

    // run the user program: it enters its own loop and should never return
    run_user_program();

    return 0;
}

