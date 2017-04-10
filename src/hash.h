#ifndef HASH_H
#define HASH_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "sdcard.h"

bool hash_equal(const uint8_t *hash_a, const size_t sizeof_hash_a,
        const uint8_t *hash_b, const size_t sizeof_hash_b);

bool hash_file(const char *filename, const size_t file_size,
        uint8_t *result_hash, const size_t sizeof_result_hash);

bool hash_flash(uint32_t flash_addr, const size_t num_bytes_to_hash,
        uint8_t *result_hash, const size_t sizeof_result_hash);

#endif

