// flash_utils.c

#include <zephyr.h>
#include <device.h>
#include <drivers/flash.h>
#include <drivers/i2s.h>
#include <logging/log.h>
#include <SEGGER_RTT.h>
#include "flash_utils.h"

LOG_MODULE_REGISTER(flash_utils);

int upload_audio_to_flash(const struct device *flash_dev, int rtt_index, off_t offset, size_t max_size, size_t buffer_size, size_t *total_size)
{
    uint8_t buffer[BUFFER_SIZE];
    size_t bytes_received;
    size_t total_written = 0;

    while (total_written < max_size) {
        bytes_received = SEGGER_RTT_Read(rtt_index, buffer, buffer_size);

        if (bytes_received > 0) {
            // Check for end-of-data signal
            if (bytes_received == 3 && memcmp(buffer, "END", 3) == 0) {
                LOG_INF("End of data received.");
                break;
            }

            // Write to flash
            int ret = flash_write(flash_dev, offset + total_written, buffer, bytes_received);
            if (ret != 0) {
                LOG_ERR("Flash write failed at offset 0x%08x: %d", (uint32_t)(offset + total_written), ret);
                return ret;
            }

            total_written += bytes_received;
            LOG_INF("Received and wrote %d bytes. Total: %d bytes.", bytes_received, total_written);
        } else {
            k_sleep(K_MSEC(10));
        }
    }

    *total_size = total_written;
    return 0;
}

int play_audio_from_flash(const struct device *flash_dev, off_t offset, size_t audio_size, size_t buffer_size)
{
    uint8_t buffer[BUFFER_SIZE];
    const struct device *i2s_dev;
    int ret;

    i2s_dev = device_get_binding(DT_LABEL(DT_NODELABEL(i2s0)));
    if (!i2s_dev) {
        LOG_ERR("Cannot find I2S device");
        return -ENODEV;
    }

    struct i2s_config i2s_cfg = {
        .word_size = 16,
        .channels = 1,
        .format = I2S_FMT_DATA_FORMAT_I2S,
        .options = I2S_OPT_FRAME_CLK_MASTER | I2S_OPT_BIT_CLK_MASTER,
        .frame_clk_freq = 22050,
        .block_size = buffer_size,
        .mem_slab = NULL,
        .timeout = 2000,
    };

    ret = i2s_configure(i2s_dev, I2S_DIR_TX, &i2s_cfg);
    if (ret != 0) {
        LOG_ERR("I2S configuration failed: %d", ret);
        return ret;
    }

    ret = i2s_trigger(i2s_dev, I2S_DIR_TX, I2S_TRIGGER_START);
    if (ret != 0) {
        LOG_ERR("I2S start failed: %d", ret);
        return ret;
    }

    size_t bytes_read = 0;
    while (bytes_read < audio_size) {
        size_t read_size = MIN(buffer_size, audio_size - bytes_read);

        ret = flash_read(flash_dev, offset + bytes_read, buffer, read_size);
        if (ret != 0) {
            LOG_ERR("Flash read failed at offset 0x%08x: %d", (uint32_t)(offset + bytes_read), ret);
            break;
        }

        struct i2s_buf buf = {
            .buf = buffer,
            .len = read_size,
            .timeout = K_FOREVER,
        };

        ret = i2s_write(i2s_dev, &buf);
        if (ret != 0) {
            LOG_ERR("I2S write failed: %d", ret);
            break;
        }

        bytes_read += read_size;
    }

    i2s_trigger(i2s_dev, I2S_DIR_TX, I2S_TRIGGER_STOP);
    return 0;
}
