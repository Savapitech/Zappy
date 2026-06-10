#pragma once

#include <memory>
#include <string>

#include "audio.hpp"

namespace Zappy {
    class audioManager {
        private:
            Audio _music;
        public:
            audioManager(const std::string &filepath){
                _music.playMusic(filepath);
            }
            void stopMusic() {_music.stopMusic();}
    };
}
