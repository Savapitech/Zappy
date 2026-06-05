#pragma once

#define MINIAUDIO_IMPLEMENTATION
#include "Utils/miniaudio.h"
#include "Logger.hpp"

namespace Zappy
{
    class Audio {
        private:
            ma_engine _engine;
        public:
            Audio();
            ~Audio();
            void playMusic(const std::string &filepath);
    };
};