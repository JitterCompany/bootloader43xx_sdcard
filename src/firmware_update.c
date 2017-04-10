#include "firmware_update.h"
#include "hash.h"
#include "flash.h"
#include <string.h>

static bool update_required(const char *filename, const size_t file_size,
        const uint32_t flash_addr_begin)
{
    // failed to hash file: cannot update
    uint8_t hash_of_file[32];
    if(!hash_file(filename, file_size,
                hash_of_file, sizeof(hash_of_file))) {
        return false;
    }

    uint8_t hash_of_flash[32];
    // failed to hash flash: cannot update
    if(!hash_flash(flash_addr_begin, file_size,
                hash_of_flash, sizeof(hash_of_flash))) {
        return false;
    }

    // file and flash match: no update required
    if(hash_equal(hash_of_file, sizeof(hash_of_file),
            hash_of_flash, sizeof(hash_of_flash))) {
        return false;
    }

    return true;
}

static bool update(const char *filename, const size_t file_size,
        const uint32_t flash_addr_begin, const size_t flash_max_size)
{
    sdcard_file file;
    if(!sdcard_open_file(&file, filename)) {
        return false;
    }

    // erase complete target flash section
    if(!flash_erase(flash_addr_begin, flash_addr_begin + flash_max_size)) {
        sdcard_close_file(&file);
        return false;
    }

    size_t offset = 0;
    uint8_t buffer[4096];
    bool success = true;
   
    while(offset < file_size) {

        size_t bytes_read = 0;
        if(!sdcard_read_file_binary(&file,
                    buffer, sizeof(buffer),
                    &bytes_read)) {
            success = false;
            break;
        }
        if(!bytes_read || (bytes_read > sizeof(buffer))) {
            success = false;
            break;
        }
        
        if(bytes_read < sizeof(buffer)) {
            memset(buffer+bytes_read, 0xFF, sizeof(buffer) - bytes_read);
        }
        offset+= bytes_read;
        
        // append current buffer to flash
        if(!flash_append(buffer, sizeof(buffer))) {
            success = false;
            break;
        }
    }

    sdcard_close_file(&file);
    return success;
}

enum FirmwareResult firmware_update(const char *filename,
        const uint32_t flash_addr_begin, const size_t flash_max_size)
{
    const size_t file_size = sdcard_file_size(filename);
    if(!file_size || file_size > flash_max_size) {
        return FIRMWARE_RESULT_ERROR;
    }
    
    if(!update_required(filename, file_size, flash_addr_begin)) {
        return FIRMWARE_RESULT_NOTHING_TO_DO;
    }

    if(update(filename, file_size, flash_addr_begin, flash_max_size)) {
        return FIRMWARE_RESULT_UPDATED;
    }
    
    return FIRMWARE_RESULT_ERROR;
}

