# bootloader43xx_sdcard
SDcard based bootloader for LPC 43xx series microcontrollers

The basic idea is to have an sdcard with some firmware file(s).
The bootloader is capable of detecting if the current version on the LPC43xx flash memory is equal to the SDcard version.
If it differs, the file from SD card is written to flash memory. On the next boot, the bootloader will detect the flash is up to date and starts running the main firmware.

The main firmware itself should have some capability of downloading firmware updates to the SD card. It can then decide to reboot and let this bootloader update the firmware.
This keeps the bootloader simple and allows a lot of flexibility in the update process.


## Board config

To set up the bootloader for a new project, copy board_config.cmake.exampe to board_config.cmake and edit it: set BOARD to BOARD_<my-board-id>.

Add your custom board config in the folder src/board_config/<my-board-id>.
CMake should automatically find any code in that folder and build it.

