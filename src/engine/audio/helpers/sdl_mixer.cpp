#include "pch.hpp"

#include "engine/audio/helpers/sdl_mixer.hpp"

#include "engine/audio/audio_components.hpp"

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
  // destroy all audio requests
  const auto audio_req_view = r.view<AudioRequestPlayEvent>();
  r.destroy(audio_req_view.begin(), audio_req_view.end());

  const auto& view = r.view<AudioSource>();
  for (const auto& [e, source] : view.each())
    Mix_HaltChannel(source.channel);
};

void
process_audio_added(entt::registry& r)
{
  SDL_Log("%s", "TODO: implement audio device added");

  SDL_Log("%s", "~~~~ Devices ~~~~");
  for (const auto& device : list_devices())
    SDL_Log("%s", std::format("{}", device).c_str());

  SDL_Log("%s", "~~~~~~~~~~~~~~~~~");
};

void
process_audio_removed(entt::registry& r) {
  // SDL_Log("%s", "TODO: implement audio device removed");

  // SDL_Log("%s", "~~~~ Devices ~~~~");
  // for (const auto& device : list_devices()) {
  //   // const std::string msg = std::format("%i", device);
  //   // SDL_Log("%s", msg.c_str());
  // }
  // SDL_Log("%s", "~~~~~~~~~~~~~~~~~");
};

} // namespace audio

} // namespace sdl_mixer

} // namespace game2d