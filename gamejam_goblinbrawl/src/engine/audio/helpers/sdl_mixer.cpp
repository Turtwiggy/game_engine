#include "sdl_mixer.hpp"

#include "engine/audio/audio_components.hpp"

#include <SDL2/SDL_audio.h>
#include <SDL2/SDL_log.h>
#include <SDL2/SDL_mixer.h>
#include <format>

namespace game2d {

namespace audio {

namespace sdl_mixer {

std::vector<std::string>
list_devices()
{
  std::vector<std::string> devices;

  // 0 to list playback devices
  // non-zero to list recording devices
  const int count = SDL_GetNumAudioDevices(0);
  for (int i = 0; i < count; ++i)
    devices.push_back({ SDL_GetAudioDeviceName(i, 0) });

  return devices;
}

void
stop_all_audio(entt::registry& r)
{
  const auto& view = r.view<AudioSource>();
  for (const auto& [e, source] : view.each()) {
    Mix_FadeOutChannel(source.channel, 1000);
  }
};

void
process_audio_added(entt::registry& r)
{
  SDL_Log("%s", std::format("TODO: implement audio device added").c_str());

  SDL_Log("%s", std::format("~~~~ Devices ~~~~").c_str());
  for (const auto& device : list_devices())
    SDL_Log("%s", std::format("{}", device).c_str());

  SDL_Log("%s", std::format("~~~~~~~~~~~~~~~~~").c_str());
};

void
process_audio_removed(entt::registry& r) {
  // SDL_Log("%s", std::format("TODO: implement audio device removed").c_str());

  // SDL_Log("%s", std::format("~~~~ Devices ~~~~").c_str());
  // for (const auto& device : list_devices()) {
  //   // const std::string msg = std::format("%i", device);
  //   // SDL_Log("%s", msg.c_str());
  // }
  // SDL_Log("%s", std::format("~~~~~~~~~~~~~~~~~").c_str());
};

} // namespace audio

} // namespace sdl_mixer

} // namespace game2d