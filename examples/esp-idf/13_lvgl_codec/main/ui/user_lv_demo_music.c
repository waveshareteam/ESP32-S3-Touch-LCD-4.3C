/**
 * @file lv_demo_music.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "user_lv_demo_music.h"

#include "user_lv_demo_music_main.h"
#include "user_lv_demo_music_list.h"

#include <stdio.h>  
#include <string.h>          // 
#include <dirent.h>          // Header for directory operations

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
char *Mp3Path[256];       // Full path to MP3 files
char *Mp3Filename[256];   // MP3 file names only
uint8_t mp3_num = 0;      // Total number of MP3 files found
uint8_t play_state = 0;   // Player status

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/
static lv_obj_t * ctrl;
static lv_obj_t * list;

/*
static const char * title_list[] = {
    "Waiting for true love",
    "Need a Better Future",
    "Vibrations",
    "Why now?",
    "Never Look Back",
    "It happened Yesterday",
    "Feeling so High",
    "Go Deeper",
    "Find You There",
    "Until the End",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
};
*/

static const char * artist_list[] = {
    "For Elise",
    "BGM 1",
    "BGM 2",
    "Something",
    "Waka Waka",
    "Robotics",
    "Robotics",
    "Unknown artist",
    "Unknown artist",
    "Unknown artist",
    "Unknown artist",
    "Unknown artist",
    "Unknown artist",
    "Unknown artist",
    "Unknown artist",
};

static const char * genre_list[] = {
    "For Elise - 2025",
    "BGM 1 - 2025",
    "BGM 2 - 2025",
    "Something - 2015",
    "Waka Waka - 2015",
    "Metal - 2015",
    "Metal - 2015",
    "Metal - 2015",
    "Metal - 2015",
    "Metal - 2015",
    "Metal - 2015",
    "Metal - 2015",
    "Metal - 2015",
    "Metal - 2015",
};

static const uint32_t time_list[] = {
    1 * 60 + 0,
    0 * 60 + 35,
    1 * 60 + 0,
    0 * 60 + 40,
    1 * 60 + 0,
    3 * 60 + 33,
    1 * 60 + 56,
    3 * 60 + 31,
    2 * 60 + 20,
    2 * 60 + 19,
    2 * 60 + 20,
    2 * 60 + 19,
    2 * 60 + 20,
    2 * 60 + 19,
};

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/


void user_lv_demo_music(void)
{
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0x343247), 0);

    list = lv_demo_music_list_create(lv_screen_active());
    ctrl = lv_demo_music_main_create(lv_screen_active()); 
}

const char * lv_demo_music_get_title(uint32_t track_id)
{

    if (track_id > mp3_num)
    {
        track_id = 0;
        return NULL;
    }
    
    return Mp3Filename[track_id];
}

const char * lv_demo_music_get_artist(uint32_t track_id)
{
    if(track_id >= sizeof(artist_list) / sizeof(artist_list[0])) return NULL;
    return artist_list[track_id];
}

const char * lv_demo_music_get_genre(uint32_t track_id)
{
    if(track_id >= sizeof(genre_list) / sizeof(genre_list[0])) return NULL;
    return genre_list[track_id];
}

uint32_t lv_demo_music_get_track_length(uint32_t track_id)
{
    if(track_id >= sizeof(time_list) / sizeof(time_list[0])) return 0;
    return time_list[track_id];
}

/**
 * @brief List all MP3 files in the specified directory.
 * 
 * @param base_path The path to search for MP3 files.
 */
void list_files(const char *base_path) {
    int i = 0;
    DIR *dir = opendir(base_path);
    if (dir) {
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            const char *file_name = entry->d_name;
            size_t len = strlen(file_name);
            if (len > 4 && strcasecmp(&file_name[len - 4], ".mp3") == 0) {
                size_t length = strlen(base_path) + len + 2; // +1 '/', +1 '\0'

                Mp3Filename[i] = malloc(length);
                Mp3Path[i] = malloc(length);
                if (!Mp3Filename[i] || !Mp3Path[i]) {
                    // Handling memory allocation failure
                    break;
                }
                snprintf(Mp3Path[i], length, "%s/%s", base_path, file_name);
                // Copy and remove the suffix
                char name_copy[256];
                strncpy(name_copy, file_name, sizeof(name_copy) - 1);
                name_copy[len - 4] = '\0';
                snprintf(Mp3Filename[i], length, "%s", name_copy);
                i++;
            }
        }
        mp3_num = i;
        closedir(dir);
    }
}