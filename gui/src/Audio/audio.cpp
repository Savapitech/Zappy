#include "audio.hpp"

namespace Zappy 
{
    Audio::Audio() {
        if (ma_engine_init(NULL, &_engine) != MA_SUCCESS) {
            LOG_FATAL("Failed to init miniAudio engine");
        }
    }
    Audio::~Audio() {
        ma_engine_uninit(&_engine);
    }
    void Audio::playMusic(const std::string &filepath)
    {
        ma_engine_play_sound(&_engine, filepath.c_str(), NULL);
    }
};
