#ifndef SDCARD_H
#define SDCARD_H
#include <stdint.h>
#include <stdbool.h>
#include <fatfs_lib/ff.h>
#include <lpc_tools/GPIO_HAL.h>

typedef bool (*line_handler)(void *context, char *line);

typedef FIL sdcard_file;

void sdcard_init(const GPIO *power_en_pin);
bool sdcard_file_exists(const char *filename);
size_t sdcard_file_size(const char *filename);
bool sdcard_read_lines(const char *filename, char *line_buf,
                       const uint32_t n, line_handler cb, void *cb_ctx);

bool sdcard_enable(void);
bool sdcard_disable(void);

bool sdcard_open_file(sdcard_file *fp, const char *filename);
bool sdcard_close_file(sdcard_file *fp);
char *sdcard_read_file(char *str, int n, sdcard_file *fp);

bool sdcard_read_file_binary(sdcard_file *fp,
        uint8_t *result, size_t sizeof_result,
        size_t *bytes_returned);
bool sdcard_read_file_offset(sdcard_file *fp, size_t offset,
        void *result, size_t sizeof_result,
        size_t *bytes_returned);


#endif
