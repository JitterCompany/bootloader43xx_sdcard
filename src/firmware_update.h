#ifndef FIRMWARE_UPDATE_H
#define FIRMWARE_UPDATE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

enum FirmwareResult {
    FIRMWARE_RESULT_ERROR = -1,
    FIRMWARE_RESULT_NOTHING_TO_DO = 0,
    FIRMWARE_RESULT_OK = 1
};
enum FirmwareResult firmware_update(const char *filename,
        const uint32_t flash_addr_begin, const size_t flash_max_size);

#endif

