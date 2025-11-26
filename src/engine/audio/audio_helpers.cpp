#include "pch.hpp"

#include "audio_helpers.hpp"

#include "audio_components.hpp"
#include "engine/audio/audio_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "modules/events/events_core/events_components.hpp"

namespace game2d {

Sound
get_sound(const SINGLE_AudioComponent& audio, const std::string& tag)
{
  const auto& res = audio.sounds;
  auto it = std::find_if(res.begin(), res.end(), [&tag](const auto& a) { return a.tag == tag; });

  if (it == res.end()) {
    SDL_Log("Unable to find sound: %s", tag.c_str());
    throw std::runtime_error("Unable to find sound.");
  }

  return (*it);
};

bool
refresh_audio(entt::registry& r, std::string device_name)
{
  {
    SDL_Log("Closing old audio device.");
    Mix_CloseAudio();
    auto view = r.view<AudioSource>();
    r.destroy(view.begin(), view.end());
  }

  auto& audio = get_first_component<SINGLE_AudioComponent>(r);

  // Initialize sdl_mixer with standard format and sample rate
  SDL_AudioSpec spec;
  spec.freq = MIX_DEFAULT_FREQUENCY;
  spec.format = MIX_DEFAULT_FORMAT;
  spec.channels = MIX_DEFAULT_CHANNELS;
  int chunk_size = 2048;

  // Try to open the most reasonable device
  // int device_index = 0;
  // const char* device_name = SDL_GetAudioDeviceName(device_index, 0);
  // if (device_name == nullptr) {
  //   SDL_Log("%s", std::format("No Default Audio Device enabled. Not loading sounds.").c_str());
  //   audio.loaded = true;
  //   return; // no available devices
  // }
  // SDL_Log("%s", std::format("Using audiodevice: {}", device_name).c_str());
  int result = -1;
  if (device_name == "")
    result = Mix_OpenAudio(spec.freq, spec.format, spec.channels, chunk_size);
  else
    result = Mix_OpenAudioDevice(spec.freq, spec.format, spec.channels, chunk_size, device_name.c_str(), 0);

  // Check if that worked.
  if (result == -1) {
    SDL_Log("%s", std::format("No Default Audio Device enabled. Not loading sounds.").c_str());
    return false;
  }

  SDL_Log("SDL audio driver: %s\n", SDL_GetCurrentAudioDriver());

  int n = SDL_GetNumAudioDevices(0);
  SDL_Log("Playback devices (%d):\n", n);
  for (int i = 0; i < n; ++i)
    SDL_Log("  [%d] %s\n", i, SDL_GetAudioDeviceName(i, 0));

  // request some channels
  const int request_channels = 64;
  Mix_AllocateChannels(request_channels);
  audio.max_audio_sources = Mix_AllocateChannels(-1); // -1 means query the number of channels
  SDL_Log("%s", std::format("Audio sources to create: {}", audio.max_audio_sources).c_str());
  for (int i = 0; i < audio.max_audio_sources; i++) {
    create_persistent<AudioSource>(r, AudioSource(i));
    // set volume to user pref
    Mix_Volume(i, static_cast<int>(MIX_MAX_VOLUME * audio.volume_master));
  }

  return true;
}

} // namespace game2d