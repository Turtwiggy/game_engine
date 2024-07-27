#include "system.hpp"

#include "app/io.hpp"
#include "audio/helpers.hpp"
#include "components.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>
#include <SDL2/SDL_mixer.h>

#include "entt/helpers.hpp"
#include <entt/entt.hpp>
#include <fmt/core.h>
#include <imgui.h>

#include <chrono>
#include <map>
#include <string>
#include <vector>

namespace game2d {

// todo: fix audio source being created every request -- ring buffer?
// todo: a request is only processed when the audio source has finished playing the sound

void
init_audio_system(entt::registry& r)
{
  auto& audio = get_first_component<SINGLETON_AudioComponent>(r);

  // Initialize sdl_mixer with standard format and sample rate
  SDL_AudioSpec spec;
  spec.freq = MIX_DEFAULT_FREQUENCY;
  spec.format = MIX_DEFAULT_FORMAT;
  spec.channels = MIX_DEFAULT_CHANNELS;
  int chunk_size = 2048;

  // Try to open the most reasonable device
  int device_index = 0;
  const char* device_name = SDL_GetAudioDeviceName(device_index, 0);
  if (device_name == nullptr) {

    fmt::println("No Default Audio Device enabled. Not loading sounds.");
    audio.loaded = true;
    return; // no available devices
  }
  fmt::println("Using audiodevice: {}", device_name);
  audio.captured_device_id = Mix_OpenAudioDevice(spec.freq, spec.format, spec.channels, chunk_size, device_name, 0);

  // Check if that worked.
  if (audio.captured_device_id == -1) {
    fmt::println("No Default Audio Device enabled. Not loading sounds.");
    return;
  }

  // request some channels
  const int request_channels = 16;
  Mix_AllocateChannels(request_channels);
  audio.max_audio_sources = Mix_AllocateChannels(-1); // -1 means query the number of channels
  fmt::println("Audio sources to create: {}", audio.max_audio_sources);
  for (int i = 0; i < audio.max_audio_sources; i++) {
    create_empty<AudioSource>(r, AudioSource(i));

    // set volume to user pref
    Mix_Volume(i, audio.volume_internal);
  }

  fmt::println("Loading audio...");
  for (auto& file : audio.sounds) {
    auto* sound = Mix_LoadWAV(file.path.c_str());
    if (!sound) {
      fmt::println("Failed to load sound: {}, {}", file.path, Mix_GetError());
      continue;
    }
    file.buffer = sound;
  }

  audio.loaded = true;
}

void
update_audio_system(entt::registry& r)
{
  const auto audio_e = get_first<SINGLETON_AudioComponent>(r);
  if (audio_e == entt::null)
    return;
  const auto& audio = get_first_component<SINGLETON_AudioComponent>(r);
  if (!audio.loaded)
    return;
  if (audio.sounds.size() == 0)
    return; // no sounds loaded

  // If muted, destroy all requests
  if (audio.mute_all) {
    const auto& view = r.view<AudioRequestPlayEvent>();
    r.destroy(view.begin(), view.end());
  }

  // check if request is a sfx effect
  for (const auto& [e, req] : r.view<AudioRequestPlayEvent>().each()) {
    const Sound& s = get_sound(audio, req.tag);
    if (audio.mute_sfx && s.type == SoundType::SFX)
      r.destroy(e);
  }

  // a vector of free audio sources, populated every frame
  std::vector<AudioSource> free_audio_sources;

  // state: playing -> free
  for (const auto& [entity, source] : r.view<AudioSource>().each()) {

    source.state = AudioSourceState::FREE;
    if (Mix_Playing(source.channel))
      source.state = AudioSourceState::PLAYING;

    if (source.state == AudioSourceState::FREE)
      free_audio_sources.push_back(source);
  }

  // compact duplicate audio requests
  std::map<std::string, std::vector<entt::entity>> compacted_requests;
  for (const auto& [entity, request] : r.view<AudioRequestPlayEvent>().each())
    compacted_requests[request.tag].push_back(entity);

  // state: process request -> playing
  for (const auto& [tag, entities] : compacted_requests) {

    // assume audio request with same tag are the same
    const auto& entity = entities[0];
    const auto& request = r.get<AudioRequestPlayEvent>(entity);

    if (free_audio_sources.size() == 0) {
      fmt::println("No free audio sources! Missed request for: {}", tag);
      continue;
    }

    AudioSource& audio_source = free_audio_sources.front();
    free_audio_sources.erase(free_audio_sources.begin());
    audio_source.state = AudioSourceState::PLAYING;

    const Sound s = get_sound(audio, request.tag);
    const int channel = Mix_PlayChannel(audio_source.channel, s.buffer, 0);
    if (channel != audio_source.channel) {
      fmt::println("Warning: sound playing on incorrect channel");
    }

    // process request
    r.destroy(entities.begin(), entities.end());
  }
};

} // namespace game2d