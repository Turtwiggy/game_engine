#include "pch.hpp"

#include "autofire_components.hpp"
#include "autofire_system.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/colour/colour.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/maths/maths.hpp"
#include "engine/physics/physics_components.hpp"
#include "engine/physics/physics_helpers.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/actors/actor_enemy/components.hpp"
#include "modules/actors/actor_islanddweller/islanddweller_components.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/combat/combat_gun_follow_player/gun_follow_player_components.hpp"
#include "modules/combat/combat_projectiles/projectile_components.hpp"
#include "modules/combat/combat_weapon_core/combat_weapon_core_components.hpp"
#include "modules/core/camera/orthographic.hpp"
#include "modules/core/colour/components.hpp"
#include "modules/core/sprites/sprite_helpers.hpp"
#include "modules/events/event_shoot/event_shoot_components.hpp"
#include "modules/events/events_core/events_components.hpp"
#include "modules/systems/system_hardpoint_arcs/hulls_components.hpp"
#include "resources/data.hpp"

namespace game2d {

void
draw_crosshair(entt::registry& r, const glm::vec2 pos, const glm::vec2 dir, const engine::SRGBColour& col)
{
  const auto camera_e = get_first<OrthographicCamera>(r);
  const auto& camera_c = r.get<OrthographicCamera>(camera_e);
  const auto zoom = camera_c.zoom_nonlinear;
  const float radius = (50 + 2) / zoom;
  engine::Ray ray;
  ray.origin = { pos.x, pos.y, 0.0 };
  ray.dir = { dir.x, dir.y, 0.0 };
  const auto crosshair_pos = engine::ray_at(ray, radius);

  Sprite adj_tgt_s;
  adj_tgt_s.pos = crosshair_pos;
  adj_tgt_s.sprite = "CROSSHAIR_2";
  adj_tgt_s.size = { 16, 16 };
  adj_tgt_s.col = col;
  draw_sprite(r, adj_tgt_s);
};

void
filter_enemies_by_shoot_angle(entt::registry& r,
                              std::vector<std::pair<int, entt::entity>>& enemies,
                              const HardpointComponent& hardpoint_c,
                              const glm::vec2 wep_pos)
{
  std::vector<std::pair<int, entt::entity>> valid_targets;

  for (const auto& [d2, body_e] : enemies) {
    const auto enemy_pos = get_position(r, body_e);

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
      valid_targets.push_back({ d2, body_e });
    } else if (angle >= min && angle <= max)
      valid_targets.push_back({ d2, body_e });
  }

  enemies = valid_targets;
};

