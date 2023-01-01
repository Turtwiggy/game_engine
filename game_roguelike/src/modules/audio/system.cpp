#include "system.hpp"

#include "components.hpp"
#include "resources/audio.hpp"
#include "audio/helpers/openal.hpp"

// temporary
#include "modules/player/components.hpp"
#include "events/helpers/keyboard.hpp"

namespace game2d {

// temporary
static float master_volume = 0.1f;

void
init_audio_system(GameEditor& editor)
{
  auto& audio = editor.audio;

  audio::init_al();

  for (Sound& sound : audio.sounds)
    sound.loaded_id = audio::load_sound(sound.path);
}

void
update_audio_system(GameEditor& editor, Game& game)
{
  // todo: process audio disconnect and connect events
  // todo: how to swap audio device?
  // todo: how to list connected audio devices?

  auto& r = game.state;
  for (const auto [entity, audio] : r.view<AudioSource>().each()) {
    //
    if (audio.source_id == 0) {
      // generate source
      alGenSources(1, &audio.source_id);
      // set pitch
      // alSourcef(audio_source_continuous_music, AL_PITCH, 1.0f);
      // get state of source
      // ALenum audio_state;
      // alGetSourcei(audio_source_continuous_music, AL_SOURCE_STATE, &audio_state);
      // get offset of source
      // ALfloat audio_offset;
      // alGetSourcef(audio_source_continuous_music, AL_SEC_OFFSET, &audio_offset);
    }

    for (const AvailableSound& audio_request : audio.wants_to_play) {

      ALuint sound_id = get_sound_id(editor.audio, audio_request);

      // attach buffer to source
      alSourcei(audio.source_id, AL_BUFFER, (ALint)sound_id);

      // set volume?
      alSourcef(audio.source_id, AL_GAIN, master_volume);

      audio::play_sound(audio.source_id);
    }

    audio.wants_to_play.clear();
  }

  // if (get_key_down(game.input, SDL_SCANCODE_R)) {
  //   auto player_id = r.view<PlayerComponent>().front();
  //   auto& audio_id = r.get<AudioSource>(player_id);
  //   audio_id.wants_to_play.push_back(AvailableSound::menu);
  // }
};

} // namespace game2d