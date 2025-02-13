#include "autofire_system.hpp"

#include "autofire_components.hpp"
#include "engine/actors/actor_helpers.hpp"
#include "engine/colour/colour.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/imgui/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/maths/maths.hpp"
#include "engine/physics/physics_components.hpp"
#include "engine/physics/physics_helpers.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/actor_enemy/components.hpp"
#include "modules/combat_gun_follow_player/gun_follow_player_components.hpp"
#include "modules/combat_projectiles/projectile_helpers.hpp"
#include "modules/core_colour/components.hpp"
#include "modules/core_sprites/sprite_helpers.hpp"
#include "modules/event_shoot/event_shoot_components.hpp"
#include "modules/events/events_components.hpp"
#include "modules/system_autofire/autofire_helpers.hpp"
#include "modules/system_cooldown/components.hpp"
#include "modules/system_cooldown/helpers.hpp"
#include "modules/system_hulls/hulls_components.hpp"

#include <box2d/b2_collision.h>
#include <magic_enum.hpp>

#include <algorithm>

namespace game2d {

void
filter_enemies_by_shoot_angle(entt::registry& r,
                              std::vector<std::pair<int, entt::entity>>& enemies,
                              const HardpointComponent& hardpoint_c,
                              const glm::vec2 wep_pos)
{
  std::vector<std::pair<int, entt::entity>> valid_targets;

  for (const auto& enemy : enemies) {
    const auto enemy_pos = get_position(r, enemy.second);

    const auto raw_dir = enemy_pos - wep_pos;
    const auto nrm_dir = engine::normalize_safe(raw_dir);
    const auto angle = engine::dir_to_angle_radians(nrm_dir);

    // GAMEPLAY: Clamp the dir between the hardpoint's calculated min_arc and max_arc
    const auto left = hardpoint_c.dir_arc_left;
    const auto right = hardpoint_c.dir_arc_right;
    const auto min = engine::dir_to_angle_radians(left);
    const auto max = engine::dir_to_angle_radians(right);

    // min is in the range 0, 2PI.
    // max is in the range 0, 2PI.
    // an enemy is approaching from the range 0, 2PI.

    // min > max, so max has looped round
    if (min > max && (angle >= min || angle <= max)) {
      valid_targets.push_back(enemy);
    } else if (angle >= min && angle <= max)
      valid_targets.push_back(enemy);
  }

  enemies = valid_targets;
};

void
update_autofire_system(entt::registry& r, glm::vec2 mouse_pos)
{
  GET_FIRST_OR_RETURN(SINGLE_Physics, r, phys_e, phys_c);
  GET_FIRST_OR_RETURN(SINGLE_Events, r, evts_e, evts_c)

  auto& dead = get_first_component<SINGLE_EntityBinComponent>(r);

  static float lead_amount = 0.4f;
#if defined(_DEBUG)
  // imgui_draw_float("shot lead amount", lead_amount);
#endif

  const auto view = r.view<TransformComponent,
                           const WeaponComponent,
                           const HasParentComponent,
                           const HardpointComponent,
                           const AutofireComponent,
                           CooldownComponent>();

  for (const auto& [wep_e, wep_t, wep_c, parent_c, hardpoint_c, autofire_c, cooldown_c] : view.each()) {

    // if (cooldown_c.time > 0.0f)
    //   continue;
    // reset_cooldown(cooldown_c);

    const auto p = parent_c.parent;
    const auto& parent_t = r.get<TransformComponent>(p);
    const auto& parent_col = r.get<DefaultColour>(p).colour;

    // get closest enemy (from your gun position)
    const auto wep_pos = glm::vec2{ wep_t.position.x, wep_t.position.y };
    const auto search_radius_meters = 3.0f; // for nearest enemy
    const std::function<bool(entt::registry&, entt::entity)> is_enemy = [](entt::registry& r, entt::entity e) -> bool {
      return r.try_get<EnemyComponent>(e) != nullptr;
    };
    const b2Vec2 center_m = pixels_to_meters(wep_pos);
    auto enemies = get_all_in_area_filtered(r, center_m, search_radius_meters, is_enemy);
    if (enemies.size() == 0)
      continue;

    // Filter by angle that this weapon can shoot
    filter_enemies_by_shoot_angle(r, enemies, hardpoint_c, wep_pos);
    if (enemies.size() == 0)
      continue;

    // Get the nearest enemy
    auto sort_by_distance = [](const auto& a, const auto& b) { return a.first < b.first; };
    std::sort(enemies.begin(), enemies.end(), sort_by_distance);
    auto nearest_e = enemies[0].second;

    const auto par_pos = glm::vec2{ parent_t.position.x, parent_t.position.y };
    const auto tgt_pos = get_position(r, nearest_e);

    // Note: Adjust the angle, so that the auto-fire leads it's shot a little
    const auto tgt_vel_m = r.get<PhysicsBodyComponent>(nearest_e).body->GetLinearVelocity();
    const auto tgt_vel_p = meters_to_pixels(tgt_vel_m);
    const auto smarter_tgt_pos = tgt_pos + glm::vec2{ tgt_vel_p.x * lead_amount, tgt_vel_p.y * lead_amount };

    // debug the nearest enemy
    // Sprite tgt_s;
    // tgt_s.pos = tgt_pos;
    // tgt_s.sprite = "CROSSHAIR_1";
    // tgt_s.size = { 16, 16 };
    // tgt_s.col = parent_col;
    // draw_sprite(r, tgt_s);

    // debug the adj tgt pos
    Sprite adj_tgt_s;
    adj_tgt_s.pos = smarter_tgt_pos;
    adj_tgt_s.sprite = "CROSSHAIR_2";
    adj_tgt_s.size = { 16, 16 };
    adj_tgt_s.col = parent_col;
    draw_sprite(r, adj_tgt_s);

    // mark a cross in the circle ui around your player,
    // so you know where you're shooting
    /*
    {
      constexpr float radius = 40;
      constexpr float radius_squared = radius * radius;
      const float dx = adj_tgt_pos.x - par_pos.x;
      const float dy = adj_tgt_pos.y - par_pos.y;
      const float d2 = dx * dx + dy * dy;
      glm::vec2 clamped_tgt_pos = adj_tgt_pos;
      if (d2 > radius_squared) {
        const auto raw_dir = glm::vec2{ adj_tgt_pos - par_pos };
        const auto nrm_dir = engine::normalize_safe(raw_dir);
        engine::Ray ray;
        ray.origin = { par_pos.x, par_pos.y, 0 };
        ray.dir = { nrm_dir.x, nrm_dir.y, 0.0 };
        clamped_tgt_pos = engine::ray_at(ray, radius);
      }

      // debug updated target position
      Sprite adj_tgt_pos_s;
      adj_tgt_pos_s.pos = clamped_tgt_pos;
      adj_tgt_pos_s.sprite = "EFFECT_30_11";
      adj_tgt_pos_s.size = { 8, 8 };
      adj_tgt_pos_s.col = parent_col;
      draw_sprite(r, adj_tgt_pos_s);
    }
    */

// DEBUG: draw shoot line
#if defined(_DEBUG)
    // auto draw_line = [&r](const LineInfo& l) {
    //   Sprite s;
    //   s.sprite = "EMPTY";
    //   s.pos = l.position;
    //   s.size = l.scale;
    //   s.z_rotation = l.rotation;
    //   s.col = engine::SRGBColour(255, 0, 0, 255);
    //   draw_sprite(r, s);
    // };
    // draw_line(generate_line(you_pos, you_pos + 100.0f * limited_dir, 4.0f));
#endif

    const WeaponDef wep_def = get_weapon_def(r, p, wep_e);
    const BulletDef bul_def = get_bullet_def(r, p, wep_e);

    // update firerate with modified value.
    // The base value is kept in the WeaponFirerate,
    // the modified value is stored in the CooldownComponent
    cooldown_c.time_max = wep_def.firerate;

    if (cooldown_c.time > 0.0f)
      continue;
    reset_cooldown(cooldown_c);

    // rotate the gun
    const auto dir_to_enemy = smarter_tgt_pos - wep_pos;
    const float shoot_angle = engine::dir_to_angle_radians(dir_to_enemy);
    wep_t.rotation_radians.z = shoot_angle;

    // Spawn X amount of bullets
    // Note: even though the angle that the weapon can fire at is limited (e.g. 30 degrees)
    // If the weapon has enough weapon spread (e.g. 90 degrees)
    // It could still shoot at the limited angles.

    const auto angles_rad = generate_angles(shoot_angle, wep_def.projectiles, wep_def.spread_deg * engine::Deg2Rad);
    for (int i = 0; i < wep_def.projectiles; i++) {
      auto bullet_e = spawn_projectile(r, bul_def, wep_pos);
      auto& body_c = r.get<PhysicsBodyComponent>(bullet_e);

      const auto bullet_dir = engine::angle_radians_to_direction(angles_rad[i]);
      const b2Vec2 bullet_vel = bul_def.speed * b2Vec2{ bullet_dir.x, bullet_dir.y };
      body_c.body->SetLinearVelocity(bullet_vel);
    }

    // Some traits fire on nth shots
    // Every time a weapon fires, send a shoot event.
    ShootEvent shoot_evt;
    shoot_evt.parent_e = p;
    shoot_evt.weapon_e = wep_e;
    evts_c.dispatcher->trigger(shoot_evt);
    evts_c.dispatcher->update();
  }
}

} // namespace game2d