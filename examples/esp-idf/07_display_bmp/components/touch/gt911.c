/*
 * SPDX-FileCopyrightText: 2015-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_check.h"

#include "i2c.h"
#include "gpio.h"
#include "io_extension.h"
#include "rgb_lcd_port.h"

#include "gt911.h"

static const char *TAG = "GT911";

/* GT911 registers */
#define ESP_LCD_TOUCH_GT911_READ_KEY_REG    (0x8093)
#define ESP_LCD_TOUCH_GT911_READ_XY_REG     (0x814E)
#define ESP_LCD_TOUCH_GT911_CONFIG_REG      (0x8047)
#define ESP_LCD_TOUCH_GT911_PRODUCT_ID_REG  (0x8140)
#define ESP_LCD_TOUCH_GT911_ENTER_SLEEP     (0x8040)

/* GT911 support key num */
#define ESP_GT911_TOUCH_MAX_BUTTONS         (4)

esp_lcd_touch_handle_t tp_handle = NULL; // Declare a handle for the touch panel
/*******************************************************************************
* Function definitions
*******************************************************************************/
static esp_err_t esp_lcd_touch_gt911_read_data(esp_lcd_touch_handle_t tp);
static bool esp_lcd_touch_gt911_get_xy(esp_lcd_touch_handle_t tp, uint16_t *x, uint16_t *y, uint16_t *strength, uint8_t *point_num, uint8_t max_point_num);
#if (ESP_LCD_TOUCH_MAX_BUTTONS > 0)
static esp_err_t esp_lcd_touch_gt911_get_button_state(esp_lcd_touch_handle_t tp, uint8_t n, uint8_t *state);
#endif
static esp_err_t esp_lcd_touch_gt911_del(esp_lcd_touch_handle_t tp);

/* I2C read/write */
static esp_err_t touch_gt911_i2c_read(esp_lcd_touch_handle_t tp, uint16_t reg, uint8_t *data, uint8_t len);
static esp_err_t touch_gt911_i2c_write(esp_lcd_touch_handle_t tp, uint16_t reg, uint8_t data);

/* GT911 reset */
static esp_err_t touch_gt911_reset(esp_lcd_touch_handle_t tp);
/* Read status and config register */
static esp_err_t touch_gt911_read_cfg(esp_lcd_touch_handle_t tp);

/* GT911 enter/exit sleep mode */
static esp_err_t esp_lcd_touch_gt911_enter_sleep(esp_lcd_touch_handle_t tp);
static esp_err_t esp_lcd_touch_gt911_exit_sleep(esp_lcd_touch_handle_t tp);

/*******************************************************************************
* Public API functions
*******************************************************************************/

