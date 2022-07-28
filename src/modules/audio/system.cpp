// header
#include "system.hpp"

// my lib
#include "modules/events/components.hpp"
#include "modules/events/helpers/keyboard.hpp"
#include "modules/events/helpers/mouse.hpp"
#include "resources/audio.hpp"

// other lib
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_log.h>
#include <SDL2/SDL_mixer.h>

// std lib
#include <iostream>

namespace game2d {

// https://wiki.libsdl.org/SDL_mixer/CategoryAPI
// https://wiki.libsdl.org/SDL_AudioSpec

void
init_audio_system(entt::registry& registry)
{
  SINGLETON_AudioComponent audio;

  if (SDL_WasInit(SDL_INIT_EVERYTHING) & SDL_INIT_AUDIO) {
    SDL_Log("SDL audio is init...");
  }

  int i, count = SDL_GetNumAudioDevices(0);
  for (i = 0; i < count; ++i) {
    SDL_Log("Audio device %d: %s", i, SDL_GetAudioDeviceName(i, 0));
  }

  int freq = 48000;
  Uint16 format = AUDIO_F32;
  int channels = 2;
  int samples = 4096;

  audio.dev = Mix_OpenAudioDevice(freq, format, channels, samples, NULL, SDL_AUDIO_ALLOW_ANY_CHANGE);
  if (audio.dev == -1) {
    std::cerr << "Failed to open audio: " << SDL_GetError() << std::endl;
    registry.ctx().emplace<SINGLETON_AudioComponent>(audio);
    return;
  }

  //
  // Load Sounds and Music
  //

  audio.sound = Mix_LoadWAV(audio.sound_path.c_str());
  if (audio.sound == NULL) {
    std::cerr << "failed to load sound: " << SDL_GetError() << std::endl;
    exit(0);
  }

  registry.ctx().emplace<SINGLETON_AudioComponent>(audio);
};

void
update_audio_system(entt::registry& registry)
{
  const auto& events = registry.ctx().at<SINGLETON_InputComponent>();
  auto& audio = registry.ctx().at<SINGLETON_AudioComponent>();

  // TODO: fix audio

  // auto audio_added = std::find_if(
  //   events.sdl_events.begin(), events.sdl_events.end(), [](const auto& e) { return e.type == SDL_AUDIODEVICEADDED;
  //   });
  // auto audio_removed = std::find_if(
  //   events.sdl_events.begin(), events.sdl_events.end(), [](const auto& e) { return e.type == SDL_AUDIODEVICEREMOVED;
  //   });

  // if (audio_added != events.sdl_events.end()) {
  //   const int count = SDL_GetNumAudioDevices(0);
  //   std::cout << "(audio device added) Audio Devices: " << count << std::endl;
  // }

  // // Note: this event is only fired if the device was in use by sdl.
  // if (audio_removed != events.sdl_events.end()) {
  //   const int count = SDL_GetNumAudioDevices(0);
  //   std::cout << "(audio device removed). Audio devices: " << count << std::endl;
  // }

  // if (audio.dev == -1) // Failed to open - TODO: implement retry?
  //   return;

  // EXAMPLE: playing audio
  // if (get_mouse_rmb_press()) {
  //   int success = Mix_PlayChannel(0, audio.sound, 0); // play once and stop
  //   // returns the channel used to play, or -1 if could not be played
  //   std::cout << "playing sound on channel: " << success;
  // }
};

} // namespace game2d