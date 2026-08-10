idf_component_register(SRCS "speaker_microphone.c" "codec_dev.c" 
                        INCLUDE_DIRS "."
                        REQUIRES esp_driver_i2s gpio i2c io_extension esp_codec_dev esp-audio-player esp-file-iterator
                    )