esp_err_t esp_lcd_touch_new_i2c_gt911(const esp_lcd_panel_io_handle_t io, const esp_lcd_touch_config_t *config, esp_lcd_touch_handle_t *out_touch)
{
    esp_err_t ret = ESP_OK;

    assert(io != NULL);
    assert(config != NULL);
    assert(out_touch != NULL);

    /* Prepare main structure */
    esp_lcd_touch_handle_t esp_lcd_touch_gt911 = heap_caps_calloc(1, sizeof(esp_lcd_touch_t), MALLOC_CAP_DEFAULT);
    ESP_GOTO_ON_FALSE(esp_lcd_touch_gt911, ESP_ERR_NO_MEM, err, TAG, "no mem for GT911 controller");

    /* Communication interface */
    esp_lcd_touch_gt911->io = io;

    /* Only supported callbacks are set */
    esp_lcd_touch_gt911->read_data = esp_lcd_touch_gt911_read_data;
    esp_lcd_touch_gt911->get_xy = esp_lcd_touch_gt911_get_xy;
#if (ESP_LCD_TOUCH_MAX_BUTTONS > 0)
    esp_lcd_touch_gt911->get_button_state = esp_lcd_touch_gt911_get_button_state;
#endif
    esp_lcd_touch_gt911->del = esp_lcd_touch_gt911_del;
    esp_lcd_touch_gt911->enter_sleep = esp_lcd_touch_gt911_enter_sleep;
    esp_lcd_touch_gt911->exit_sleep = esp_lcd_touch_gt911_exit_sleep;

    /* Mutex */
    esp_lcd_touch_gt911->data.lock.owner = portMUX_FREE_VAL;

    /* Save config */
    memcpy(&esp_lcd_touch_gt911->config, config, sizeof(esp_lcd_touch_config_t));
    esp_lcd_touch_io_gt911_config_t *gt911_config = (esp_lcd_touch_io_gt911_config_t *)esp_lcd_touch_gt911->config.driver_data;

    /* Prepare pin for touch controller reset */
    if (esp_lcd_touch_gt911->config.rst_gpio_num != GPIO_NUM_NC) {
        const gpio_config_t rst_gpio_config = {
            .mode = GPIO_MODE_OUTPUT,
            .pin_bit_mask = BIT64(esp_lcd_touch_gt911->config.rst_gpio_num)
        };
        ret = gpio_config(&rst_gpio_config);
        ESP_GOTO_ON_ERROR(ret, err, TAG, "GPIO config failed");
    }

    if (gt911_config && esp_lcd_touch_gt911->config.rst_gpio_num != GPIO_NUM_NC && esp_lcd_touch_gt911->config.int_gpio_num != GPIO_NUM_NC) {
        /* Prepare pin for touch controller int */
        const gpio_config_t int_gpio_config = {
            .mode = GPIO_MODE_OUTPUT,
            .intr_type = GPIO_INTR_DISABLE,
            .pull_down_en = 0,
            .pull_up_en = 1,
            .pin_bit_mask = BIT64(esp_lcd_touch_gt911->config.int_gpio_num),
        };
        ret = gpio_config(&int_gpio_config);
        ESP_GOTO_ON_ERROR(ret, err, TAG, "GPIO config failed");

        ESP_RETURN_ON_ERROR(gpio_set_level(esp_lcd_touch_gt911->config.rst_gpio_num, esp_lcd_touch_gt911->config.levels.reset), TAG, "GPIO set level error!");
        ESP_RETURN_ON_ERROR(gpio_set_level(esp_lcd_touch_gt911->config.int_gpio_num, 0), TAG, "GPIO set level error!");
        vTaskDelay(pdMS_TO_TICKS(10));

        /* Select I2C addr, set output high or low */
        uint32_t gpio_level;
        if (ESP_LCD_TOUCH_IO_I2C_GT911_ADDRESS_BACKUP == gt911_config->dev_addr) {
            gpio_level = 1;
        } else if (ESP_LCD_TOUCH_IO_I2C_GT911_ADDRESS == gt911_config->dev_addr) {
            gpio_level = 0;
        } else {
            gpio_level = 0;
            ESP_LOGE(TAG, "Addr (0x%X) is invalid", gt911_config->dev_addr);
        }
        ESP_RETURN_ON_ERROR(gpio_set_level(esp_lcd_touch_gt911->config.int_gpio_num, gpio_level), TAG, "GPIO set level error!");
        vTaskDelay(pdMS_TO_TICKS(1));

        ESP_RETURN_ON_ERROR(gpio_set_level(esp_lcd_touch_gt911->config.rst_gpio_num, !esp_lcd_touch_gt911->config.levels.reset), TAG, "GPIO set level error!");
        vTaskDelay(pdMS_TO_TICKS(10));

        vTaskDelay(pdMS_TO_TICKS(50));
    } else {
        ESP_LOGW(TAG, "Unable to initialize the I2C address");
        /* Reset controller */
        ret = touch_gt911_reset(esp_lcd_touch_gt911);
        ESP_GOTO_ON_ERROR(ret, err, TAG, "GT911 reset failed");
    }

    /* Prepare pin for touch interrupt */
    if (esp_lcd_touch_gt911->config.int_gpio_num != GPIO_NUM_NC) {
        const gpio_config_t int_gpio_config = {
            .mode = GPIO_MODE_INPUT,
            .intr_type = (esp_lcd_touch_gt911->config.levels.interrupt ? GPIO_INTR_POSEDGE : GPIO_INTR_NEGEDGE),
            .pin_bit_mask = BIT64(esp_lcd_touch_gt911->config.int_gpio_num)
        };
        ret = gpio_config(&int_gpio_config);
        ESP_GOTO_ON_ERROR(ret, err, TAG, "GPIO config failed");

        /* Register interrupt callback */
        if (esp_lcd_touch_gt911->config.interrupt_callback) {
            esp_lcd_touch_register_interrupt_callback(esp_lcd_touch_gt911, esp_lcd_touch_gt911->config.interrupt_callback);
        }
    }

    /* Read status and config info */
    ret = touch_gt911_read_cfg(esp_lcd_touch_gt911);
    ESP_GOTO_ON_ERROR(ret, err, TAG, "GT911 init failed");

err:
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error (0x%x)! Touch controller GT911 initialization failed!", ret);
        if (esp_lcd_touch_gt911) {
            esp_lcd_touch_gt911_del(esp_lcd_touch_gt911);
        }
    }

    *out_touch = esp_lcd_touch_gt911;

    return ret;
}

