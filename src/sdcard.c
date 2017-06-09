#include <string.h>
#include "chip.h"
#include "sdcard.h"
#include <mcu_timing/delay.h>
#include <fatfs_lib/ff.h>

#include <mcu_timing/profile.h>

/* SDMMC card info structure */
mci_card_struct sdcardinfo;
static volatile int32_t sdio_wait_exit = 0;
/* buffer size (in byte) for R/W operations */

static FATFS Fatfs;	/* File system object */

static const GPIO *g_sdcard_power_en_pin;

/* Delay callback for timed SDIF/SDMMC functions */
static void sdmmc_waitms(uint32_t time)
{
    delay_us(time*1000);
    return;
}
/**
 * @brief	A better wait callback for SDMMC driven by the IRQ flag
 * @return	0 on success, or failure condition (-1)
 */
static uint32_t sdmmc_irq_driven_wait(void)
{
    uint32_t status;

    /* Wait for event, would be nice to have a timeout, but keep it  simple */
    while (sdio_wait_exit == 0) {}

    /* Get status and clear interrupts */
    status = Chip_SDIF_GetIntStatus(LPC_SDMMC);
    Chip_SDIF_ClrIntStatus(LPC_SDMMC, status);
    Chip_SDIF_SetIntMask(LPC_SDMMC, 0);

    return status;
}

/**
 * @brief	Sets up the SD event driven wakeup
 * @param	bits : Status bits to poll for command completion
 * @return	Nothing
 */
static void sdmmc_setup_wakeup(void *bits)
{
    uint32_t bit_mask = *((uint32_t *)bits);
    /* Wait for IRQ - for an RTOS, you would pend on an event here with a IRQ based wakeup. */
    NVIC_ClearPendingIRQ(SDIO_IRQn);
    sdio_wait_exit = 0;
    Chip_SDIF_SetIntMask(LPC_SDMMC, bit_mask);
    NVIC_EnableIRQ(SDIO_IRQn);
}


/**
 * @brief	SDIO controller interrupt handler
 * @return	Nothing
 */
void SDIO_IRQHandler(void)
{
    /* All SD based register handling is done in the callback
       function. The SDIO interrupt is not enabled as part of this
       driver and needs to be enabled/disabled in the callbacks or
       application as needed. This is to allow flexibility with IRQ
       handling for applicaitons and RTOSes. */
    /* Set wait exit flag to tell wait function we are ready. In an RTOS,
       this would trigger wakeup of a thread waiting for the IRQ. */
    NVIC_DisableIRQ(SDIO_IRQn);
    sdio_wait_exit = 1;
}

bool sdcard_read_lines(const char *filename, char *line_buf,
                       const uint32_t n, line_handler cb, void *cb_ctx)
{
    FIL file;
    bool success = false;
    FRESULT rc = f_open(&file, filename,  FA_OPEN_EXISTING | FA_READ);
    if (rc == FR_OK) {

        for (;;) {
            char *rbuf = f_gets(line_buf, n, &file);

            if (!rbuf) {
                break;
            }
            if(!cb(cb_ctx, rbuf)) {
                break;
            }
        }

        rc = f_close(&file);
        if (!rc) {
            success = true;
        }
    }
    return success;
}

bool sdcard_file_exists(const char *filename)
{
    sdcard_file fp;
    bool exists = sdcard_open_file(&fp, filename);

    if(exists) {
        sdcard_close_file(&fp);
    }
    return exists;    
}

size_t sdcard_file_size(const char *filename)
{
    sdcard_file fp;
    size_t result_size = 0;

    if(sdcard_open_file(&fp, filename)) {
        result_size = f_size(&fp);
        sdcard_close_file(&fp);
    }
    return result_size;
}

bool sdcard_open_file(sdcard_file *fp, const char *filename)
{
    FRESULT rc = f_open(fp, filename,  FA_OPEN_EXISTING | FA_READ);
    return !rc;
}

bool sdcard_close_file(sdcard_file *fp)
{
    FRESULT rc = f_close(fp);
    return !rc;
}

char *sdcard_read_file(char *str, int n, sdcard_file *fp)
{
    return f_gets(str, n, fp);
}

bool sdcard_read_file_binary(sdcard_file *fp,
        uint8_t *result, size_t sizeof_result,
        size_t *bytes_returned) {

     FRESULT rc = f_read (fp, result, sizeof_result, bytes_returned);
    return !rc;
}

bool sdcard_read_file_offset(sdcard_file *fp, size_t offset,
        void *result, size_t sizeof_result,
        size_t *bytes_returned) {

    FRESULT rc = f_lseek(fp, offset);
    if (rc != FR_OK) {
        return false;
    }

     rc = f_read (fp, result, sizeof_result, bytes_returned);
    return !rc;
}

bool sdcard_enable(void)
{
    // enable power
    GPIO_HAL_set(g_sdcard_power_en_pin, HIGH);

    // mount filesystem
    FRESULT rc = f_mount(&Fatfs, "", 0);
    return !rc;
}

bool sdcard_disable(void)
{
    //unmount
    FRESULT rc = f_mount(NULL, "", 0);

    // disable power
    GPIO_HAL_set(g_sdcard_power_en_pin, LOW);

    return !rc;
}

void sdcard_init(const GPIO *power_en_pin)
{
    g_sdcard_power_en_pin = power_en_pin;

    GPIO_HAL_set_dir(power_en_pin, GPIO_DIR_OUTPUT);

    memset(&sdcardinfo, 0, sizeof(sdcardinfo));
    sdcardinfo.card_info.evsetup_cb = sdmmc_setup_wakeup;
    sdcardinfo.card_info.waitfunc_cb = sdmmc_irq_driven_wait;
    sdcardinfo.card_info.msdelay_func = sdmmc_waitms;

    // The SDIO driver needs to know the SDIO clock rate
    Chip_SDIF_Init(LPC_SDMMC);


}

