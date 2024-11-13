#include "trait_bleed_helpers.hpp"

#include "modules/system_combat_bleed/combat_bleed_components.hpp"

namespace game2d {

void
handle_damage_event_apply_bleed(entt::registry& r, const DamageEvent& evt)
{
  const auto to_e = evt.to;

  // apply bleed trait

  for (const Trait& trait : evt.traits) {
    if (trait.key.find("bleed") == std::string::npos)
      continue;
    SDL_Log("Applying bleed effect");

    // TODO(gameplay): dont emplace_or_replace, just extend duration?
    r.emplace_or_replace<BleedComponent>(to_e);
  }
}

} // namespace game2d