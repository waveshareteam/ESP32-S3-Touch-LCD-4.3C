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
#include "speaker_microphone.h" 

static const char *TAG = "codec_dev";  // Define a tag for logging

static esp_codec_dev_handle_t play_dev_handle;
static esp_codec_dev_handle_t record_dev_handle;

static bool _is_audio_init = false;
static bool _is_player_init = false;
static int _vloume_intensity = CODEC_DEFAULT_VOLUME;
static int _gain_intensity = CODEC_DEFAULT_ADC_VOLUME;

static audio_player_cb_t audio_idle_callback = NULL;
static void *audio_idle_cb_user_data = NULL;
static char audio_file_path[128];

/**************************************************************************************************
 *
 * Player Function
 *
 **************************************************************************************************/

static esp_err_t audio_mute_function(AUDIO_PLAYER_MUTE_SETTING setting)
{
    // Volume saved when muting and restored when unmuting. Restoring volume is necessary
    // as es8311_set_voice_mute(true) results in voice volume (REG32) being set to zero.

    speaker_codec_mute_set(setting == AUDIO_PLAYER_MUTE ? true : false);

    // restore the voice volume upon unmuting
    if (setting == AUDIO_PLAYER_UNMUTE) {
        ESP_RETURN_ON_ERROR(esp_codec_dev_set_out_vol(play_dev_handle, _vloume_intensity), TAG, "Set Codec volume failed");
    }

    return ESP_OK;
}

static void audio_callback(audio_player_cb_ctx_t *ctx)
{
    if (audio_idle_callback) {
        ctx->user_ctx = audio_idle_cb_user_data;
        audio_idle_callback(ctx);
    }
}

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

esp_err_t speaker_codec_set_fs(uint32_t rate, uint32_t bits_cfg, i2s_slot_mode_t ch)
{
    esp_err_t ret = ESP_OK;

    esp_codec_dev_sample_info_t fs = {
        .sample_rate = rate,
        .channel = ch,
        .bits_per_sample = bits_cfg,
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
    return speaker_codec_set_fs(CODEC_DEFAULT_SAMPLE_RATE, CODEC_DEFAULT_BIT_WIDTH, CODEC_DEFAULT_CHANNEL);
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

    speaker_codec_set_fs(CODEC_DEFAULT_SAMPLE_RATE, CODEC_DEFAULT_BIT_WIDTH, CODEC_DEFAULT_CHANNEL);

    _is_audio_init = true;

    return ESP_OK;
}

esp_err_t speaker_player_init(void)
{
    if (_is_player_init) {
        return ESP_OK;
    }

    audio_player_config_t config = { .mute_fn = audio_mute_function,
                                     .write_fn = speaker_i2s_write,
                                     .clk_set_fn = speaker_codec_set_fs,
                                     .priority = 5
                                   };
    ESP_RETURN_ON_ERROR(audio_player_new(config), TAG, "audio_player_init failed");
    audio_player_callback_register(audio_callback, NULL);

    _is_player_init = true;

    return ESP_OK;
}

esp_err_t speaker_player_del(void)
{
    _is_player_init = false;

    ESP_RETURN_ON_ERROR(audio_player_delete(), TAG, "audio_player_delete failed");

    return ESP_OK;
}

esp_err_t speaker_file_instance_init(const char *path, file_iterator_instance_t **ret_instance)
{
    ESP_RETURN_ON_FALSE(path, ESP_FAIL, TAG, "path is NULL");
    ESP_RETURN_ON_FALSE(ret_instance, ESP_FAIL, TAG, "ret_instance is NULL");

    file_iterator_instance_t *file_iterator = file_iterator_new(path);
    ESP_RETURN_ON_FALSE(file_iterator, ESP_FAIL, TAG, "file_iterator_new failed, %s", path);

    *ret_instance = file_iterator;

    return ESP_OK;
}

esp_err_t speaker_player_play_index(file_iterator_instance_t *instance, int index)
{
    ESP_RETURN_ON_FALSE(instance, ESP_FAIL, TAG, "instance is NULL");

    ESP_LOGI(TAG, "play_index(%d)", index);
    char filename[128];
    int retval = file_iterator_get_full_path_from_index(instance, index, filename, sizeof(filename));
    ESP_RETURN_ON_FALSE(retval != 0, ESP_FAIL, TAG, "file_iterator_get_full_path_from_index failed");

    ESP_LOGI(TAG, "opening file '%s'", filename);
    FILE *fp = fopen(filename, "rb");
    ESP_RETURN_ON_FALSE(fp, ESP_FAIL, TAG, "unable to open file");

    ESP_LOGI(TAG, "Playing '%s'", filename);
    ESP_RETURN_ON_ERROR(audio_player_play(fp), TAG, "audio_player_play failed");

    memcpy(audio_file_path, filename, sizeof(audio_file_path));

    return ESP_OK;
}

esp_err_t speaker_player_play_file(const char *file_path)
{
    ESP_LOGI(TAG, "opening file '%s'", file_path);
    FILE *fp = fopen(file_path, "rb");
    ESP_RETURN_ON_FALSE(fp, ESP_FAIL, TAG, "unable to open file");

    ESP_LOGI(TAG, "Playing '%s'", file_path);
    ESP_RETURN_ON_ERROR(audio_player_play(fp), TAG, "audio_player_play failed");

    memcpy(audio_file_path, file_path, sizeof(audio_file_path));

    return ESP_OK;
}

void speaker_player_register_callback(audio_player_cb_t cb, void *user_data)
{
    audio_idle_callback = cb;
    audio_idle_cb_user_data = user_data;
}

bool speaker_player_is_playing_by_path(const char *file_path)
{
    return (strcmp(audio_file_path, file_path) == 0);
}

bool speaker_player_is_playing_by_index(file_iterator_instance_t *instance, int index)
{
    return (index == file_iterator_get_index(instance));
}










