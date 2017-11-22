#include "board_config.h"
#include "chip.h"
#include <c_utils/static_assert.h>
#include <lpc_tools/GPIO_HAL.h>
#include <lpc_tools/GPIO_HAL_LPC.h>

#include <stdint.h>
#include <stddef.h>

const uint32_t OscRateIn = 12000000;
const uint32_t ExtRateIn = 0;


void board_config_v2_setup(void);
void board_config_v5_setup(void);

static int version_hardware_get(void);

void board_setup(void)
{
    const int hw_version = version_hardware_get();

    switch(hw_version) {
        case 2:
        case 3:
        case 4:
            board_config_v2_setup();
            break;

        case 5:
        case 6:
            board_config_v5_setup();
            break;

        default:
            // unsupported board
            while(1);
    }
}

#define HW_NUM_VERSION_BITS (3)

static const PINMUX_GRP_T hw_bit_pinmuxing[] = {

        // Hardware version bits
        {6, 1, (SCU_MODE_FUNC0 
                | SCU_MODE_INBUFF_EN
                | SCU_MODE_PULLUP)},     // HW_VERSION_B0
        {2, 0, (SCU_MODE_FUNC4 
                | SCU_MODE_INBUFF_EN
                | SCU_MODE_PULLUP)},     // HW_VERSION_B1
        {2, 3, (SCU_MODE_FUNC4 
                | SCU_MODE_INBUFF_EN
                | SCU_MODE_PULLUP)},     // HW_VERSION_B2
};

STATIC_ASSERT(HW_NUM_VERSION_BITS == (sizeof(hw_bit_pinmuxing) / \
        sizeof(hw_bit_pinmuxing[0])));

static const GPIO hw_bit_pins[] = {
    // port , pin
    {3      , 0     },
    {5      , 0     },
    {5      , 3     }
};

STATIC_ASSERT(HW_NUM_VERSION_BITS == (sizeof(hw_bit_pins) / \
        sizeof(hw_bit_pins[0])));



static int version_hardware_get(void)
{
    // NOTE: this function should not have dependencies on board_get_GPIO etc
    // because it is needed very early during startup.
    // Therefore we need to do pinmuxing etc ourselves.

    Chip_SCU_SetPinMuxing(hw_bit_pinmuxing, HW_NUM_VERSION_BITS);

    int version = 0;

    for (size_t bit=0;bit<HW_NUM_VERSION_BITS;bit++) {

        const GPIO IO = hw_bit_pins[bit];
        Chip_GPIO_SetPinDIRInput(LPC_GPIO_PORT, IO.port, IO.pin);

        version|= ((!Chip_GPIO_GetPinState(LPC_GPIO_PORT,
                    IO.port, IO.pin)) << bit);
    }
    
    // v2 did not have the hardware bits yet, so the version bits will be 0.
    if(version == 0) {
       version = 2;
    } 
    return version;
}

