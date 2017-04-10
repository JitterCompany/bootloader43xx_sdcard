#ifndef BOARD_H
#define BOARD_H

void board_setup(void);


enum GPIO_ID {
    GPIO_ID_LED_BLUE,
    GPIO_ID_LED_RED,

    GPIO_ID_EXT_LED_GREEN,
    GPIO_ID_EXT_LED_RED,

    GPIO_ID_SDCARD_POWER_ENABLE,

    GPIO_ID_MAX // This should be last: it is used to count
};


#endif