static esp_err_t esp_lcd_touch_gt911_enter_sleep(esp_lcd_touch_handle_t tp)
{
    esp_err_t err = touch_gt911_i2c_write(tp, ESP_LCD_TOUCH_GT911_ENTER_SLEEP, 0x05);
    ESP_RETURN_ON_ERROR(err, TAG, "Enter Sleep failed!");

    return ESP_OK;
}

static esp_err_t esp_lcd_touch_gt911_exit_sleep(esp_lcd_touch_handle_t tp)
{
    esp_err_t ret;
    esp_lcd_touch_handle_t esp_lcd_touch_gt911 = tp;

    if (esp_lcd_touch_gt911->config.int_gpio_num != GPIO_NUM_NC) {
        const gpio_config_t int_gpio_config_high = {
            .mode = GPIO_MODE_OUTPUT,
            .pin_bit_mask = BIT64(esp_lcd_touch_gt911->config.int_gpio_num)
        };
        ret = gpio_config(&int_gpio_config_high);
        ESP_RETURN_ON_ERROR(ret, TAG, "High GPIO config failed");
        gpio_set_level(esp_lcd_touch_gt911->config.int_gpio_num, 1);

        vTaskDelay(pdMS_TO_TICKS(5));

        const gpio_config_t int_gpio_config_float = {
            .mode = GPIO_MODE_OUTPUT_OD,
            .pin_bit_mask = BIT64(esp_lcd_touch_gt911->config.int_gpio_num)
        };
        ret = gpio_config(&int_gpio_config_float);
        ESP_RETURN_ON_ERROR(ret, TAG, "Float GPIO config failed");
    }

    return ESP_OK;
}

