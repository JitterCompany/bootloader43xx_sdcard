#ifndef FIRMWARE_UPDATE_H
#define FIRMWARE_UPDATE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

bool firmware_update(const char *filename,
        const uint32_t flash_addr_begin, const size_t flash_max_size);

#endif

