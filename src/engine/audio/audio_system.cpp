#include "pch.hpp"

#include "audio_system.hpp"

// this is the engine/ directory... modules/ shouldn't be here...
#include "game_state.hpp"
#include "modules/system_pause/pause_helpers.hpp"

#include "audio_components.hpp"
#include "engine/audio/audio_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include <stdexcept>

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
  SDL_Log("%s", std::format("Using audiodevice: {}", device_name).c_str());
  audio.captured_device_id = Mix_OpenAudioDevice(spec.freq, spec.format, spec.channels, chunk_size, device_name, 0);

  // Check if that worked.
  if (audio.captured_device_id == -1) {
    SDL_Log("%s", std::format("No Default Audio Device enabled. Not loading sounds.").c_str());
    return;
  }

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

  SDL_Log("%s", std::format("Loading audio...").c_str());
  for (auto& file : audio.sounds) {
    auto* sound = Mix_LoadWAV(file.path.c_str());
    if (!sound) {
      auto err = std::format("Failed to load sound: {}, {}", file.path.c_str(), Mix_GetError());
      SDL_Log("%s", err.c_str());
      throw std::runtime_error(err);
      continue;
    }
    file.buffer = sound;
  }

  SDL_Log("%s", std::format("Loaded audio.").c_str());
  audio.loaded = true;
};

void
update_audio_channel_volume(const AudioSource& source, float vol_sfx, float vol_mus, bool paused)
{
  const auto t = source.sound_type;

  float volume = MIX_MAX_VOLUME;

  if (t == SoundType::SFX)
    volume *= vol_sfx;
  else if (t == SoundType::BACKGROUND)
    volume *= vol_mus;
  else
    throw std::runtime_error("unknown audio type");

  if (paused)
    volume *= 0.4f;

  const int volume_int = (int)volume;
  Mix_Volume(source.channel, volume_int);
};

void
update_audio_system(entt::registry& r, const float dt)
{
  GET_FIRST_OR_RETURN(SINGLE_AudioComponent, r, audio_e, audio_c);
  GET_FIRST_OR_RETURN(SINGLE_GameStateComponent, r, state_e, state_c);

  // dampen music if paused
  bool paused = state_c.state == GameState::PAUSED;
  paused |= require_pause(r); // gameplay logic

  if (!audio_c.loaded)
    return;

  if (audio_c.sounds.size() == 0)
    return; // no sounds loaded

  // audio levels
  const float epsilon = 0.001f;
  const bool muted_master = audio_c.volume_master - epsilon <= 0.0f;
  const bool muted_music = audio_c.volume_music - epsilon <= 0.0f;
  const bool muted_sfx = audio_c.volume_sfx - epsilon <= 0.0f;
  const float volume_music = audio_c.volume_master * audio_c.volume_music;
  const float volume_sfx = audio_c.volume_master * audio_c.volume_sfx;

  // If muted, destroy all requests
  // if (muted_master) {
  //   const auto& view = r.view<AudioRequestPlayEvent>();
  //   r.destroy(view.begin(), view.end());
  // }

  // check if request is a sfx effect
  for (const auto& [e, req] : r.view<const AudioRequestPlayEvent>().each()) {
    const Sound& s = get_sound(audio_c, req.tag);
    if (muted_sfx && s.type == SoundType::SFX)
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

  // Update the volume of all playing sources.
  for (const auto& [entity, source] : r.view<AudioSource>().each()) {
    const auto is_playing = source.state == AudioSourceState::PLAYING;
    if (!is_playing)
      continue;
    update_audio_channel_volume(source, volume_sfx, volume_music, paused);
  }

  // compact duplicate audio requests
  std::map<std::string, std::vector<entt::entity>> compacted_requests;
  for (const auto& [e, request] : r.view<const AudioRequestPlayEvent>().each())
    compacted_requests[request.tag].push_back(e);

  // state: process request -> playing
  for (const auto& [tag, entities] : compacted_requests) {
    SDL_Log("Audio request to play... %s", tag.c_str());

    // assume audio request with same tag are the same
    const auto& entity = entities[0];
    const auto& request = r.get<AudioRequestPlayEvent>(entity);

    if (free_audio_sources.size() == 0) {
      // SDL_Log("%s", std::format("No free audio sources! Missed request for: {}", tag).c_str());
      return;
    }

    AudioSource& audio_source = free_audio_sources.front();
    free_audio_sources.erase(free_audio_sources.begin());
    audio_source.state = AudioSourceState::PLAYING;

    const Sound s = get_sound(audio_c, request.tag);

    const int channel = Mix_PlayChannel(audio_source.channel, s.buffer, request.looping ? -1 : 0);
    if (channel != audio_source.channel)
      SDL_Log("%s", std::format("Warning: sound playing on incorrect channel").c_str());
    audio_source.sound_type = s.type;
    update_audio_channel_volume(audio_source, volume_sfx, volume_music, paused);

    // process request
    r.destroy(entities.begin(), entities.end());
  }
};

} // namespace game2d