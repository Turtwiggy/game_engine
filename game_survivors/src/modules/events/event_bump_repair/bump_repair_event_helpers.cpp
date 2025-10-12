#include "pch.hpp"

#include "bump_repair_event_helpers.hpp"
#include "engine/audio/audio_components.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/maths/maths.hpp"
#include "modules/actors/actor_island_cannon/island_cannon_components.hpp"

namespace game2d {

void
handle_bump_event__repair(entt::registry& r, const BumpEvent& evt)
{
  auto* cannon_c = r.try_get<IslandCannonComponent>(evt.to);
  if (!cannon_c)
    return;

  // repair the thing.
  cannon_c->hits_to_repair_left--;
  cannon_c->hits_to_repair_left = std::max(cannon_c->hits_to_repair_left, 0);

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