entt::entity
get_nearest_target(entt::registry& r, const entt::entity wep_e, const TransformComponent& wep_t, const WeaponDef& wep_def)
{
  // Get enemies in your weapon range
  const auto wep_pos = glm::vec2{ wep_t.position.x, wep_t.position.y };
  const float search_radius_meters = wep_def.range; // for nearest enemy

  const std::function<bool(entt::registry&, entt::entity)> is_enemy = [&](entt::registry& r, entt::entity parent_e) -> bool {
    bool is_enemy = r.try_get<EnemyComponent>(parent_e) != nullptr;

    // Filter enemies in radius so it's a circle shape not a box shape.
    const auto enemy_pos_in_meters = pixels_to_meters(get_position(r, parent_e));
    const auto wep_pos_in_meters = pixels_to_meters(get_position(r, wep_e));

    // adjust to include enemy radius as in-range - not just center.
    const auto enemy_size = pixels_to_meters(get_size(r, parent_e));
    const float enemy_radius_meters = glm::max(enemy_size.x, enemy_size.y) * 0.5f;

    // check for circle col...
    const bool coll = engine::circle_collision(
      engine::Circle{ .pos = { wep_pos_in_meters.x, wep_pos_in_meters.y }, .radius = search_radius_meters },
      engine::Circle{ .pos = { enemy_pos_in_meters.x, enemy_pos_in_meters.y }, .radius = enemy_radius_meters });

#if defined(_DEBUG)
    if (is_enemy) {
      // Sprite s;
      // s.sprite = "EMPTY";
      // s.pos = meters_to_pixels(enemy_pos_in_meters);
      // s.size = { 8, 8 };
      // if (coll)
      //   s.col = engine::SRGBColour(0.0f, 1.0f, 0.0f, 1.0f);
      // else
      //   s.col = engine::SRGBColour(1.0f, 0.0f, 0.0f, 1.0f);
      // draw_sprite(r, s);
    }
#endif

    return is_enemy && coll;
  };
  const b2Vec2 center_m = pixels_to_meters(wep_pos);
  auto enemies_map = get_all_in_area_filtered(r, center_m, search_radius_meters, is_enemy);
  if (enemies_map.empty())
    return entt::null;

  std::vector<std::pair<int, entt::entity>> enemies;
  for (const auto& [parent_e, coll_fixtures] : enemies_map) {
    for (const auto& fixture_coll_result : coll_fixtures) {
      const auto fixture_e = fixture_coll_result.fixture_e;

      const bool has_hp = r.try_get<HealthComponent>(fixture_e);
      if (!has_hp)
        continue; // shield or xp zone or something without health

      enemies.push_back({ fixture_coll_result.d2, parent_e });
      break; // you hit an enemy fixture with health; damage the enemy once.
    }
  }

  // Filter by angle that this weapon can shoot
  if (auto* hardpoint_c = r.try_get<HardpointComponent>(wep_e))
    filter_enemies_by_shoot_angle(r, enemies, *hardpoint_c, wep_pos);

  // Check if enemies after all filter conditions
  if (enemies.empty())
    return entt::null;

  // Get the nearest enemy
  // note: a good modifier would be to get the enemy with the highest hp
  auto sort_by_distance = [](const auto& a, const auto& b) { return a.first < b.first; };
  std::sort(enemies.begin(), enemies.end(), sort_by_distance);
  auto nearest_e = enemies[0].second;
  return nearest_e;
};

glm::vec2
calculate_aim_dir(const glm::vec2 a_pos,
                  const glm::vec2 a_vel,
                  const glm::vec2 b_pos,
                  const glm::vec2 b_vel,
                  const float bul_speed)
{
  const auto rel_pos = b_pos - a_pos;
  const auto rel_vel = b_vel - a_vel;

  const float a = glm::dot(rel_vel, rel_vel) - bul_speed * bul_speed;
  const float b = 2.0f * glm::dot(rel_pos, rel_vel);
  const float c = glm::dot(rel_pos, rel_pos);
  const float discriminant = b * b - 4 * a * c;

  // no intercept possible, just aim directly at the target.
  if (discriminant <= 0)
    return engine::normalize_safe(rel_pos);

  const float sqrt_discriminant = (float)glm::sqrt(discriminant);
  const float t1 = (-b + sqrt_discriminant) / (2.0f * a);
  const float t2 = (-b + sqrt_discriminant) / (2.0f * a);
  const float t = glm::max(t1, t2); // earliest positive time

  // no intercept time.
  if (t <= 0)
    return engine::normalize_safe(rel_pos);

  // aim at the intercept point
  const auto intercept_point = b_pos + b_vel * t;
  return engine::normalize_safe(intercept_point - a_pos);
};

void
aim_in_movement_direction(entt::registry& r, const b2Vec2 par_vel, const entt::entity wep_e)
{
  auto& wep_t = r.get<TransformComponent>(wep_e);
  wep_t.rotation_radians.z = engine::dir_to_angle_radians({ par_vel.x, par_vel.y });
};

bool
update_aquire_target(entt::registry& r,
                     AutofireComponent& autofire_c,
                     const entt::entity wep_e,
                     const TransformComponent& wep_t,
                     const WeaponDef& wep_def,
                     const float dt)
{
  // stop spamming costly get_nearest_target by checking for targets every ~1s
  if (autofire_c.target_aquisition_cooldown_cur > 0.0f) {
    autofire_c.target_aquisition_cooldown_cur -= dt;
    return false;
  }
  autofire_c.target_aquisition_cooldown_cur = autofire_c.target_aquisition_cooldown_max;

  auto nearest_e = get_nearest_target(r, wep_e, wep_t, wep_def);
  if (nearest_e == entt::null)
    return false;

  autofire_c.target = nearest_e;
  return true;
}

