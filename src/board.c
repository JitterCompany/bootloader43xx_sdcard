#include "board.h"
#include <chip.h>

static const PINMUX_GRP_T pinmuxing[] = {

        // Board LEDs 
        {1, 15, (SCU_MODE_FUNC0)}, //green
        {1, 16, (SCU_MODE_FUNC0)}, //blue
        {1, 17, (SCU_MODE_FUNC0)}, //red
        {1, 18, (SCU_MODE_FUNC0)}, //yellow
};

void board_setup_muxing(void)
{
    Chip_SCU_SetPinMuxing(pinmuxing, sizeof(pinmuxing) / sizeof(PINMUX_GRP_T));
}

void board_setup_GPIO(void)
{
    // blue
    Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, 0, 13);
    Chip_GPIO_SetPinState(LPC_GPIO_PORT, 0, 13, false);
    // red
    Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, 0, 2);
    Chip_GPIO_SetPinState(LPC_GPIO_PORT, 0, 2, false);
}

void board_setup_pins(void)
{
    board_setup_muxing();
    board_setup_GPIO();
}

