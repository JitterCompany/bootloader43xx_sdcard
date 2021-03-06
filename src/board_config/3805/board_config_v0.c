#include "board_config.h"
#include <chip.h>
#include <lpc_tools/boardconfig.h>
#include <c_utils/static_assert.h>

static const NVICConfig NVIC_config[] = {
    {TIMER2_IRQn,       0},     // delay timer: high priority to ensure
                                // timestamps are correct in any context 
    {SDIO_IRQn,         3},     // SD card: probably not timing sensitive
};

static const PINMUX_GRP_T pinmuxing[] = {

        // LEDs
        {1, 16, (SCU_MODE_FUNC0)},      // GPIO_ID_LED_BLUE
        {1, 17, (SCU_MODE_FUNC0)},      // GPIO_ID_LED_RED

        // BTN
        {7, 7, (SCU_MODE_FUNC0
                | SCU_MODE_INBUFF_EN
                | SCU_MODE_PULLUP)},

        // SD Card
        {1, 6, (SCU_MODE_FUNC7
                | SCU_MODE_INBUFF_EN
                | SCU_MODE_PULLUP)},    // SDCARD_CMD
        {1, 9, (SCU_MODE_FUNC7
                | SCU_MODE_INBUFF_EN
                | SCU_MODE_PULLUP)},    // SDCARD_DATA0
        {1, 10, (SCU_MODE_FUNC7
                | SCU_MODE_INBUFF_EN
                | SCU_MODE_PULLUP)},    // SDCARD_DATA1
        {1, 11, (SCU_MODE_FUNC7
                | SCU_MODE_INBUFF_EN
                | SCU_MODE_PULLUP)},    // SDCARD_DATA2
        {1, 12, (SCU_MODE_FUNC7
                | SCU_MODE_INBUFF_EN
                | SCU_MODE_PULLUP)},    // SDCARD_DATA3
};

static const GPIOConfig pin_config[] = {
    {{0, 3 }, GPIO_CFG_DIR_OUTPUT_LOW}, // GPIO_ID_LED_BLUE
    {{0, 12}, GPIO_CFG_DIR_OUTPUT_LOW}, // GPIO_ID_LED_RED

    {{0, 0 }, GPIO_CFG_DIR_INVALID},    // GPIO_ID_EXT_LED_GREEN
    {{0, 0 }, GPIO_CFG_DIR_INVALID},    // GPIO_ID_EXT_LED_RED
    {{3, 15}, GPIO_CFG_DIR_INPUT},      // GPIO_ID_EXT_BUTTON

    {{1, 8 }, GPIO_CFG_DIR_OUTPUT_HIGH},// GPIO_ID_SDCARD_POWER_ENABLE
};

// pin config struct should match GPIO_ID enum
STATIC_ASSERT( (GPIO_ID_MAX == (sizeof(pin_config)/sizeof(pin_config[0]))));


static const BoardConfig config = {
    .nvic_configs = NVIC_config,
    .nvic_count = sizeof(NVIC_config) / sizeof(NVIC_config[0]),

    .pinmux_configs = pinmuxing,
    .pinmux_count = sizeof(pinmuxing) / sizeof(pinmuxing[0]),
    
    .GPIO_configs = pin_config,
    .GPIO_count = sizeof(pin_config) / sizeof(pin_config[0])
};


void board_config_v0_setup(void)
{
    board_set_config(&config);

    Chip_SCU_ClockPinMuxSet(0, (SCU_PINIO_FAST | SCU_MODE_FUNC4)); //SD CLK

    board_setup_pins();

    // required for chip library to function properly
    SystemCoreClockUpdate();
}

