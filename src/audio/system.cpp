#include "system.hpp"

#include "audio/helpers/openal.hpp"
#include "components.hpp"
#include "helpers.hpp"
#include "renderer/components.hpp"

#include <imgui.h>

#include <iostream>
#include <map>
#include <string>
#include <vector>

namespace game2d {

void
init_audio_system(SINGLETON_AudioComponent& audio)
{
  audio::init_al();

  for (Sound& sound : audio.sounds)
    sound.loaded_id = audio::load_sound(sound.path);

  const int max_audio_sources = 16;
  // for (int i = 0; i < max_audio_sources; i++) {
  //   const auto& e = r.create();
  //   ALuint source_id;
  //   alGenSources(1, &source_id);
  //   r.emplace<TagComponent>(e, "Audio Source");
  //   r.emplace<AudioSource>(e, source_id);
  //   // alSourcef(music, AL_PITCH, 1.0f); // pitch
  //   // alGetSourcei(music, AL_SOURCE_STATE, &audio_state); // state
  //   // alGetSourcef(music, AL_SEC_OFFSET, &audio_offset); // offset
  // }
}

void
update_audio_system(SINGLETON_AudioComponent& audio){
  // todo: fix audio source being created every request -- ring buffer?
  // todo: process audio device disconnect and connect events
  // todo: how to swap audio device?
  // todo: how to list connected audio devices?

  // // a vector of free audio sources, populated every frame
  // std::vector<AudioSource> free_audio_sources;

  // // state: playing -> free
  // for (const auto& [entity, source] : r.view<AudioSource>().each()) {
  //   ALint source_state;
  //   alGetSourcei(source.source_id, AL_SOURCE_STATE, &source_state);

  //   source.state = AudioSourceState::FREE;
  //   if (source_state == AL_PLAYING)
  //     source.state = AudioSourceState::PLAYING;

  //   if (source.state == AudioSourceState::FREE)
  //     free_audio_sources.push_back(source);
  // }

  // // compact duplicate audio requests
  // std::map<std::string, std::vector<entt::entity>> compacted_requests;
  // for (auto [entity, request] : r.view<AudioRequestPlayEvent>().each())
  //   compacted_requests[request.tag].push_back(entity);

  // // state: process request -> playing
  // for (const auto& [tag, entities] : compacted_requests) {
  //   const auto& entity = entities[0]; // assume audio request with same tag are the same
  //   const auto& request = r.get<AudioRequestPlayEvent>(entity);
  //   std::cout << "processing play audio request\n";

  //   if (free_audio_sources.size() == 0) {
  //     std::cout << "no free audio sources!\n";
  //     continue;
  //   }
  //   AudioSource& audio_source = free_audio_sources.front();
  //   free_audio_sources.erase(free_audio_sources.begin());

  //   // set as playing
  //   const auto& source_id = audio_source.source_id;
  //   ALuint sound_id = get_sound_id(audio, request.tag);
  //   alSourcei(source_id, AL_BUFFER, (ALint)sound_id);   // attach buffer
  //   alSourcef(source_id, AL_GAIN, audio.master_volume); // set volume
  //   alSourceStop(source_id);
  //   alSourcePlay(source_id);

  //   // process request
  //   r.remove<AudioRequestPlayEvent>(entity);
};

} // namespace game2d