#include "system.hpp"

#include "audio/helpers/openal.hpp"
#include "helpers.hpp"

#include <iostream>
#include <vector>

namespace game2d {

void
init_audio_system(SINGLETON_AudioComponent& audio)
{
  audio::init_al();

  for (Sound& sound : audio.sounds)
    sound.loaded_id = audio::load_sound(sound.path);
}

void
update_audio_system(entt::registry& r, SINGLETON_AudioComponent& audio)
{
  // todo: check for the same audio clip being played twice in the same frame
  // todo: fix audio source being created every request -- ring buffer?
  // todo: process audio device disconnect and connect events
  // todo: how to swap audio device?
  // todo: how to list connected audio devices?

  // Check if sources have finished playing audio
  for (const auto [entity, event] : r.view<AudioPlayingEvent>().each()) {
    ALint source_state;
    alGetSourcei(event.source_id, AL_SOURCE_STATE, &source_state);
    if (source_state != AL_PLAYING)
      alDeleteSources(1, &event.source_id);
    r.destroy(entity);
  }

  // Check queue for audio request
  for (const auto [entity, event] : r.view<AudioRequestPlayEvent>().each()) {
    std::cout << "processing audio event\n";

    // generate source
    ALuint source_id;
    alGenSources(1, &source_id);
    std::cout << "sourceid: " << source_id << "\n";
    // alSourcef(music, AL_PITCH, 1.0f); // pitch
    // ALenum audio_state;
    // alGetSourcei(music, AL_SOURCE_STATE, &audio_state); // state
    // ALfloat audio_offset;
    // alGetSourcef(music, AL_SEC_OFFSET, &audio_offset); // offset

    ALuint sound_id = get_sound_id(audio, event.tag);
    alSourcei(source_id, AL_BUFFER, (ALint)sound_id);   // attach buffer
    alSourcef(source_id, AL_GAIN, audio.master_volume); // set volume
    alSourceStop(source_id);
    alSourcePlay(source_id);

    AudioPlayingEvent playing_event;
    playing_event.source_id = source_id;
    r.remove<AudioRequestPlayEvent>(entity); // process the event
    r.emplace<AudioPlayingEvent>(entity);
  };
};

} // namespace game2d