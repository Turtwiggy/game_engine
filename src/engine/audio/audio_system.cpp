#include "pch.hpp"

#include "audio_system.hpp"

// this is the engine/ directory... modules/ shouldn't be here...
#include "game_state.hpp"

#include "audio_components.hpp"
#include "engine/audio/audio_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "modules/events/events_core/events_components.hpp"

namespace game2d {

void
init_audio_system(entt::registry& r)
{
  auto& audio = get_first_component<SINGLE_AudioComponent>(r);

  if (!refresh_audio(r)) {
    SDL_Log("Audio init failed; (no audio device.)");
    audio.loaded = true;
    return;
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
#if defined(_DEBUG)
  ZoneScoped;
#endif
  GET_FIRST_OR_RETURN(SINGLE_AudioComponent, r, audio_e, audio_c);
  GET_FIRST_OR_RETURN(SINGLE_GameStateComponent, r, state_e, state_c);
  const auto& evts_c = SINGLE_Events::instance;

  // dampen music if paused
  // bool paused = state_c.state == GameState::PAUSED;
  // paused |= require_pause(r); // gameplay logic

  // NOTE: this DOESNT dampen the music if the options menu is open.
  // this is due to the fact that if audio settings are being adjusted,
  // you dont want to adjust them while listening to damped audio,
  // then go back and have the audio suddenly be loud.
  // const auto& pause_menu_c = get_first_component<SINGLE_PauseMenuState>(r);
  // const bool dampen_music = pause_menu_c.open;
  const bool dampen_music = false;

  if (!audio_c.loaded)
    return;

  if (audio_c.sounds.empty())
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
  std::vector<entt::entity> free_audio_sources;

  // state: playing -> free
  for (const auto& [e, source] : r.view<AudioSource>().each()) {

    const auto is_playing = Mix_Playing(source.channel) == 1;
    const auto old_state = source.state;
    source.state = is_playing ? AudioSourceState::PLAYING : AudioSourceState::FREE;

    if (old_state == AudioSourceState::PLAYING && !is_playing) {
      // track potentially completed.
      if (source.sound_type == SoundType::BACKGROUND) {
        SDL_Log("bg sound complete: %s", source.sound.c_str());
        AudioCompleteEvent evt;
        evt.tag = source.sound;
        evts_c.dispatcher->trigger(evt);
        evts_c.dispatcher->update();
      }

      source.state = AudioSourceState::FREE;
      source.sound = "";
    }

    if (source.state == AudioSourceState::FREE)
      free_audio_sources.push_back(e);
  }

  // Update the volume of all playing sources.
  for (const auto& [entity, source] : r.view<AudioSource>().each()) {
    const auto is_playing = source.state == AudioSourceState::PLAYING;
    if (!is_playing)
      continue;
    update_audio_channel_volume(source, volume_sfx, volume_music, dampen_music);
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

    if (free_audio_sources.empty()) {
      // SDL_Log("%s", std::format("No free audio sources! Missed request for: {}", tag).c_str());
      r.destroy(entities.begin(), entities.end());
      continue;
    }

    const Sound s = get_sound(audio_c, request.tag);
    const entt::entity audio_source_e = free_audio_sources.front();
    free_audio_sources.erase(free_audio_sources.begin());
    auto& audio_source_c = r.get<AudioSource>(audio_source_e);
    audio_source_c.state = AudioSourceState::PLAYING;
    audio_source_c.sound = tag;
    audio_source_c.sound_type = s.type;
    update_audio_channel_volume(audio_source_c, volume_sfx, volume_music, dampen_music);

    // start play
    const int channel = Mix_PlayChannel(audio_source_c.channel, s.buffer, request.looping ? -1 : 0);

    if (channel != audio_source_c.channel)
      SDL_Log("%s", std::format("Warning: sound playing on incorrect channel").c_str());

    // process request
    r.destroy(entities.begin(), entities.end());
  }
};

} // namespace game2d