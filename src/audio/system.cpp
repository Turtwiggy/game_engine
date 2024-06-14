#include "system.hpp"

#include "audio/helpers/openal.hpp"
#include "components.hpp"
#include "helpers.hpp"

#include "entt/helpers.hpp"
#include <entt/entt.hpp>
#include <imgui.h>

#include <iostream>
#include <map>
#include <string>
#include <vector>

namespace game2d {

// todo: fix audio source being created every request -- ring buffer?
// todo: a request is only processed when the audio source has finished playing the sound

void
init_audio_system(entt::registry& r)
{
  const int max_audio_sources = 16;
  for (int i = 0; i < max_audio_sources; i++) {
    const auto audio_e = create_empty<AudioSource>(r);
    r.emplace<AudioSource>(audio_e);
  }

  open_audio_new_device(r, std::nullopt);
}

void
update_audio_system(entt::registry& r)
{
  const auto& audio = get_first_component<SINGLETON_AudioComponent>(r);

  // If muted, destroy all requests
  if (audio.all_mute) {
    const auto& view = r.view<AudioRequestPlayEvent>();
    r.destroy(view.begin(), view.end());
  }

  // a vector of free audio sources, populated every frame
  std::vector<AudioSource> free_audio_sources;

  // state: playing -> free
  for (const auto& [entity, source] : r.view<AudioSource>().each()) {

    ALint source_state;
    alGetSourcei(source.source_id, AL_SOURCE_STATE, &source_state);

    source.state = AudioSourceState::FREE;
    if (source_state == AL_PLAYING)
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
      std::cout << "no free audio sources!"
                << "missed request for: " << tag << std::endl;
      continue;
    }

    AudioSource& audio_source = free_audio_sources.front();
    free_audio_sources.erase(free_audio_sources.begin());
    audio_source.state = AudioSourceState::PLAYING;

    ALint id = (ALint)get_sound(audio, request.tag).result;
    const auto& source_id = audio_source.source_id;
    alSourcei(source_id, AL_BUFFER, id);                // attach buffer
    alSourcef(source_id, AL_GAIN, audio.master_volume); // set volume
    alSourceStop(source_id);
    alSourcePlay(source_id);

    // process request
    r.destroy(entities.begin(), entities.end());
  }
};

} // namespace game2d