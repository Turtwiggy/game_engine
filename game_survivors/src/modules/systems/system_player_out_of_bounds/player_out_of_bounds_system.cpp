#include "pch.hpp"

#include "player_out_of_bounds_system.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/colour/colour.hpp"
#include "engine/physics/physics_helpers.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/actors/actor_rock/rock_components.hpp"
#include "modules/combat/combat_core/components.hpp"
#include "modules/core/renderer/components.hpp"
#include "modules/core/sprites/sprite_helpers.hpp"
#include "modules/events/event_damage/event_damage_components.hpp"
#include "modules/events/events_core/events_components.hpp"
#include "modules/systems/system_cooldown/helpers.hpp"
#include "modules/systems/system_upgrade_hp_regen/upgrade_hp_regen_components.hpp"
#include "player_out_of_bounds_components.hpp"

namespace game2d {

void
update_player_out_of_bounds_system(entt::registry& r, const float dt)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif
  auto& evts_c = SINGLE_Events::instance;

  const int tilesize = SINGLE_Islands::instance.tilesize;
  const int tiles = 20;
  const int map_radius = tilesize * tiles;
  const int map_radius_sqr = map_radius * map_radius;

  for (const auto& [e, player_c, damage_c] : r.view<PlayerComponent, OutOfBoundsTimer>().each()) {

    const auto d = get_position(r, e) - glm::vec2{ 0, 0 };
    const auto d2 = d.x * d.x + d.y * d.y;

    // #if defined(_DEBUG)
    //     ImGui::Text("%f %i", d2, map_radius_sqr);
    // #endif

    if (d2 < map_radius_sqr)
      continue; // in the safe-zone circle

    // start to apply damage to player!;
    Sprite s;
    s.pos = get_position(r, e);
    s.sprite = "SKULL_AND_BONES";
    s.size = { 32, 32 };
    s.col = engine::SRGBColour(1.0f, 0.0f, 0.0f, 1.0f);
    s.z_idx = ZLayer::FOREGROUND;
    draw_sprite(r, s);

    // damage every 2 seconds.
    damage_c.cooldown_c.time -= dt;
    if (damage_c.cooldown_c.time > 0.0f)
      continue;
    reset_cooldown(damage_c.cooldown_c);

    // deal 10% of your max health
    auto core_e = get_fixture_by_tag(r, e, "fixture_player");
    const auto& hp_c = r.get<HealthComponent>(core_e);
    float damage_per_tick = hp_c.max_hp * 0.1f;

    // note: consider regen.
    if (auto* regen_c = r.try_get<ActorHealthRegenComponent>(e)) {
      const auto hp_per_sec = regen_c->hp_per_second;
      const auto regen_damage = hp_per_sec * damage_c.cooldown_c.time_max;
      damage_per_tick += regen_damage; // completely offset regen
    }

    DamageEvent evt;
    evt.from = entt::null; // likely dead
    evt.to_parent = e;
    evt.to_fixture = core_e;
    evt.type = WEAPON_DAMAGE::KINETIC;
    evt.amount = damage_per_tick;
    evts_c.dispatcher->trigger(evt);
    evts_c.dispatcher->update();
  }
}

} // namespace game2d