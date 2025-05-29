#include "pch.hpp"

#include "aoe_slow_components.hpp"
#include "aoe_slow_system.hpp"
#include "engine/actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/physics/physics_helpers.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/actors/actor_enemy/components.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/combat/combat_core/components.hpp"
#include "modules/combat/combat_elemental_damage/elemental_damage_components.hpp"
#include "modules/events/event_damage/event_damage_components.hpp"
#include "modules/events/events_core/events_components.hpp"
#include "modules/systems/system_particles/components.hpp"

namespace game2d {

void
update_aoe_slow_system(entt::registry& r, const float dt)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif

  for (const auto& [e, slow_c] : r.view<AoE_SlowComponent>().each()) {
    slow_c.time_between_slow_seconds -= dt;
    if (slow_c.time_between_slow_seconds > 0.0)
      continue;
    slow_c.time_between_slow_seconds = slow_c.time_between_slow_seconds_max;

    //
    // produce an aoe slow effect.
    //

    // produce an aoe slow particles.
    RequestToSpawnParticles request;
    request.key = "vfx_ice_boop";
    request.position = get_position(r, e);
    create_empty<RequestToSpawnParticles>(r, request);

    const std::function<bool(entt::registry&, entt::entity)> cond = [](entt::registry& r, entt::entity e) -> bool {
      bool valid_target = false;
      valid_target |= r.try_get<EnemyComponent>(e) != nullptr;
      return valid_target;
    };

    // n.b.: radius so half
    const float explosion_radius_pixels = 50;
    const float explosion_radius_meters = pixels_to_meters(explosion_radius_pixels);
    const b2Vec2 center_m = pixels_to_meters(get_position(r, e));
    const auto enemies = get_all_in_area_filtered(r, center_m, explosion_radius_meters, cond);

    for (const auto& [enemy_e, coll_fixtures] : enemies) {
      for (const auto coll_result : coll_fixtures) {
        // add some ice stacks to them to slow them.
        auto& ice = r.get_or_emplace<TickDamageComponent>(coll_result.fixture_e).ice;
        const float time = 3.0f;
        ice.push_back({ WEAPON_DAMAGE::ICE, time });
        break; // only apply to 1st fixture
      }
    }
  }
}

} // namespace game2d