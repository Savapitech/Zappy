#pragma once

#include "Utils/miniaudio.h"
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