/*****************************************************************************
 * | File         :   codec_dev.h
 * | Author       :   Waveshare team
 * | Function     :   Hardware underlying interface
 * | Info         :
 * |                 
 * ----------------
 * | This version :   V1.0
 * | Date         :   2025-07-28
 * | Info         :   Basic version
 *
 ******************************************************************************/
#ifndef __CODEC_DEV_H
#define __CODEC_DEV_H

#include "speaker_microphone.h" 

#define CODEC_DEFAULT_SAMPLE_RATE           (44100)
#define CODEC_DEFAULT_BIT_WIDTH             (16)
#define CODEC_DEFAULT_ADC_VOLUME            (24.0)
#define CODEC_DEFAULT_CHANNEL               (2)
#define CODEC_DEFAULT_VOLUME                (60)
#define CODEC_DEFAULT_TDM                   (0)
/**
 * @brief Player set mute.
 *
 * @param enable: true or false
 *
 * @return
 *    - ESP_OK: Success
 *    - Others: Fail
 */
esp_err_t speaker_codec_mute_set(bool enable);

/**
 * @brief Player set volume.
 *
 * @param volume: volume set
 * @param volume_set: volume set response
 *
 * @return
 *    - ESP_OK: Success
 *    - Others: Fail
 */
esp_err_t speaker_codec_volume_set(int volume, int *volume_set);

/**
 * @brief Microphone set gain.
 *
 * @param gain: gain set
 * @param gain_set: gain set response
 *
 * @return
 *    - ESP_OK: Success
 *    - Others: Fail
 */
esp_err_t microphone_codec_gain_set(int gain, int *gain_set);

/** 
 * @brief Player get volume.
 * 
 * @return
 *   - volume: volume get
 */
int speaker_codec_volume_get(void);

/**
 * @brief Stop I2S function.
 *
 * @return
 *    - ESP_OK: Success
 *    - Others: Fail
 */
esp_err_t speaker_codec_dev_stop(void);

/**
 * @brief Resume I2S function.
 *
 * @return
 *    - ESP_OK: Success
 *    - Others: Fail
 */
esp_err_t speaker_codec_dev_resume(void);

/**
 * @brief Set I2S format to codec.
 *
 * @param rate: Sample rate of sample
 * @param bits_cfg: Bit lengths of one channel data
 * @param ch: Channels of sample
 *
 * @return
 *    - ESP_OK: Success
 *    - Others: Fail
 */
esp_err_t speaker_codec_set_fs(uint32_t rate, uint8_t bits_cfg, i2s_slot_mode_t ch);

/**
 * @brief Read data from recoder.
 *
 * @param audio_buffer: The pointer of receiving data buffer
 * @param len: Max data buffer length
 * @param bytes_read: Byte number that actually be read, can be NULL if not needed
 * @param timeout_ms: Max block time
 *
 * @return
 *    - ESP_OK: Success
 *    - Others: Fail
 */
esp_err_t mic_i2s_read(void *audio_buffer, size_t len, size_t *bytes_read, uint32_t timeout_ms);

/**
 * @brief Write data to player.
 *
 * @param audio_buffer: The pointer of sent data buffer
 * @param len: Max data buffer length
 * @param bytes_written: Byte number that actually be sent, can be NULL if not needed
 * @param timeout_ms: Max block time
 *
 * @return
 *    - ESP_OK: Success
 *    - Others: Fail
 */
esp_err_t speaker_i2s_write(void *audio_buffer, size_t len, size_t *bytes_written, uint32_t timeout_ms);


/**
 * @brief Initialize codec play and record handle.
 *
 * @return
 *      - ESP_OK: Success
 *      - Others: Fail
 */
esp_err_t codec_init();

#endif