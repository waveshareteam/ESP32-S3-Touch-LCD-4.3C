/**
 * @file lv_demo_music.h
 *
 */

#ifndef LV_DEMO_MUSIC_H
#define LV_DEMO_MUSIC_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#include "lvgl.h"
#include "codec_dev.h"       // Header for audio codec device interface

/*********************
 *      DEFINES
 *********************/

#if LV_DEMO_MUSIC_LARGE
#  define LV_DEMO_MUSIC_HANDLE_SIZE  40
#else
#  define LV_DEMO_MUSIC_HANDLE_SIZE  20
#endif

/**********************
 *      TYPEDEFS
 **********************/
extern char *Mp3Path[];       // Full path to MP3 files
extern char *Mp3Filename[];   // MP3 file names only
extern uint8_t mp3_num;          // Total number of MP3 files found
extern uint8_t play_state;          // 播放器状态

/**********************
 * GLOBAL PROTOTYPES
 **********************/

void user_lv_demo_music(void);
const char * lv_demo_music_get_title(uint32_t track_id);
const char * lv_demo_music_get_artist(uint32_t track_id);
const char * lv_demo_music_get_genre(uint32_t track_id);
uint32_t lv_demo_music_get_track_length(uint32_t track_id);

void list_files(const char *base_path);
void speaker_callback(audio_player_cb_ctx_t *ctx);

/**********************
 *      MACROS
 **********************/


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LV_DEMO_MUSIC_H*/
