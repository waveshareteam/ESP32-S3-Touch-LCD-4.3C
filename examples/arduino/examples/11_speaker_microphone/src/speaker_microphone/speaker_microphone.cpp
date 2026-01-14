/*****************************************************************************
 * | File         :   speaker_microphone.c
 * | Author       :   Waveshare team
 * | Function     :   Hardware underlying interface
 * | Info         :   I2S + I2C based audio codec initialization for speaker and microphone
 * ----------------
 * | This version :   V1.0
 * | Date         :   2025-07-28
 * | Info         :   Basic version
 *
 ******************************************************************************/

#include "speaker_microphone.h"  // Include header for speaker and microphone interface
#include "codec_dev.h"

static const char *TAG = "speaker_microphone";  // Logging tag for ESP log output

// Static handles for I2S channels
static i2s_chan_handle_t i2s_tx_chan = NULL;
static i2s_chan_handle_t i2s_rx_chan = NULL;

// Pointer to I2S codec data interface
static const audio_codec_data_if_t *i2s_data_if = NULL;

// I2C bus handle and init flag
static i2c_master_bus_handle_t i2c_handle = NULL;
static bool i2c_initialized = false;

/**************************************************************************************************
 *
 * I2C Initialization Function
 *
 **************************************************************************************************/

// Initialize I2C interface for codec communication
esp_err_t codec_i2c_init()
{
    if (i2c_initialized)
    {
        // Already initialized
        return ESP_OK;
    }

    // Get I2C handle from device-level function
    i2c_handle = DEV_I2C_Get_Bus_Device();
    i2c_initialized = true;

    return ESP_OK;
}

/**************************************************************************************************
 *
 * I2S Initialization Function
 *
 **************************************************************************************************/

// Initialize I2S interface with optional custom configuration
esp_err_t codec_audio_init()
{
    esp_err_t ret = ESP_FAIL;

    // Return if already initialized
    if (i2s_tx_chan && i2s_rx_chan)
    {
        return ESP_OK;
    }

    // Default I2S channel configuration
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM, I2S_ROLE_MASTER);
    chan_cfg.auto_clear = true;  // Clear DMA buffer on stop
    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, &i2s_tx_chan, &i2s_rx_chan));

    // Use default configuration if none provided
#if CODEC_DEFAULT_TDM 
    const i2s_tdm_slot_mask_t slot_mask = I2S_TDM_SLOT0 | I2S_TDM_SLOT1 | I2S_TDM_SLOT2 | I2S_TDM_SLOT3;
    i2s_tdm_config_t tdm_cfg_default = I2S_DUPLEX_MONO_CFG_TDM(CODEC_DEFAULT_SAMPLE_RATE,slot_mask);
    tdm_cfg_default.slot_cfg.total_slot = 4;
    const i2s_tdm_config_t *p_i2s_cfg = &tdm_cfg_default;
#else
    const i2s_std_config_t std_cfg_default = I2S_DUPLEX_MONO_CFG(CODEC_DEFAULT_SAMPLE_RATE);
    const i2s_std_config_t *p_i2s_cfg = &std_cfg_default;
#endif  

    // Initialize TX channel
    if (i2s_tx_chan)
    { 
#if CODEC_DEFAULT_TDM 
    ESP_GOTO_ON_ERROR(i2s_channel_init_tdm_mode(i2s_tx_chan, p_i2s_cfg), err, TAG, "TX channel init failed");
#else
    ESP_GOTO_ON_ERROR(i2s_channel_init_std_mode(i2s_tx_chan, p_i2s_cfg), err, TAG, "TX channel init failed");
#endif 
        ESP_GOTO_ON_ERROR(i2s_channel_enable(i2s_tx_chan), err, TAG, "TX enable failed");
    }

    // Initialize RX channel
    if (i2s_rx_chan)
    {
#if CODEC_DEFAULT_TDM 
    ESP_GOTO_ON_ERROR(i2s_channel_init_tdm_mode(i2s_rx_chan, p_i2s_cfg), err, TAG, "TX channel init failed");
#else
    ESP_GOTO_ON_ERROR(i2s_channel_init_std_mode(i2s_rx_chan, p_i2s_cfg), err, TAG, "TX channel init failed");
#endif 
        ESP_GOTO_ON_ERROR(i2s_channel_enable(i2s_rx_chan), err, TAG, "RX enable failed");
    }

    // Create I2S data interface for codec
    audio_codec_i2s_cfg_t i2s_cfg;
    i2s_cfg = {
        .port = I2S_NUM,
        .rx_handle = i2s_rx_chan,
        .tx_handle = i2s_tx_chan,
    };
    i2s_data_if = audio_codec_new_i2s_data(&i2s_cfg);
    assert(i2s_data_if);

    return ESP_OK;