static esp_err_t esp_lcd_touch_gt911_read_data(esp_lcd_touch_handle_t tp)
{
    esp_err_t err;
    uint8_t buf[41];
    uint8_t touch_cnt = 0;
    uint8_t clear = 0;
    size_t i = 0;

    assert(tp != NULL);

    err = touch_gt911_i2c_read(tp, ESP_LCD_TOUCH_GT911_READ_XY_REG, buf, 1);
    ESP_RETURN_ON_ERROR(err, TAG, "I2C read error!");

    /* Any touch data? */
    if ((buf[0] & 0x80) == 0x00) {
        touch_gt911_i2c_write(tp, ESP_LCD_TOUCH_GT911_READ_XY_REG, clear);
#if (ESP_LCD_TOUCH_MAX_BUTTONS > 0)
    } else if ((buf[0] & 0x10) == 0x10) {
        /* Read all keys */
        uint8_t key_max = ((ESP_GT911_TOUCH_MAX_BUTTONS < ESP_LCD_TOUCH_MAX_BUTTONS) ? \
                           (ESP_GT911_TOUCH_MAX_BUTTONS) : (ESP_LCD_TOUCH_MAX_BUTTONS));
        err = touch_gt911_i2c_read(tp, ESP_LCD_TOUCH_GT911_READ_KEY_REG, &buf[0], key_max);
        ESP_RETURN_ON_ERROR(err, TAG, "I2C read error!");

        /* Clear all */
        touch_gt911_i2c_write(tp, ESP_LCD_TOUCH_GT911_READ_XY_REG, clear);
        ESP_RETURN_ON_ERROR(err, TAG, "I2C write error!");

        portENTER_CRITICAL(&tp->data.lock);

        /* Buttons count */
        tp->data.buttons = key_max;
        for (i = 0; i < key_max; i++) {
            tp->data.button[i].status = buf[0] ? 1 : 0;
        }

        portEXIT_CRITICAL(&tp->data.lock);
#endif
    } else if ((buf[0] & 0x80) == 0x80) {
#if (ESP_LCD_TOUCH_MAX_BUTTONS > 0)
        portENTER_CRITICAL(&tp->data.lock);
        for (i = 0; i < ESP_LCD_TOUCH_MAX_BUTTONS; i++) {
            tp->data.button[i].status = 0;
        }
        portEXIT_CRITICAL(&tp->data.lock);
#endif
        /* Count of touched points */
        touch_cnt = buf[0] & 0x0f;
        if (touch_cnt > 5 || touch_cnt == 0) {
            touch_gt911_i2c_write(tp, ESP_LCD_TOUCH_GT911_READ_XY_REG, clear);
            return ESP_OK;
        }

        /* Read all points */
        err = touch_gt911_i2c_read(tp, ESP_LCD_TOUCH_GT911_READ_XY_REG + 1, &buf[1], touch_cnt * 8);
        ESP_RETURN_ON_ERROR(err, TAG, "I2C read error!");

        /* Clear all */
        err = touch_gt911_i2c_write(tp, ESP_LCD_TOUCH_GT911_READ_XY_REG, clear);
        ESP_RETURN_ON_ERROR(err, TAG, "I2C read error!");

        portENTER_CRITICAL(&tp->data.lock);

        /* Number of touched points */
        touch_cnt = (touch_cnt > ESP_LCD_TOUCH_MAX_POINTS ? ESP_LCD_TOUCH_MAX_POINTS : touch_cnt);
        tp->data.points = touch_cnt;

        /* Fill all coordinates */
        for (i = 0; i < touch_cnt; i++) {
            tp->data.coords[i].x = ((uint16_t)buf[(i * 8) + 3] << 8) + buf[(i * 8) + 2];
            tp->data.coords[i].y = (((uint16_t)buf[(i * 8) + 5] << 8) + buf[(i * 8) + 4]);
            tp->data.coords[i].strength = (((uint16_t)buf[(i * 8) + 7] << 8) + buf[(i * 8) + 6]);
        }

        portEXIT_CRITICAL(&tp->data.lock);
    }

    return ESP_OK;
}

static bool esp_lcd_touch_gt911_get_xy(esp_lcd_touch_handle_t tp, uint16_t *x, uint16_t *y, uint16_t *strength, uint8_t *point_num, uint8_t max_point_num)
{
    assert(tp != NULL);
    assert(x != NULL);
    assert(y != NULL);
    assert(point_num != NULL);
    assert(max_point_num > 0);

    portENTER_CRITICAL(&tp->data.lock);

    /* Count of points */
    *point_num = (tp->data.points > max_point_num ? max_point_num : tp->data.points);

    for (size_t i = 0; i < *point_num; i++) {
        x[i] = tp->data.coords[i].x;
        y[i] = tp->data.coords[i].y;

        if (strength) {
            strength[i] = tp->data.coords[i].strength;
        }
    }

    /* Invalidate */
    tp->data.points = 0;

    portEXIT_CRITICAL(&tp->data.lock);

    return (*point_num > 0);
}

