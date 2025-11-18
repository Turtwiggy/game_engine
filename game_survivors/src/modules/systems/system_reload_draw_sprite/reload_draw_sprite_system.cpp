
#include "reload_draw_sprite_system.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/combat/combat_projectiles/projectile_components.hpp"
#include "modules/combat/combat_weapon_core/combat_weapon_core_components.hpp"
#include "modules/core/renderer/components.hpp"
#include "modules/core/sprites/sprite_helpers.hpp"
#include "resources/data.hpp"

namespace game2d {

void
update_reload_draw_sprite_system(entt::registry& r)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif

  const auto view = r.view<const WeaponDef, WeaponFireRate, WeaponReloadRate, WeaponClipSize>();
  for (const auto& [wep_e, wep_def, weapon_fire_rate_c, weapon_reload_rate_c, weapon_clip_size_c] : view.each()) {
    if (weapon_reload_rate_c.seconds_cur <= 0.0f)
      continue;

    auto parent_e = r.get<HasParentComponent>(wep_e);
    if (parent_e.parent == entt::null || !r.valid(parent_e.parent))
      continue;

    auto* player_c = r.try_get<PlayerComponent>(parent_e.parent);
    if (!player_c)
      continue;

    const auto player_idx = player_c->idx;

    // draw reload sprite.
    Sprite desc{
      .sprite = "CROSS",
      .pos = get_position(r, wep_e),
      .size = { 8, 8 },
      .z_idx = ZLayer::FOREGROUND,
      // .col = default_player_colours[player_idx],
      .col = my_reload_col,
    };
    draw_sprite(r, desc);
  }
}

} // namespace game2d