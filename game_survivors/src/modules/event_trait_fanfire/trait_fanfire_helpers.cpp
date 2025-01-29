#include "trait_fanfire_helpers.hpp"

#include "modules/system_traits/trait_components.hpp"
#include "modules/system_traits/trait_helpers.hpp"

#include <SDL2/SDL_log.h>

namespace game2d {

void
handle_shoot_event__trait_fanfire(entt::registry& r, const ShootEvent& evt)
{
  const auto from_e = evt.parent_e;
  const auto wep_e = evt.weapon_e;

  if (from_e == entt::null)
    return;

  const auto* trait_c = r.try_get<TraitComponent>(from_e);
  if (!trait_c)
    return;

  auto trait = AquirableTrait::FAN_FIRE;
  if (!has_trait(r, trait_c->traits, trait))
    return;

  SDL_Log("player's gun shot");
}

} // namespace game2d