#if (ESP_LCD_TOUCH_MAX_BUTTONS > 0)
static esp_err_t esp_lcd_touch_gt911_get_button_state(esp_lcd_touch_handle_t tp, uint8_t n, uint8_t *state)
{
    esp_err_t err = ESP_OK;
    assert(tp != NULL);
    assert(state != NULL);

    *state = 0;

    portENTER_CRITICAL(&tp->data.lock);

    if (n > tp->data.buttons) {
        err = ESP_ERR_INVALID_ARG;
    } else {
        *state = tp->data.button[n].status;
    }

    portEXIT_CRITICAL(&tp->data.lock);

    return err;
}
#endif

static esp_err_t esp_lcd_touch_gt911_del(esp_lcd_touch_handle_t tp)
{
    assert(tp != NULL);

    /* Reset GPIO pin settings */
    if (tp->config.int_gpio_num != GPIO_NUM_NC) {
        gpio_reset_pin(tp->config.int_gpio_num);
        if (tp->config.interrupt_callback) {
            gpio_isr_handler_remove(tp->config.int_gpio_num);
        }
    }

    /* Reset GPIO pin settings */
    if (tp->config.rst_gpio_num != GPIO_NUM_NC) {
        gpio_reset_pin(tp->config.rst_gpio_num);
    }

    free(tp);

    return ESP_OK;
}

// Function to initialize the GT911 touch controller
esp_lcd_touch_handle_t touch_gt911_init()
{
    esp_lcd_panel_io_handle_t tp_io_handle = NULL;  // Declare a handle for touch panel I/O
    // Configure the I2C communication settings for the GT911 touch controller
    const esp_lcd_panel_io_i2c_config_t tp_io_config = ESP_LCD_TOUCH_IO_I2C_GT911_CONFIG();

    // Reset the touch screen before usage
    DEV_I2C_Port port = DEV_I2C_Init();  // Initialize I2C port
    IO_EXTENSION_Init();  // Initialize the IO EXTENSION GPIO chip for backlight control
    DEV_GPIO_Mode(EXAMPLE_PIN_NUM_TOUCH_INT, GPIO_MODE_INPUT_OUTPUT);  // Set GPIO pin mode for interrupt
    IO_EXTENSION_Output(IO_EXTENSION_IO_1, 0);  // Set GPIO for backlight control to low (off)
    
    vTaskDelay(pdMS_TO_TICKS(100));  // Wait for 100ms
    DEV_Digital_Write(EXAMPLE_PIN_NUM_TOUCH_INT, 0);  // Set interrupt pin low (disable interrupt)
    
    vTaskDelay(pdMS_TO_TICKS(100));  // Wait for another 100ms
    IO_EXTENSION_Output(IO_EXTENSION_IO_1, 1);  // Set GPIO for backlight control to high (on)
    
    vTaskDelay(pdMS_TO_TICKS(200));  // Wait for 200ms to ensure the touch controller is ready

    ESP_LOGI(TAG, "Initialize I2C panel IO");  // Log I2C panel I/O initialization
    // Create a new I2C panel I/O handle for the touch controller
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c(port.bus, &tp_io_config, &tp_io_handle));

    ESP_LOGI(TAG, "Initialize touch controller GT911");  // Log touch controller initialization
    // Configure the touch controller with necessary settings (coordinates, GPIO pins, etc.)
    const esp_lcd_touch_config_t tp_cfg = {
        .x_max = EXAMPLE_LCD_H_RES,  // Set the maximum X coordinate based on screen resolution
        .y_max = EXAMPLE_LCD_V_RES,  // Set the maximum Y coordinate based on screen resolution
        .rst_gpio_num = EXAMPLE_PIN_NUM_TOUCH_RST,  // GPIO number for reset
        .int_gpio_num = EXAMPLE_PIN_NUM_TOUCH_INT,  // GPIO number for interrupt
        .levels = {
            .reset = 0,  // Low level for reset
            .interrupt = 0,  // Low level for interrupt
        },
        .flags = {
            .swap_xy = 0,  // No swap of X and Y coordinates
            .mirror_x = 0,  // No mirroring of X axis
            .mirror_y = 0,  // No mirroring of Y axis
        },
    };

    // Create a new touch controller instance using the configured I2C and settings
    ESP_ERROR_CHECK(esp_lcd_touch_new_i2c_gt911(tp_io_handle, &tp_cfg, &tp_handle));

    return tp_handle;  // Return the touch controller handle
}

