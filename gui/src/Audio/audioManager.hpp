#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include "audio.hpp"

namespace Zappy {
    class audioManager {
        private:
            std::unordered_map<std::string, std::unique_ptr<Audio>> _audios;
        public:
            void upload(const std::string &filepath) {
                auto it = _audios.find(filepath);

                if (it == _audios.end()) {
                    _audios[filepath] = std::make_unique<Audio>(filepath);
                }
            }
            void playMusicAt(const std::string &filepath)
            {
                auto it = _audios.find(filepath);
                if (it == _audios.end())
                    return;
                it->second->playMusic();
            }
            void playSoundAt(const std::string &filepath)
            {
                auto it = _audios.find(filepath);
                if (it == _audios.end())
                    return;
                it->second->playSound();
            }
            void stopMusicAt(const std::string &filepath)
            {
                auto it = _audios.find(filepath);
                if (it == _audios.end())
                    return;
                it->second->stopMusic();
            }
    };
}
