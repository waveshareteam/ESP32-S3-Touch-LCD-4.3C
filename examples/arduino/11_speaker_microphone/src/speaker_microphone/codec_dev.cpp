/*****************************************************************************
 * | File         :   codec_dev.c
 * | Author       :   Waveshare team
 * | Function     :   Hardware underlying interface
 * | Info         :
 * |                 I2S driver code for I2S communication.
 * ----------------
 * | This version :   V1.0
 * | Date         :   2025-07-28
 * | Info         :   Basic version
 *
 ******************************************************************************/

#include "codec_dev.h"  // Include I2C driver header for I2C functions

static const char *TAG = "codec_dev";  // Define a tag for logging

static esp_codec_dev_handle_t play_dev_handle;
static esp_codec_dev_handle_t record_dev_handle;

static bool _is_audio_init = false;
static bool _is_player_init = false;
static int _vloume_intensity = CODEC_DEFAULT_VOLUME;
static int _gain_intensity = CODEC_DEFAULT_ADC_VOLUME;

/**************************************************************************************************
 *
 * Player Function
 *
 **************************************************************************************************/
esp_err_t mic_i2s_read(void *audio_buffer, size_t len, size_t *bytes_read, uint32_t timeout_ms)
{
    esp_err_t ret = ESP_OK;
    ret = esp_codec_dev_read(record_dev_handle, audio_buffer, len);
    *bytes_read = len;
    return ret;
}

esp_err_t speaker_i2s_write(void *audio_buffer, size_t len, size_t *bytes_written, uint32_t timeout_ms)
{
    esp_err_t ret = ESP_OK;
    ret = esp_codec_dev_write(play_dev_handle, audio_buffer, len);
    *bytes_written = len;
    return ret;
}

esp_err_t speaker_codec_set_fs(uint32_t rate, uint8_t bits_cfg, i2s_slot_mode_t ch)
{
    esp_err_t ret = ESP_OK;

    esp_codec_dev_sample_info_t fs = {
        .bits_per_sample = bits_cfg,
        .channel = ch,
        .sample_rate = rate,
    };

    if (play_dev_handle) {
        ret = esp_codec_dev_close(play_dev_handle);
    }
    if (record_dev_handle) {
        ret |= esp_codec_dev_close(record_dev_handle);
        ret |= esp_codec_dev_set_in_gain(record_dev_handle, CODEC_DEFAULT_ADC_VOLUME);
    }

    if (play_dev_handle) {
        ret |= esp_codec_dev_open(play_dev_handle, &fs);
    }

#if CODEC_DEFAULT_TDM     
    fs.channel = 4;
    fs.channel_mask = ESP_CODEC_DEV_MAKE_CHANNEL_MASK(0) | ESP_CODEC_DEV_MAKE_CHANNEL_MASK(3) | ESP_CODEC_DEV_MAKE_CHANNEL_MASK(1) | ESP_CODEC_DEV_MAKE_CHANNEL_MASK(2);
#endif   
    if (record_dev_handle) {
        ret |= esp_codec_dev_open(record_dev_handle, &fs);
    }
    return ret;
}

esp_err_t speaker_codec_volume_set(int volume, int *volume_set)
{
    ESP_RETURN_ON_ERROR(esp_codec_dev_set_out_vol(play_dev_handle, volume), TAG, "Set Codec volume failed");
    _vloume_intensity = volume;

    ESP_LOGI(TAG, "Setting volume: %d", volume);

    return ESP_OK;
}

esp_err_t microphone_codec_gain_set(int gain, int *gain_set)
{
    ESP_RETURN_ON_ERROR(esp_codec_dev_set_in_gain(record_dev_handle, gain), TAG, "Set Codec gain failed");
    _gain_intensity = gain;

    ESP_LOGI(TAG, "Setting gain: %d", gain);

    return ESP_OK;
}

int speaker_codec_volume_get(void)
{
    return _vloume_intensity;
}

int microphone_codec_gain_get(void)
{
    return _gain_intensity;
}

esp_err_t speaker_codec_mute_set(bool enable)
{
    esp_err_t ret = ESP_OK;
    ret = esp_codec_dev_set_out_mute(play_dev_handle, enable);
    return ret;
}

esp_err_t speaker_codec_dev_stop(void)
{
    esp_err_t ret = ESP_OK;

    if (play_dev_handle) {
        ret = esp_codec_dev_close(play_dev_handle);
    }

    if (record_dev_handle) {
        ret = esp_codec_dev_close(record_dev_handle);
    }
    return ret;
}

esp_err_t speaker_codec_dev_resume(void)
{
    return speaker_codec_set_fs(CODEC_DEFAULT_SAMPLE_RATE, CODEC_DEFAULT_BIT_WIDTH, (i2s_slot_mode_t)CODEC_DEFAULT_CHANNEL);
}

esp_err_t codec_init()
{
    if (_is_audio_init) {
        return ESP_OK;
    }

    play_dev_handle = speaker_init();
    assert((play_dev_handle) && "play_dev_handle not initialized");

    record_dev_handle = microphone_init();
    assert((record_dev_handle) && "record_dev_handle not initialized");

    speaker_codec_set_fs(CODEC_DEFAULT_SAMPLE_RATE, CODEC_DEFAULT_BIT_WIDTH, (i2s_slot_mode_t)CODEC_DEFAULT_CHANNEL);

    _is_audio_init = true;

    return ESP_OK;
}









