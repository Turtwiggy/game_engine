#include "exploder_screenshake.hpp"

#include <SDL2/SDL_log.h>

namespace game2d {

void
handle_death_event__exploder_screenshake(entt::registry& r, const DeathEvent& evt)
{
  //
  // once the exploder has finished their deaththroes...
  //
  auto dead_e = evt.dead;
  if (dead_e == entt::null)
    return;

  SDL_Log("TODO: add screenshake for exploder");
};

} // namespace game2d