err:
    if (i2s_tx_chan) i2s_del_channel(i2s_tx_chan);
    if (i2s_rx_chan) i2s_del_channel(i2s_rx_chan);

    return ret;
}

/**************************************************************************************************
 *
 * Speaker Related Functions
 *
 **************************************************************************************************/

// Enable or disable speaker power amplifier
esp_err_t speaker_audio_poweramp_enable(bool enable)
{
    IO_EXTENSION_Output(IO_EXTENSION_IO_3, enable);
    return ESP_OK;
}

// Initialize the speaker driver and configure ES8311 codec
esp_codec_dev_handle_t speaker_init(void)
{
    if (i2s_data_if == NULL) {
        // Initialize I2C and I2S if not already done
        USER_ERROR_CHECK_RETURN_NULL(codec_i2c_init());
        USER_ERROR_CHECK_RETURN_NULL(codec_audio_init());
    }
    assert(i2s_data_if);

    // Enable power amplifier
    speaker_audio_poweramp_enable(true);

    // Create GPIO and I2C control interface
    const audio_codec_gpio_if_t *gpio_if = audio_codec_new_gpio();
    audio_codec_i2c_cfg_t i2c_cfg = {
        .port = EXAMPLE_I2C_MASTER_NUM,
        .addr = ES8311_CODEC_DEFAULT_ADDR,
        .bus_handle = i2c_handle,
    };
    const audio_codec_ctrl_if_t *i2c_ctrl_if = audio_codec_new_i2c_ctrl(&i2c_cfg);
    USER_NULL_CHECK(i2c_ctrl_if, NULL);

    // Configure ES8311 codec parameters
    esp_codec_dev_hw_gain_t gain = {
        .pa_voltage = 5,
        .codec_dac_voltage = 3.3,
    };

    es8311_codec_cfg_t es8311_cfg = {
        .ctrl_if = i2c_ctrl_if,
        .gpio_if = gpio_if,
        .codec_mode = ESP_CODEC_DEV_WORK_MODE_DAC,
        .pa_pin = GPIO_NUM_NC,
        .pa_reverted = false,
        .master_mode = false,
        .use_mclk = true,
        .digital_mic = false,
        .invert_mclk = false,
        .invert_sclk = false,
        .hw_gain = gain,
    };

    const audio_codec_if_t *es8311_dev = es8311_codec_new(&es8311_cfg);
    USER_NULL_CHECK(es8311_dev, NULL);

    // Register codec device
    esp_codec_dev_cfg_t codec_dev_cfg = {
        .dev_type = ESP_CODEC_DEV_TYPE_OUT,
        .codec_if = es8311_dev,
        .data_if = i2s_data_if,
    };

    return esp_codec_dev_new(&codec_dev_cfg);
}

/**************************************************************************************************
 *
 * Microphone Related Functions
 *
 **************************************************************************************************/

// Initialize the microphone driver and configure ES7210 codec
esp_codec_dev_handle_t microphone_init(void)
{
    if (i2s_data_if == NULL) {
        // Initialize I2C and I2S if not already done
        USER_ERROR_CHECK_RETURN_NULL(codec_i2c_init());
        USER_ERROR_CHECK_RETURN_NULL(codec_audio_init());
    }
    assert(i2s_data_if);

    // Configure I2C for ES7210 codec
    audio_codec_i2c_cfg_t i2c_cfg = {
        .port = EXAMPLE_I2C_MASTER_NUM,
        .addr = ES7210_CODEC_ADDR,
        .bus_handle = i2c_handle,
    };
    const audio_codec_ctrl_if_t *i2c_ctrl_if = audio_codec_new_i2c_ctrl(&i2c_cfg);
    USER_NULL_CHECK(i2c_ctrl_if, NULL);

    // Configure codec
    es7210_codec_cfg_t es7210_cfg = {
        .ctrl_if = i2c_ctrl_if,
        .mic_selected = ES7120_SEL_MIC1 | ES7120_SEL_MIC2 , /*!< Selected microphone */
#if CODEC_DEFAULT_TDM 
        .mic_selected |= ES7120_SEL_MIC3 | ES7120_SEL_MIC4, /*!< Selected microphone */
#endif
        
    };
    const audio_codec_if_t *es7210_dev = es7210_codec_new(&es7210_cfg);
    USER_NULL_CHECK(es7210_dev, NULL);

    // Register codec device
    esp_codec_dev_cfg_t codec_es7210_dev_cfg = {
        .dev_type = ESP_CODEC_DEV_TYPE_IN,
        .codec_if = es7210_dev,
        .data_if = i2s_data_if,
    };

    return esp_codec_dev_new(&codec_es7210_dev_cfg);
}
