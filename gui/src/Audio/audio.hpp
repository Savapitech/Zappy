#pragma once



#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

#include "Utils/miniaudio.h"

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

#include "Logger.hpp"

namespace Zappy
{
    class Audio {
        private:
            ma_engine _engine;
            ma_sound _music;
            bool _isMusicLoaded;

        public:
            Audio();
            ~Audio();
            void playMusic(const std::string &filepath);
            void playSound(const std::string &filepath);
            void stopMusic();
    };
};