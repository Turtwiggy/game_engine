#include "audio_system.hpp"

#include "audio_components.hpp"
#include "engine/audio/audio_helpers.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>
#include <SDL2/SDL_mixer.h>

#include "engine/entt/helpers.hpp"
#include <SDL2/SDL_log.h>
#include <entt/entt.hpp>
#include <format>
#include <imgui.h>

#include <map>
#include <string>
#include <vector>

namespace game2d {

void
init_audio_system(entt::registry& r)
{
  auto& audio = get_first_component<SINGLE_AudioComponent>(r);

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

    SDL_Log("%s", std::format("No Default Audio Device enabled. Not loading sounds.").c_str());
    audio.loaded = true;
    return; // no available devices
  }
  SDL_Log("%s", std::format("Using audiodevice: %s", device_name).c_str());
  audio.captured_device_id = Mix_OpenAudioDevice(spec.freq, spec.format, spec.channels, chunk_size, device_name, 0);

  // Check if that worked.
  if (audio.captured_device_id == -1) {
    SDL_Log("%s", std::format("No Default Audio Device enabled. Not loading sounds.").c_str());
    return;
  }

  // request some channels
  const int request_channels = 16;
  Mix_AllocateChannels(request_channels);
  audio.max_audio_sources = Mix_AllocateChannels(-1); // -1 means query the number of channels
  SDL_Log("%s", std::format("Audio sources to create: %i", audio.max_audio_sources).c_str());
  for (int i = 0; i < audio.max_audio_sources; i++) {
    create_persistent<AudioSource>(r, AudioSource(i));

    // set volume to user pref
    Mix_Volume(i, audio.volume_internal);
  }

  SDL_Log("%s", std::format("Loading audio...").c_str());
  for (auto& file : audio.sounds) {
    auto* sound = Mix_LoadWAV(file.path.c_str());
    if (!sound) {
      SDL_Log("%s", std::format("Failed to load sound: %s, %s", file.path.c_str(), Mix_GetError()).c_str());
      continue;
    }
    file.buffer = sound;
  }

  SDL_Log("%s", std::format("Loaded audio.").c_str());
  audio.loaded = true;
}

void
update_audio_system(entt::registry& r)
{
  const auto audio_e = get_first<SINGLE_AudioComponent>(r);
  if (audio_e == entt::null)
    return;

  const auto& audio = get_first_component<SINGLE_AudioComponent>(r);
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
  for (const auto& [e, request] : r.view<AudioRequestPlayEvent>().each())
    compacted_requests[request.tag].push_back(e);

  // state: process request -> playing
  for (const auto& [tag, entities] : compacted_requests) {

    // assume audio request with same tag are the same
    const auto& entity = entities[0];
    const auto& request = r.get<AudioRequestPlayEvent>(entity);

    if (free_audio_sources.size() == 0) {
      SDL_Log("%s", std::format("No free audio sources! Missed request for: {}", tag).c_str());

      continue;
    }

    AudioSource& audio_source = free_audio_sources.front();
    free_audio_sources.erase(free_audio_sources.begin());
    audio_source.state = AudioSourceState::PLAYING;

    const Sound s = get_sound(audio, request.tag);
    const int channel = Mix_PlayChannel(audio_source.channel, s.buffer, request.looping ? -1 : 0);
    if (channel != audio_source.channel) {
      SDL_Log("%s", std::format("Warning: sound playing on incorrect channel").c_str());
    }

    // process request
    r.destroy(entities.begin(), entities.end());
  }
};

} // namespace game2d