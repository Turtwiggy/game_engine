#include "enemy_projectile_system.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/maths/maths.hpp"
#include "engine/physics/components.hpp"
#include "engine/sprites/helpers.hpp"
#include "modules/actor_enemy/components.hpp"
#include "modules/combat/components.hpp"
#include "modules/event_coll_bullet_enemy/event_coll_bullet_enemy_components.hpp"
#include "modules/raws/raws_components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/helpers.hpp"
#include "modules/system_autofire/autofire_components.hpp"
#include "modules/system_cooldown/components.hpp"
#include "modules/system_cooldown/helpers.hpp"
#include "modules/system_physics_apply_force/components.hpp"
#include "modules/ui_colours/ui_colours_helpers.hpp"

namespace game2d {

void
update_enemy_projectile_system(entt::registry& r)
{
  const auto& view =
    r.view<const ApplyForceToDynamicTarget, const PhysicsDynamicTarget, const ProjectileEnemyComponent, CooldownComponent>();
  for (const auto& [e, force_c, target_c, type_c, cooldown_c] : view.each()) {
    if (cooldown_c.time > 0.0f)
      continue;
    reset_cooldown(cooldown_c);

    auto target_e = target_c.target;
    if (target_e == entt::null || !r.valid(target_e))
      continue; // your target died...

    //
    // BULLETS: MAKE EM LARGE AND SLOW
    //

    const auto raw_dir = get_position(r, target_e) - get_position(r, e);
    const auto nrm_dir = engine::normalize_safe(raw_dir);

    int bullet_damage = r.get<BulletDamage>(e).dmg;

    auto bullet_e = spawn(r, "bullet_default");
    give_life(r, bullet_e, get_position(r, e), { 36, 36 });
    r.emplace<TeamComponent>(bullet_e, AvailableTeams::enemy);
    r.get<PhysicsBodyComponent>(bullet_e).base_speed = 50.0f;
    r.emplace<EntityTimedLifecycle>(bullet_e, 10 * 1000);
    set_z_index(r, bullet_e, ZLayer::PROJECTILE);
    set_colour(r, bullet_e, hex_to_srgb("#00c420"));
    // set_sprite(r, bullet_e, "FIREWORK");

    // set velocity
    auto& body_c = r.get<PhysicsBodyComponent>(bullet_e);
    body_c.body->SetLinearVelocity({ body_c.base_speed * nrm_dir.x, body_c.base_speed * nrm_dir.y });

    r.emplace<BulletComponent>(bullet_e, bullet_damage);
    r.emplace<SetTransformRotationBasedOnPhysicsVelocity>(bullet_e);
  }
}

} // namespace game2d