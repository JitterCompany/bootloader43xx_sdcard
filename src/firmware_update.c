#include <string.h>
#include <c_utils/static_assert.h>
#include "firmware_update.h"
#include "hash.h"
#include "flash.h"
#include "sdcard.h"

const uint8_t expected_header[] = "3853:sha256";
typedef struct {
    uint8_t header[11];
    uint8_t reserved[21];
    uint8_t sha256[32]; 
} FirmwareFooter;

STATIC_ASSERT(sizeof(FirmwareFooter) == 64);

static bool validate_file(const char *filename, size_t file_size)
{
    if(file_size < sizeof(FirmwareFooter)) {
        return false;
    }
    const size_t data_size = file_size - sizeof(FirmwareFooter);

    FIL file;
    if(!sdcard_open_file(&file, filename)) {
        return false;
    }

    // Read 64-byte footer from file
    FirmwareFooter footer;
    size_t result_size;
    const bool read_ok = sdcard_read_file_offset(&file, data_size,
            &footer, sizeof(footer), &result_size);
    sdcard_close_file(&file);
    if((!read_ok) || (result_size != sizeof(footer))) {
        return false;
    }

    // Assert that a proper footer is found
    for(size_t i=0;i<sizeof(footer.header);i++) {
        if(footer.header[i] != expected_header[i]) {
            return false;
        }
    }

    // Hash the data part of the file and check it against the footer
    uint8_t hash[32];
    if(!hash_file(filename, data_size, hash, sizeof(hash))) {
        return false;
    }
    if(!hash_equal(footer.sha256, sizeof(footer.sha256),
                hash, sizeof(hash))) {
        return false;
    }

    // File contains a proper and the footer hash matches the content
    return true;
}

static enum FirmwareResult update_required(const char *filename,
        const size_t file_size,
        const uint32_t flash_addr_begin)
{
    // failed to validate file: cannot update
    if(!validate_file(filename, file_size)) {
        return FIRMWARE_RESULT_ERROR;
    }

    // failed to hash file: cannot update
    uint8_t hash_of_file[32];
    if(!hash_file(filename, file_size,
                hash_of_file, sizeof(hash_of_file))) {
        return FIRMWARE_RESULT_ERROR;
    }

    uint8_t hash_of_flash[32];
    // failed to hash flash: cannot update
    if(!hash_flash(flash_addr_begin, file_size,
                hash_of_flash, sizeof(hash_of_flash))) {
        return FIRMWARE_RESULT_ERROR;
    }

    // file and flash match: no update required
    if(hash_equal(hash_of_file, sizeof(hash_of_file),
            hash_of_flash, sizeof(hash_of_flash))) {
        return FIRMWARE_RESULT_NOTHING_TO_DO;
    }

    return FIRMWARE_RESULT_OK;
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
    
    enum FirmwareResult should_update = update_required(filename,
            file_size, flash_addr_begin);
    if(should_update != FIRMWARE_RESULT_OK) {
        return should_update;
    }

    if(update(filename, file_size, flash_addr_begin, flash_max_size)) {
        return FIRMWARE_RESULT_OK;
    }
    
    return FIRMWARE_RESULT_ERROR;
}