// Function to read touch points from the GT911 touch controller
touch_gt911_point_t touch_gt911_read_point(uint8_t max_touch_cnt)
{
    touch_gt911_point_t data;  // Declare a structure to hold touch point data

    /* Read touch data from the touch controller */
    esp_lcd_touch_read_data(tp_handle);  // Read raw data from the touch controller

    /* Get the touch coordinates and count of touch points */
    esp_lcd_touch_get_coordinates(tp_handle, data.x, data.y, NULL, &data.cnt, max_touch_cnt);

    return data;  // Return the touch point data
}


/*******************************************************************************
* Private API function
*******************************************************************************/

/* Reset controller */
static esp_err_t touch_gt911_reset(esp_lcd_touch_handle_t tp)
{
    assert(tp != NULL);

    if (tp->config.rst_gpio_num != GPIO_NUM_NC) {
        ESP_RETURN_ON_ERROR(gpio_set_level(tp->config.rst_gpio_num, tp->config.levels.reset), TAG, "GPIO set level error!");
        vTaskDelay(pdMS_TO_TICKS(10));
        ESP_RETURN_ON_ERROR(gpio_set_level(tp->config.rst_gpio_num, !tp->config.levels.reset), TAG, "GPIO set level error!");
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    return ESP_OK;
}

static esp_err_t touch_gt911_read_cfg(esp_lcd_touch_handle_t tp)
{
    uint8_t buf[4];

    assert(tp != NULL);

    ESP_RETURN_ON_ERROR(touch_gt911_i2c_read(tp, ESP_LCD_TOUCH_GT911_PRODUCT_ID_REG, (uint8_t *)&buf[0], 3), TAG, "GT911 read error!");
    ESP_RETURN_ON_ERROR(touch_gt911_i2c_read(tp, ESP_LCD_TOUCH_GT911_CONFIG_REG, (uint8_t *)&buf[3], 1), TAG, "GT911 read error!");

    ESP_LOGI(TAG, "TouchPad_ID:0x%02x,0x%02x,0x%02x", buf[0], buf[1], buf[2]);
    ESP_LOGI(TAG, "TouchPad_Config_Version:%d", buf[3]);

    return ESP_OK;
}

static esp_err_t touch_gt911_i2c_read(esp_lcd_touch_handle_t tp, uint16_t reg, uint8_t *data, uint8_t len)
{
    assert(tp != NULL);
    assert(data != NULL);

    /* Read data */
    return esp_lcd_panel_io_rx_param(tp->io, reg, data, len);
}

static esp_err_t touch_gt911_i2c_write(esp_lcd_touch_handle_t tp, uint16_t reg, uint8_t data)
{
    assert(tp != NULL);

    // *INDENT-OFF*
    /* Write data */
    return esp_lcd_panel_io_tx_param(tp->io, reg, (uint8_t[]){data}, 1);
    // *INDENT-ON*
}
