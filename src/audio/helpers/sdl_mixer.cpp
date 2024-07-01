#include "sdl_mixer.hpp"

#include "audio/components.hpp"
#include <SDL_audio.h>
#include <SDL_mixer.h>
#include <fmt/core.h>

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
  fmt::println("TODO: implement audio device added");

  fmt::println("~~~~ Devices ~~~~");
  for (const auto& device : list_devices())
    fmt::println("{}", device);
  fmt::println("~~~~~~~~~~~~~~~~~");
};

void
process_audio_removed(entt::registry& r)
{
  fmt::println("TODO: implement audio device removed");

  fmt::println("~~~~ Devices ~~~~");
  for (const auto& device : list_devices())
    fmt::println("{}", device);
  fmt::println("~~~~~~~~~~~~~~~~~");
};

} // namespace audio

} // namespace sdl_mixer

} // namespace game2d