void
update_autofire_system(entt::registry& r, const float dt)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif
  GET_FIRST_OR_RETURN(SINGLE_Physics, r, phys_e, phys_c);
  auto& evts_c = SINGLE_Events::instance;
  auto& dead = get_first_component<SINGLE_EntityBinComponent>(r);

  // weapons with a parent component.
  {
    const auto view = r.view<const WeaponComponent,
                             const WeaponDef,
                             const WeaponRange,
                             const HasParentComponent,
                             TransformComponent,
                             AutofireComponent>();
    for (const auto& [wep_e, weapon_c, wep_def, wep_range_c, par_c, wep_t, autofire_c] : view.each()) {

      if (!r.valid(par_c.parent))
        continue;

      const auto par_e = par_c.parent;
      const auto& par_inp = r.get<const InputComponent>(par_e);
      const auto& par_t = r.get<const TransformComponent>(par_e);
      const auto& par_col = r.get<const DefaultColour>(par_e).colour;
      const auto* par_pb = r.try_get<PhysicsBodyComponent>(par_e);
      const auto par_vel_m = b2Body_GetLinearVelocity(par_pb->bodyId);

      const auto wep_pos = glm::vec2{ wep_t.position.x, wep_t.position.y };
      auto dir_to_enemy = glm::vec2();

      // If the player is holding the right analogue, overwrite the shoot_angle.
      const float deadzone = 0.05f;
      auto override_autofire = false;
      if (glm::abs(par_inp.rx) > deadzone || glm::abs(par_inp.ry) > deadzone) {
        override_autofire = true;
        autofire_c.target = entt::null;
        dir_to_enemy = { par_inp.rx, par_inp.ry };
        draw_crosshair(r, wep_pos, dir_to_enemy, par_col);

        // rotate the gun to the target
        wep_t.rotation_radians.z = engine::dir_to_angle_radians(dir_to_enemy);
        continue;
      }

      // get a target
      if (autofire_c.target == entt::null || !r.valid(autofire_c.target)) {
        aim_in_movement_direction(r, par_vel_m, wep_e);
        if (!update_aquire_target(r, autofire_c, wep_e, wep_t, wep_def, dt))
          continue; // target aquire cd
      }

      // check your target is still within distance
      // (optional) theres a line of sight between you and it
      const auto d = wep_pos - get_position(r, autofire_c.target);
      const auto d2 = d.x * d.x + d.y * d.y;
      const auto d2_threshold = pow(meters_to_pixels(wep_range_c.meters), 2);
      if (d2 > d2_threshold)
        autofire_c.target = entt::null;
      if (autofire_c.target == entt::null) {
        aim_in_movement_direction(r, par_vel_m, wep_e);
        continue;
      }

      const auto tgt = autofire_c.target;
      const auto tgt_pos = get_position(r, tgt);
      const auto you_pos = wep_pos;
      auto aim_dir = tgt_pos - you_pos;

      // if you're shooting projectiles, aim at the intercept point
      if (auto* bul_def = r.try_get<BulletDef>(wep_e)) {
        const auto bullet_speed_p = meters_to_pixels(bul_def->speed);
        const auto tgt_vel_m = b2Body_GetLinearVelocity(r.get<const PhysicsBodyComponent>(tgt).bodyId);
        const glm::vec2 tgt_vel_p = meters_to_pixels(tgt_vel_m);
        const auto you_vel_m = par_vel_m;
        const auto you_vel_p = meters_to_pixels(you_vel_m);
        aim_dir = calculate_aim_dir(you_pos, you_vel_p, tgt_pos, tgt_vel_p, bullet_speed_p);
      }

      dir_to_enemy = engine::normalize_safe(aim_dir);
      draw_crosshair(r, wep_pos, dir_to_enemy, par_col);

      // rotate the gun to the target
      wep_t.rotation_radians.z = engine::dir_to_angle_radians(dir_to_enemy);
    }
  }

  // weapons without a parent component.
  {
    const auto view =
      r.view<const WeaponComponent, const WeaponDef, const WeaponRange, TransformComponent, AutofireComponent>(
        entt::exclude<HasParentComponent>);
    for (const auto& [wep_e, weapon_c, wep_def, wep_range_c, wep_t, autofire_c] : view.each()) {

      const auto wep_pos = glm::vec2{ wep_t.position.x, wep_t.position.y };
      auto dir_to_enemy = glm::vec2();

      // get a target
      if (autofire_c.target == entt::null || !r.valid(autofire_c.target)) {
        if (!update_aquire_target(r, autofire_c, wep_e, wep_t, wep_def, dt))
          continue; // target aquire cd
      }

      // check your target is still within distance
      // (optional) theres a line of sight between you and it
      const auto d = wep_pos - get_position(r, autofire_c.target);
      const auto d2 = d.x * d.x + d.y * d.y;
      const auto d2_threshold = pow(meters_to_pixels(wep_range_c.meters), 2);
      if (d2 > d2_threshold)
        autofire_c.target = entt::null;
      if (autofire_c.target == entt::null)
        continue;

      const auto tgt = autofire_c.target;
      const auto tgt_pos = get_position(r, tgt);
      const auto you_pos = wep_pos;
      auto aim_dir = tgt_pos - you_pos;

      // if you're shooting projectiles, aim at the intercept point
      if (auto* bul_def = r.try_get<BulletDef>(wep_e)) {
        const auto bullet_speed_p = meters_to_pixels(bul_def->speed);
        const auto tgt_vel_m = b2Body_GetLinearVelocity(r.get<const PhysicsBodyComponent>(tgt).bodyId);
        const glm::vec2 tgt_vel_p = meters_to_pixels(tgt_vel_m);
        const auto you_vel_m = b2Vec2_zero;
        const auto you_vel_p = meters_to_pixels(you_vel_m);
        aim_dir = calculate_aim_dir(you_pos, you_vel_p, tgt_pos, tgt_vel_p, bullet_speed_p);
      }

      dir_to_enemy = engine::normalize_safe(aim_dir);
      draw_crosshair(r, wep_pos, dir_to_enemy, my_greenish);

      // rotate the gun to the target
      wep_t.rotation_radians.z = engine::dir_to_angle_radians(dir_to_enemy);
    }
  }

  // handle sending ShootEvent
  {
    const auto view = r.view<const WeaponDef, WeaponFireRate, WeaponReloadRate, WeaponClipSize>();
    for (const auto& [wep_e, wep_def, weapon_fire_rate_c, weapon_reload_rate_c, weapon_clip_size_c] : view.each()) {

      // parent has dropped anchor, stop firing.
      if (const auto* par_c = r.try_get<HasParentComponent>(wep_e)) {
        const auto par_e = par_c->parent;
        if (r.all_of<DroppedAnchorComponent>(par_e))
          continue;
      }

      // you gotta reload
      if (weapon_reload_rate_c.seconds_cur > 0.0) {
        weapon_reload_rate_c.seconds_cur -= dt;
        continue;
      }

      // you've reloaded
      if (weapon_clip_size_c.bullets_cur <= 0) {
        weapon_clip_size_c.bullets_cur = wep_def.bullets_max;
        weapon_fire_rate_c.seconds_between_shots_left = 0.0f;
      }

      // Check if you're fire-rate limited.
      // note: updates the _max time based on the modded firerate
      weapon_fire_rate_c.seconds_between_shots_max = 1.0f / wep_def.fire_rate;
      if (weapon_fire_rate_c.seconds_between_shots_left >= 0.0) {
        weapon_fire_rate_c.seconds_between_shots_left -= dt;
        continue;
      }

      // Check the clip size before firing.
      if (weapon_clip_size_c.bullets_cur <= 0) { // time to reload
        weapon_reload_rate_c.seconds_cur = wep_def.reload_rate;
        continue;
      }

      // Shoot a bullet! (in this case, the bullet is a turret)
      weapon_clip_size_c.bullets_cur--;
      weapon_fire_rate_c.seconds_between_shots_left = weapon_fire_rate_c.seconds_between_shots_max;

      // do the shoot event
      ShootEvent shoot_evt;
      shoot_evt.weapon_e = wep_e;
      evts_c.dispatcher->trigger(shoot_evt);
      evts_c.dispatcher->update();

      // Check the clip size after firing.
      if (weapon_clip_size_c.bullets_cur <= 0)
        weapon_reload_rate_c.seconds_cur = wep_def.reload_rate;
    }
  }
}

} // namespace game2d