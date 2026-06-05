#define MINIAUDIO_IMPLEMENTATION
#include "audio.hpp"

namespace Zappy 
{
    Audio::Audio() : _isMusicLoaded(false) 
    {
        ma_engine_config engineConfig = ma_engine_config_init();
        engineConfig.periodSizeInMilliseconds = 50;
        if (ma_engine_init(NULL, &_engine) != MA_SUCCESS) {
            LOG_FATAL("Failed to init miniAudio engine");
        }
    }
    Audio::~Audio() {
        ma_engine_uninit(&_engine);
    }
    void Audio::playMusic(const std::string &filepath)
    {
        stopMusic();
        ma_result result = ma_sound_init_from_file(&_engine, filepath.c_str(), 0, NULL, NULL, &_music);
        if (result == MA_SUCCESS) {
            _isMusicLoaded = true;
            ma_sound_set_spatialization_enabled(&_music, MA_FALSE);
            ma_sound_set_looping(&_music, MA_TRUE);
            ma_sound_start(&_music);
        } else 
            LOG_FATAL("Failed to load the music: " + filepath);
    }
    void Audio::playSound(const std::string &filepath)
    {
        ma_engine_play_sound(&_engine, filepath.c_str(), NULL);
    }
    void Audio::stopMusic()
    {
        if (_isMusicLoaded) {
            ma_sound_uninit(&_music);
            _isMusicLoaded = false;
        }
    }
};
