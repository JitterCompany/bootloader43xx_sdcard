#include "board_config.h"

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

#include "bootloader.h"

unsigned int stack_value = 0xA5A55A5A;


#define BLINK_SLOW      (400000)
#define BLINK_FAST      (100000)

static void try_set(const GPIO *pin, enum GPIO_state state)
{
    if(!pin) {
        return;
    }
    GPIO_HAL_set(pin, state);
}

static void blink(int count, int duration_us) {

    const GPIO *led_board = NULL;
    if(board_has_GPIO(GPIO_ID_LED_BLUE)) {
        led_board = board_get_GPIO(GPIO_ID_LED_BLUE);
    }
    const GPIO *led_ext = NULL;
    if(board_has_GPIO(GPIO_ID_EXT_LED_GREEN)) {
        led_ext = board_get_GPIO(GPIO_ID_EXT_LED_GREEN);
    }

    for(int i=0;i<count;i++) {
        delay_us(duration_us);
        try_set(led_board,     HIGH);
        try_set(led_ext,       HIGH);
        delay_us(duration_us);
        try_set(led_board,     LOW);
        try_set(led_ext,       LOW);
    }
}

static struct {
    unsigned int error_count;
    bool updated; 
} g_update_state;

static void check_fw(const char *filename,
        const uint32_t flash_addr_begin, const size_t flash_max_size)
{
    enum FirmwareResult result = firmware_update(filename,
            flash_addr_begin, flash_max_size);

    if(result == FIRMWARE_RESULT_ERROR) {
        g_update_state.error_count+= 1;

    } else if(result == FIRMWARE_RESULT_OK) {
        g_update_state.updated = true;
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

void fatal_error(void) {
    while(1) {
        blink(10, BLINK_FAST);
    }
}

int main(void) {
    board_setup();
    board_setup_NVIC();

    if(board_has_GPIO(GPIO_ID_LED_RED)) { 
        GPIO_HAL_set(board_get_GPIO(GPIO_ID_LED_RED),       HIGH);
    }
    if(board_has_GPIO(GPIO_ID_EXT_LED_RED)) { 
        GPIO_HAL_set(board_get_GPIO(GPIO_ID_EXT_LED_RED),   HIGH);
    }

    delay_init();
    bool skip_update = false;
    
    // skip update if the EXT_BUTTON GPIO exists and is pulled low
    if(board_has_GPIO(GPIO_ID_EXT_BUTTON)) {
        skip_update = !GPIO_HAL_get(board_get_GPIO(GPIO_ID_EXT_BUTTON));
    }

    sdcard_init(board_get_GPIO(GPIO_ID_SDCARD_POWER_ENABLE));
    if(!sdcard_enable()) {
        skip_update = true;
    }

    fpuInit();

    // Assert the bootloader metadata is sane.
    // If this ever fails, it is a firmware/linker file bug.
    if(BOOTLOADER_PTR->valid != BOOTLOADER_VALID) {
        fatal_error();
    }

    // slow blink on startup
    blink(2, BLINK_SLOW);

    memset(&g_update_state, 0, sizeof(g_update_state));

    if(skip_update) {
        blink(3, BLINK_FAST);
    } else {
        check_fw("fw_m4.bin", FLASH_PROGRAM_M4_ADDR, FLASH_PROGRAM_M4_SIZE);
        check_fw("fw_m0.bin", FLASH_PROGRAM_M0_ADDR, FLASH_PROGRAM_M0_SIZE);
    }

    // fast blinking if firmware could not be updated
    blink(10*g_update_state.error_count, BLINK_FAST);

    if(g_update_state.updated) {
        // slow blink after an update
        blink(5, BLINK_SLOW);

        // reset mcu
        delay_us(200000);
        Chip_RGU_TriggerReset(RGU_CORE_RST);
    }
    

    if(board_has_GPIO(GPIO_ID_LED_RED)) {
        GPIO_HAL_set(board_get_GPIO(GPIO_ID_LED_RED),       LOW);
    }
    if(board_has_GPIO(GPIO_ID_EXT_LED_RED)) {
        GPIO_HAL_set(board_get_GPIO(GPIO_ID_EXT_LED_RED),   LOW);
    }
    
    // NOTE: disable all interrupts that this bootloader has enabled
    // to avoid unexpected calling of user IRQHandlers
    sdcard_disable();
    delay_deinit();

    // run the user program: it enters its own loop and should never return
    run_user_program();

    return 0;
}

