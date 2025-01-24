#include "autofire_system.hpp"

#include "autofire_components.hpp"
#include "engine/actors/actor_helpers.hpp"
#include "engine/colour/colour.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/imgui/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/maths/maths.hpp"
#include "engine/physics/components.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/colour/components.hpp"
#include "modules/combat/components.hpp"
#include "modules/combat_gun_follow_player/gun_follow_player_components.hpp"
#include "modules/combat_projectiles/projectile_helpers.hpp"
#include "modules/event_coll_bullet_enemy/event_coll_bullet_enemy_components.hpp"
#include "modules/sprites/sprite_helpers.hpp"
#include "modules/system_cooldown/components.hpp"
#include "modules/system_cooldown/helpers.hpp"
#include "modules/system_hulls/hulls_components.hpp"
#include "modules/system_traits/trait_components.hpp"
#include "modules/system_upgrade/upgrade_components.hpp"

#include <box2d/b2_collision.h>

namespace game2d {

class NearestEnemyCallback : public b2QueryCallback
{
public:
  entt::registry& r;
  float nearestDistanceSquared = std::numeric_limits<float>::max();
  b2Vec2 position;

  b2Body* nearestEnemy = nullptr;
  entt::entity nearest_e = entt::null;

  NearestEnemyCallback(entt::registry& r_ref, const b2Vec2& pos)
    : r(r_ref)
    , position(pos)
  {
  }

  bool ReportFixture(b2Fixture* fixture) override
  {
    b2Body* body = fixture->GetBody();

    if (!is_enemy(body))
      return true;

    // Calculate the distance squared (avoiding sqrt for performance)
    b2Vec2 diff = body->GetPosition() - position;
    float distanceSquared = diff.LengthSquared();

    if (distanceSquared < nearestDistanceSquared) {
      nearestDistanceSquared = distanceSquared;
      nearestEnemy = body;
      nearest_e = (entt::entity)body->GetUserData().pointer;
    }

    return true; // Continue the query
  }

