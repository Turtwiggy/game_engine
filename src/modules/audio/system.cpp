#include "system.hpp"

#include "helpers.hpp"

namespace game2d {

static ALuint temporary = 0;

void
init_audio_system(GameEditor& editor)
{
  audio::init_al();
  temporary = audio::load_sound("assets/audio/chase.mp3");
}

void
update_audio_system(GameEditor& editor, Game& game){
  //
};

} // namespace game2d