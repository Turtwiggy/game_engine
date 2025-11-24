#include "pch.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/maths/maths.hpp"
#include "engine/physics/physics_components.hpp"
#include "engine/physics/physics_helpers.hpp"
#include "engine/sprites/components.hpp"
#include "engine/std/vector/helpers.hpp"
#include "modules/combat/combat_core/components.hpp"
#include "modules/combat/combat_flamethrower/flamethrower_components.hpp"
#include "modules/combat/combat_projectiles/projectile_helpers.hpp"
#include "modules/core/colour/components.hpp"
#include "modules/systems/system_autofire/autofire_components.hpp"
#include "modules/systems/system_weapon_upgrade/weapon_upgrade_components.hpp"
#include "trait_shoot_fireball.hpp"
#include "trait_shoot_fireball_components.hpp"

namespace game2d {

void
handle_shoot_event__shoot_fireball(entt::registry& r, const ShootEvent& evt)
{
  const auto wep_e = evt.weapon_e;

  const auto* trait_c = r.try_get<WeaponBehaviourComponent>(wep_e);
  if (!trait_c)
    return;

  const auto behaviour = WeaponBehaviour::SMG_FIREBALL_UPGRADE;
  if (!has(trait_c->behaviours, behaviour))
    return;

  auto& fireball_c = r.get_or_emplace<TraitFireballComponent>(wep_e);
  fireball_c.shots_until_fireball_cur -= 1;

  // activation not reached.
  if (fireball_c.shots_until_fireball_cur > 0)
    return;
  fireball_c.shots_until_fireball_cur = fireball_c.shots_until_fireball_max;
  SDL_Log("Shooting fireball.");

  // define a fireball.
  const BulletDef fireball_def{
    .key = "bullet_default",
    .parent_e = wep_e,
    .size = { 20, 20 },
    .team = AvailableTeams::player,
    .damage = 10,
    .pierce = 10,
    .speed = 0.25f, // m/s
    .bounces = 0,
    .crit_chance = 0,
    .lifesteal = 0,
    .damage_type = WEAPON_DAMAGE::FIRE,
  };

  // shoot a fireball!
  const auto pos = get_position(r, wep_e);

  // Get the velocity of your parent
  auto b2_vel = b2Vec2_zero;
  if (auto* has_parent_c = r.try_get<HasParentComponent>(wep_e)) {
    const auto parent_e = has_parent_c->parent;
    if (r.valid(parent_e))
      b2_vel = b2Body_GetLinearVelocity(r.get<PhysicsBodyComponent>(parent_e).bodyId);
  }
  auto dir = glm::vec2{ b2_vel.x, b2_vel.y };

  // if you have a target, shoot that
  if (auto* autofire_c = r.try_get<AutofireComponent>(wep_e)) {
    if (!r.valid(autofire_c->target) || autofire_c->target == entt::null) {
      autofire_c->target = entt::null;
    } else
      dir = engine::normalize_safe(get_position(r, autofire_c->target) - get_position(r, wep_e));
  }

  const auto bullet_e = spawn_projectile(r, fireball_def, pos);
  const auto bullet_vel = fireball_def.speed * b2Vec2{ dir.x, dir.y };
  b2Body_SetLinearVelocity(r.get<PhysicsBodyComponent>(bullet_e).bodyId, bullet_vel);

  // make it look like a fireball
  r.remove<SpriteComponent>(bullet_e);
  r.emplace<FlamethrowerFlameComponent>(bullet_e);
  auto fixture_e = get_fixture_by_tag(r, bullet_e, "fixture_bullet");
  r.emplace<FlamethrowerFlameFixtureComponent>(fixture_e);

  // fireballs to player col
  // const auto& player_col = r.get<DefaultColour>(from_e).colour;
  // set_colour(r, bullet_e, player_col);
  // set_colour(r, bullet_e, { 255, 0, 0, 255 });
}

} // namespace game2d