  // Example placeholder for identifying enemies
  bool is_enemy(b2Body* body)
  {
    const entt::entity e = (entt::entity)body->GetUserData().pointer;
    if (auto* team_c = r.try_get<TeamComponent>(e))
      return team_c->team == AvailableTeams::enemy;
    return false;
  }
};

float
cross(glm::vec2 a, glm::vec2 b)
{
  return a.x * b.y - b.x * a.y;
};

// puts an angle in the range [0, 2π]
float
clamp_axis(float angle)
{
  // range: [-2PI, 2PI]
  angle = std::fmod(angle, engine::TWO_PI);
  // range: [0, 2PI]
  if (angle < 0.0f)
    angle += engine::TWO_PI;
  return angle;
};

// puts an angle in the range [-π, π]
float
normalize_axis(float angle)
{
  angle = std::fmod(angle, engine::TWO_PI);
  if (angle > engine::PI)
    angle -= engine::TWO_PI;
  else if (angle < -engine::PI)
    angle += engine::TWO_PI;
  return angle;
};

float
clamp_angle(float rad_a, float rad_min, float rad_max)
{
  const float max_delta = clamp_axis(rad_max - rad_min) * 0.5;      // 0..π
  const float midpoint = clamp_axis(rad_min + max_delta);           // 0..2π
  const float delta_from_center = normalize_axis(rad_a - midpoint); // -π..π

  if (delta_from_center > max_delta)
    return normalize_axis(midpoint + max_delta);

  if (delta_from_center < -max_delta)
    return normalize_axis(midpoint - max_delta);

  return normalize_axis(rad_a);
};

void
update_autofire_system(entt::registry& r, glm::vec2 mouse_pos)
{
  GET_FIRST_OR_RETURN(SINGLE_Physics, r, phys_e, phys_c);

  auto& dead = get_first_component<SINGLE_EntityBinComponent>(r);

  const float search_radius = 500.0f; // for nearest enemy

  static float lead_amount = 0.4f;
#if defined(_DEBUG)
  imgui_draw_float("shot lead amount", lead_amount);
#endif

  const auto& view = r.view<TransformComponent,
                            const WeaponComponent,
                            const HasParentComponent,
                            const HardpointComponent,
                            const AutofireComponent,
                            CooldownComponent>();

  for (const auto& [wep_e, wep_t, wep_c, parent_c, arc_c, autofire_c, cooldown_c] : view.each()) {

    const auto p = parent_c.parent;
    if (p == entt::null || !r.valid(p)) {
      dead.dead.emplace(wep_e); // kill this parentless entity (soz)
      continue;
    }

    // if (cooldown_c.time > 0.0f)
    //   continue;
    // reset_cooldown(cooldown_c);

    const auto& parent_t = r.get<TransformComponent>(p);
    const auto& parent_col = r.get<DefaultColour>(p).colour;

    // get closest enemy
    NearestEnemyCallback callback(r, b2Vec2{ parent_t.position.x, parent_t.position.y });
    b2AABB aabb;
    aabb.lowerBound = b2Vec2{ parent_t.position.x, parent_t.position.y } - b2Vec2{ search_radius, search_radius };
    aabb.upperBound = b2Vec2{ parent_t.position.x, parent_t.position.y } + b2Vec2{ search_radius, search_radius };
    phys_c.world->QueryAABB(&callback, aabb);
    auto nearest_e = callback.nearest_e;
    if (nearest_e == entt::null)
      continue;

    const auto you_pos = glm::vec2{ parent_t.position.x, parent_t.position.y };
    const auto tgt_pos = get_position(r, nearest_e);

    // Note: Adjust the angle, so that the auto-fire leads it's shot a little
    const auto tgt_vel = r.get<PhysicsBodyComponent>(nearest_e).body->GetLinearVelocity();
    const auto adj_tgt_pos = tgt_pos + glm::vec2{ tgt_vel.x * lead_amount, tgt_vel.y * lead_amount };

    // debug the nearest enemy
    // Sprite tgt_s;
    // tgt_s.pos = tgt_pos;
    // tgt_s.sprite = "CROSSHAIR_1";
    // tgt_s.size = { 16, 16 };
    // tgt_s.col = parent_col;
    // draw_sprite(r, tgt_s);

    // debug updated target position
    Sprite adj_tgt_pos_s;
    adj_tgt_pos_s.pos = adj_tgt_pos;
    adj_tgt_pos_s.sprite = "EFFECT_30_11";
    adj_tgt_pos_s.size = { 8, 8 };
    adj_tgt_pos_s.col = parent_col;
    draw_sprite(r, adj_tgt_pos_s);

    const auto raw_dir = adj_tgt_pos - you_pos;
    const auto nrm_dir = engine::normalize_safe(raw_dir);
    const auto angle = engine::dir_to_angle_radians(nrm_dir);
    const auto fwd = parent_t.rotation_radians.z;
    const auto fwd_dir = engine::angle_radians_to_direction(fwd);

    // GAMEPLAY: Clamp the dir between the hardpoint's calculated min_arc and max_arc
    const auto left = arc_c.dir_arc_left;
    const auto right = arc_c.dir_arc_right;
    const auto min = engine::dir_to_angle_radians(left);
    const auto max = engine::dir_to_angle_radians(right);
    const auto limited_angle = clamp_angle(angle, min, max);
    const auto limited_dir = engine::angle_radians_to_direction(limited_angle);

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

    int bullet_damage = r.get<BulletDamage>(parent_c.parent).dmg;
    int bullet_speed = 250;
    auto& upgrades_c = r.get<StatModifierComponent>(p);
    const float modified_speed = upgrades_c.apply_modifiers(bullet_speed, bullet_speed_key);
    const float modified_damage = upgrades_c.apply_modifiers(bullet_damage, bullet_damage_key);

    if (cooldown_c.time > 0.0f)
      continue;
    reset_cooldown(cooldown_c);

    BulletDef bullet_def;
    bullet_def.key = "bullet_default";
    bullet_def.parent_e = wep_e;
    bullet_def.size = { 6, 6 };
    bullet_def.team = AvailableTeams::player;
    bullet_def.damage = (int)modified_damage;
    bullet_def.speed = modified_speed;
    bullet_def.lifecycle = 3 * 1000;
    bullet_def.traits = r.get<TraitComponent>(p).traits; // traits from wep's parent, not wep
    auto bullet_e = spawn_projectile(r, bullet_def);

    // set velocity
    auto& body_c = r.get<PhysicsBodyComponent>(bullet_e);
    body_c.body->SetLinearVelocity(b2Vec2{ body_c.base_speed * limited_dir.x, body_c.base_speed * limited_dir.y });

    // rotate the gun
    const float shoot_angle = engine::dir_to_angle_radians(limited_dir);
    wep_t.rotation_radians.z = shoot_angle;
  }
}

} // namespace game2d