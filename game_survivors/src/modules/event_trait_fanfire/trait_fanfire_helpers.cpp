#include "trait_fanfire_helpers.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/maths/maths.hpp"
#include "engine/physics/physics_components.hpp"
#include "modules/colour/components.hpp"
#include "modules/combat_projectiles/projectile_helpers.hpp"
#include "modules/system_autofire/autofire_helpers.hpp"
#include "modules/system_traits/trait_components.hpp"
#include "modules/system_traits/trait_helpers.hpp"

#include <SDL2/SDL_log.h>

namespace game2d {

struct FanfireTraitComponent
{
  int shots_until_fanfire = 20;
  int shots_until_fanfire_left = 20;

  int projectiles_to_fanfire = 10;
};

void
handle_shoot_event__trait_fanfire(entt::registry& r, const ShootEvent& evt)
{
  const auto from_e = evt.parent_e;
  const auto wep_e = evt.weapon_e;

  if (from_e == entt::null)
    return;
  if (wep_e == entt::null)
    return;

  const auto* trait_c = r.try_get<TraitComponent>(from_e);
  if (!trait_c)
    return;

  auto trait = AquirableTrait::FAN_FIRE;
  if (!has_trait(r, trait_c->traits, trait))
    return;

  // SDL_Log("one of player's gun shot");

  auto& fanfire_c = r.get_or_emplace<FanfireTraitComponent>(from_e);
  fanfire_c.shots_until_fanfire_left -= 1;

  // Fanfire limit: number of shots
  if (fanfire_c.shots_until_fanfire_left > 0)
    return;
  fanfire_c.shots_until_fanfire_left = fanfire_c.shots_until_fanfire;

  // do the fanfire
  // Note: count the bullet as one of the player's bullets.
  BulletDef bul_def = get_bullet_def(r, from_e, wep_e);
  bul_def.damage *= 0.2;   // deal 20% damage
  bul_def.size = { 4, 4 }; // fanfire bullets slightly smaller

  const auto& player_col = r.get<DefaultColour>(from_e).colour;
  const auto angles_rad = generate_angles(0, fanfire_c.projectiles_to_fanfire, engine::TWO_PI);
  for (const auto& a : angles_rad) {
    auto pos = get_position(r, from_e);

    const auto bullet_e = spawn_projectile(r, bul_def, pos);
    auto& body_c = r.get<PhysicsBodyComponent>(bullet_e);
    const auto bullet_dir = engine::angle_radians_to_direction(a);
    const auto bullet_vel = bul_def.speed * b2Vec2{ bullet_dir.x, bullet_dir.y };
    body_c.body->SetLinearVelocity(bullet_vel);

    // fanfire bullets to player col
    set_colour(r, bullet_e, player_col);
  }
}

} // namespace game2d