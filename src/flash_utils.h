// flash_utils.h

#ifndef FLASH_UTILS_H
#define FLASH_UTILS_H

#include <device.h>
#include <stddef.h>

int upload_audio_to_flash(const struct device *flash_dev, int rtt_index, off_t offset, size_t max_size, size_t buffer_size, size_t *total_size);
int play_audio_from_flash(const struct device *flash_dev, off_t offset, size_t audio_size, size_t buffer_size);

#endif // FLASH_UTILS_H
