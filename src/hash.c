#include "hash.h"
#include <mbedtls/sha256.h>
#include <c_utils/max.h>
#include <mcu_sdcard/sdcard.h>

bool hash_equal(const uint8_t *hash_a, const size_t sizeof_hash_a,
        const uint8_t *hash_b, const size_t sizeof_hash_b)
{
    if(sizeof_hash_a != sizeof_hash_b) {
       return false;
    } 
    
    const size_t size = sizeof_hash_a;

    // Constant-time compare.
    // Usefull in case this hash is used in a security context.
    volatile uint8_t diff = 0;
    for(size_t i=0;i<size;i++) {
        diff|= (((volatile uint8_t*)hash_a)[i]
                ^ ((volatile uint8_t*)hash_b)[i]);
    }

    return (diff == 0);
}

static void zeroize(void *ptr, size_t num_bytes)
{
    volatile uint8_t *p = ptr;
    while(num_bytes--) {
        *p++ = 0;
    }
}

bool hash_file(const char *filename, const size_t file_size,
        uint8_t *result_hash, const size_t sizeof_result_hash)
{
    if(sizeof_result_hash != 32) {
        return false;
    }
    zeroize(result_hash, sizeof_result_hash);    

    sdcard_file file;
    if(!sdcard_open_file(&file, filename)) {
        return false;
    }

    size_t offset = 0;
    uint8_t buffer[4096];
    bool success = true;
   
    // start a new sha256 hash 
    mbedtls_sha256_context sha256;
    mbedtls_sha256_init(&sha256);
    mbedtls_sha256_starts(&sha256, 0);


    while(offset < file_size) {
        
        const size_t bytes_to_read = min(sizeof(buffer), file_size - offset);
        size_t bytes_read = 0;
        if(!sdcard_read_file_binary(&file,
                    buffer, bytes_to_read,
                    &bytes_read)) {
            success = false;
            break;
        }
        if(!bytes_read || (bytes_read > bytes_to_read)) {
            success = false;
            break;
        }
        
        // update the sha256 hash
        mbedtls_sha256_update(&sha256, buffer, bytes_read);

        offset+= bytes_read;
    }
    // finish sha256 hash
    mbedtls_sha256_finish(&sha256, result_hash);
    mbedtls_sha256_free(&sha256);

    sdcard_close_file(&file);
    return success;
}

bool hash_flash(uint32_t flash_addr, const size_t num_bytes_to_hash,
        uint8_t *result_hash, const size_t sizeof_result_hash)
{
    if(sizeof_result_hash != 32) {
        return false;
    }

    mbedtls_sha256((uint8_t*)flash_addr, num_bytes_to_hash,
            result_hash, 0);
    return true;
}

