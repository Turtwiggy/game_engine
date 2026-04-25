#include "pch.hpp"

#include "island_cannon_repair_system.hpp"

#include "engine/audio/audio_components.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/maths/maths.hpp"
#include "modules/actors/actor_island_cannon/island_cannon_components.hpp"
#include "modules/combat/combat_weapon_core/combat_weapon_core_components.hpp"

namespace game2d {

void
update_island_cannon_repair_system(entt::registry& r, float delta)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif

  auto view = r.view<IslandCannonComponent, WeaponClipSize>();
  for (const auto& [e, island_cannon_c, mag_c] : view.each()) {

    if (island_cannon_c.time_to_repair_left > 0.f) {
      island_cannon_c.time_to_repair_left -= delta;

      if (island_cannon_c.time_to_repair_left <= 0.f) {
        island_cannon_c.state = IslandCannonState::WORKING;

        // refill the ammo
        mag_c.bullets_cur = mag_c.bullets_max;

        // play some audio
        static engine::RandomState rnd(0);
        const auto track_key = std::format("CLANK_01");
        AudioRequestPlayEvent audio_evt;
        audio_evt.tag = track_key;
        create_empty<AudioRequestPlayEvent>(r, audio_evt);
      }
    }
  }
}

} // namespace game2d