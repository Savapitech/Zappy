#define MINIAUDIO_IMPLEMENTATION
#include "audio.hpp"

namespace Zappy {
Audio::Audio(const std::string &filepath)
    : _isMusicLoaded(false), _filepath(filepath) {
  if (ma_engine_init(NULL, &_engine) != MA_SUCCESS) {
    LOG_FATAL("Failed to init miniAudio engine");
  }
}
Audio::~Audio() { ma_engine_uninit(&_engine); }
void Audio::playMusic() {
  stopMusic();
  ma_result result = ma_sound_init_from_file(&_engine, _filepath.c_str(), 0,
                                             NULL, NULL, &_music);
  if (result == MA_SUCCESS) {
    _isMusicLoaded = true;
    ma_sound_set_spatialization_enabled(&_music, MA_FALSE);
    ma_sound_set_looping(&_music, MA_TRUE);
    ma_sound_start(&_music);
  } else
    LOG_FATAL("Failed to load the music: " + _filepath);
}
void Audio::playSound() {
  ma_engine_play_sound(&_engine, _filepath.c_str(), NULL);
}
void Audio::stopMusic() {
  if (_isMusicLoaded) {
    ma_sound_uninit(&_music);
    _isMusicLoaded = false;
  }
}
}; // namespace Zappy
