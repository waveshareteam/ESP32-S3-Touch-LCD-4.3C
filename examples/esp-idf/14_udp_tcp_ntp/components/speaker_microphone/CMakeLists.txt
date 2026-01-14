idf_component_register(SRCS "speaker_microphone.c" "codec_dev.c" 
                        INCLUDE_DIRS "."
                        REQUIRES driver gpio i2c io_extension esp_codec_dev esp-audio-player esp-file-iterator 
                    )
