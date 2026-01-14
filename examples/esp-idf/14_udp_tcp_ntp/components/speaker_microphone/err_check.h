#pragma once

#include "esp_check.h"
#include "sdkconfig.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ERROR_CHECK 1
/* Assert on error, if selected in menuconfig. Otherwise return error code. */
#if ERROR_CHECK
#define USER_ERROR_CHECK_RETURN_ERR(x)    ESP_ERROR_CHECK(x)
#define USER_ERROR_CHECK_RETURN_NULL(x)   ESP_ERROR_CHECK(x)
#define USER_ERROR_CHECK(x, ret)          ESP_ERROR_CHECK(x)
#define USER_NULL_CHECK(x, ret)           assert(x)
#define USER_NULL_CHECK_GOTO(x, goto_tag) assert(x)
#else
#define USER_ERROR_CHECK_RETURN_ERR(x) do { \
        esp_err_t err_rc_ = (x);            \
        if (unlikely(err_rc_ != ESP_OK)) {  \
            return err_rc_;                 \
        }                                   \
    } while(0)

#define USER_ERROR_CHECK_RETURN_NULL(x)  do { \
        if (unlikely((x) != ESP_OK)) {      \
            return NULL;                    \
        }                                   \
    } while(0)

#define USER_NULL_CHECK(x, ret) do { \
        if ((x) == NULL) {          \
            return ret;             \
        }                           \
    } while(0)

#define USER_ERROR_CHECK(x, ret)      do { \
        if (unlikely((x) != ESP_OK)) {    \
            return ret;                   \
        }                                 \
    } while(0)

#define USER_NULL_CHECK_GOTO(x, goto_tag) do { \
        if ((x) == NULL) {      \
            goto goto_tag;      \
        }                       \
    } while(0)
#endif

#ifdef __cplusplus
}
#endif