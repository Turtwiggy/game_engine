#include "exploder_screenshake.hpp"
#include "engine/entt/helpers.hpp"
#include "modules/system_screenshake/components.hpp"
#include "modules/system_traits/trait_components.hpp"

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

  auto* traits_c = r.try_get<TraitComponent>(dead_e);
  if (!traits_c)
    return;

  auto it = std::find(traits_c->traits.begin(), traits_c->traits.end(), AquirableTrait::EXPLODE);
  if (it == traits_c->traits.end())
    return;

  create_empty<RequestScreenshakeComponent>(r, RequestScreenshakeComponent{ ScreenshakeType::EXPLODE });
};

} // namespace game2d