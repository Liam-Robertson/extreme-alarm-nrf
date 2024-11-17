// main.c

#include <zephyr.h>
#include <device.h>
#include <drivers/flash.h>
#include <drivers/i2s.h>
#include <logging/log.h>
#include <SEGGER_RTT.h>
#include "flash_utils.h"

LOG_MODULE_REGISTER(main);

#define SPI_FLASH_LABEL DT_LABEL(DT_NODELABEL(spi_flash))
#define RTT_BUFFER_INDEX 0
#define BUFFER_SIZE 256
#define FLASH_BASE_OFFSET 0x00000000
#define FLASH_SIZE (4 * 1024 * 1024)  // 4MB Flash size

void main(void)
{
    const struct device *flash_dev;
    int ret;
    size_t audio_data_size = 0;

    LOG_INF("Starting audio upload and playback...");

    // Initialize RTT
    SEGGER_RTT_Init();

    // Get SPI flash device binding
    flash_dev = device_get_binding(SPI_FLASH_LABEL);
    if (!flash_dev) {
        LOG_ERR("Cannot find SPI flash device");
        return;
    }

    // Erase flash memory
    LOG_INF("Erasing flash memory...");
    ret = flash_erase(flash_dev, FLASH_BASE_OFFSET, FLASH_SIZE);
    if (ret != 0) {
        LOG_ERR("Flash erase failed: %d", ret);
        return;
    }
    LOG_INF("Flash memory erased.");

    // Upload audio data via RTT
    LOG_INF("Uploading audio data...");
    ret = upload_audio_to_flash(flash_dev, RTT_BUFFER_INDEX, FLASH_BASE_OFFSET, FLASH_SIZE, BUFFER_SIZE, &audio_data_size);
    if (ret != 0) {
        LOG_ERR("Audio upload failed: %d", ret);
        return;
    }
    LOG_INF("Audio upload completed. Total size: %d bytes", audio_data_size);

    // Play audio from flash
    LOG_INF("Playing audio from flash...");
    ret = play_audio_from_flash(flash_dev, FLASH_BASE_OFFSET, audio_data_size, BUFFER_SIZE);
    if (ret != 0) {
        LOG_ERR("Audio playback failed: %d", ret);
    }
    LOG_INF("Audio playback completed.");
}
