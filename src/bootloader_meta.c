#include "generated/bootloader_version.h"
#include "bootloader.h"

// startup linker script puts .meta data right after the section table
static volatile const BootloaderMetadata meta \
                    __attribute__((section(".meta"))) = {
    .valid      = BOOTLOADER_VALID,
    .version    = BOOTLOADER_VERSION,
};

