#ifndef HASH_H
#define HASH_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * Test if two hashes are equal.
 *
 * NOTE: this is done in constant-time to avoid leaking data via
 * timing side-channel.
 */
bool hash_equal(const uint8_t *hash_a, const size_t sizeof_hash_a,
        const uint8_t *hash_b, const size_t sizeof_hash_b);

/**
 * Hash a file up to the specified size
 */
bool hash_file(const char *filename, const size_t file_size,
        uint8_t *result_hash, const size_t sizeof_result_hash);

/**
 * Hash the specified flash area
 */
bool hash_flash(uint32_t flash_addr, const size_t num_bytes_to_hash,
        uint8_t *result_hash, const size_t sizeof_result_hash);

#endif

