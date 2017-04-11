#ifndef METADATA_H
#define METADATA_H

#include <stdint.h>
#include <c_utils/static_assert.h>

#define BOOTLOADER_VALID    0xC0DE3853
typedef struct {
    const uint32_t valid;          // Detect if a valid bootloader exists
    const char version[32];        // Null-terminated version string
    uint32_t reserved[55];
} BootloaderMetadata;

STATIC_ASSERT(sizeof(BootloaderMetadata) == 256);
#define BOOTLOADER_PTR ((const BootloaderMetadata *const)0x1A000114)
#endif

