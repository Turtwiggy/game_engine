#include "pch.hpp"

#include "bump_repair_event_helpers.hpp"
#include "engine/audio/audio_components.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/maths/maths.hpp"
#include "modules/actors/actor_island_cannon/island_cannon_components.hpp"
#include "modules/combat/combat_weapon_core/combat_weapon_core_components.hpp"

namespace game2d {

void
handle_bump_event__repair(entt::registry& r, const BumpEvent& evt)
{
  auto* children_c = r.try_get<HasChildrenComponent>(evt.to);
  if (!children_c)
    return;

  entt::entity cannon_e = entt::null;
  for (const auto child_e : children_c->children) {
    auto* cannon_c = r.try_get<IslandCannonComponent>(child_e);
    if (cannon_c) {
      cannon_e = child_e;
      break;
    }
  }
  if (cannon_e == entt::null)
    return;
  auto& cannon_c = r.get<IslandCannonComponent>(cannon_e);

  // repair the thing.
  cannon_c.hits_to_repair_left--;
  cannon_c.hits_to_repair_left = std::max(cannon_c.hits_to_repair_left, 0);

  // is it repaired?
  bool repaired = cannon_c.hits_to_repair_left == 0;
  if (repaired && cannon_c.state == IslandCannonState::BROKEN)
    cannon_c.state = IslandCannonState::WORKING;

  // refill the ammo.
  auto& mag_c = r.get<WeaponClipSize>(cannon_e);
  // if (repaired && mag_c.bullets_cur == 0)
  if (repaired) // allow fill up bullets when repaired
    mag_c.bullets_cur = mag_c.bullets_max;

  // play some audio
  static engine::RandomState rnd(0);
  const auto track_key = std::format("CLANK_01");
  AudioRequestPlayEvent audio_evt;
  audio_evt.tag = track_key;
  create_empty<AudioRequestPlayEvent>(r, audio_evt);

  // ideas
  // show some sparks?
  // change the sprite?
}

} // namespace game2d