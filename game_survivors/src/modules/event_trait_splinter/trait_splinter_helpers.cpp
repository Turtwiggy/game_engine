#include "trait_splinter_helpers.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/maths/maths.hpp"
#include "engine/physics/physics_components.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/combat/components.hpp"
#include "modules/combat_projectiles/projectile_helpers.hpp"
#include "modules/system_autofire/autofire_helpers.hpp"
#include "modules/system_traits/trait_components.hpp"
#include "modules/system_traits/trait_helpers.hpp"

#include <SDL2/SDL_log.h>

namespace game2d {

static engine::RandomState rnd_dir(0);

void
handle_death_event__trait_splinter(entt::registry& r, const DeathEvent& evt)
{
  const auto from_e = evt.killed_by;
  const auto to_e = evt.dead;

  if (from_e == entt::null || to_e == entt::null)
    return;

  auto* trait_c = r.try_get<TraitComponent>(from_e);
  if (!trait_c)
    return;

  auto trait = AquirableTrait::SPLINTER;
  if (!has_trait(r, trait_c->traits, trait))
    return;

  const auto splinter_callback = [from_e](entt::registry& r, const entt::entity dead_e) {
    // Spawn 3 bullets in each 360/3 degrees apart, and
    // each of them deals 10% damage of the current player BulletDamage

    const auto player_bullet_e = from_e;

    const auto wep_e = r.get<HasParentComponent>(player_bullet_e).parent;
    const auto par_e = r.get<HasParentComponent>(wep_e).parent;

    if (!r.valid(par_e) || par_e == entt::null)
      return; // parently probably died

    // Note: count the bullet as one of the player's bullets.
    BulletDef bul_def = get_bullet_def(r, par_e, wep_e);
    bul_def.damage *= 0.1;   // deal 10% damage
    bul_def.size = { 4, 4 }; // splinter bullets slightly smaller

    constexpr int projectiles = 3;
    const auto random_dir = engine::rand_det_s(rnd_dir.rng, 0.0f, engine::TWO_PI);
    const auto angles_rad = generate_angles(random_dir, projectiles, engine::TWO_PI);

    for (const auto& a : angles_rad) {
      auto pos = get_position(r, dead_e);

      const auto bullet_e = spawn_projectile(r, bul_def, pos);
      auto& body_c = r.get<PhysicsBodyComponent>(bullet_e);
      const auto bullet_dir = engine::angle_radians_to_direction(a);
      const auto bullet_vel = bul_def.speed * b2Vec2{ bullet_dir.x, bullet_dir.y };
      body_c.body->SetLinearVelocity(bullet_vel);

      // colour splinter bullets differently?
      set_colour(r, bullet_e, { 0.6f, 0.9f, 0.6f, 1.0f });
    }
  };

  // auto& tag_c = r.get<TagComponent>(to_e).tag;
  // SDL_Log("%s considering splintering", tag_c.c_str());

  // Add an OnDeathCallback that spawns 3 bullets
  auto& death_c = r.get<OnDeathCallbacks>(to_e);
  death_c.callbacks.push_back(splinter_callback);
}

} // namespace game2d