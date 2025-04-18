#include "pch.hpp"

#include "projectile_helpers.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/physics/physics_helpers.hpp"
#include "modules/actors/actor_weapon/weapon_components.hpp"
#include "modules/core/colour/components.hpp"
#include "modules/core/raws/raws_components.hpp"
#include "modules/core/renderer/components.hpp"
#include "modules/core/renderer/helpers.hpp"
#include "modules/events/event_coll_bullet_other/event_coll_bullet_other_components.hpp"
#include "modules/events/event_damage_lifesteal/lifesteal_components.hpp"
#include "modules/systems/system_weapon_upgrade/weapon_upgrade_components.hpp"

namespace game2d {

entt::entity
spawn_projectile(entt::registry& r, const BulletDef& bullet_def, glm::vec2 pos)
{
  // note: modifiers already applied, provided via BulletDef

  const auto parent_e = bullet_def.parent_e;

  const auto bullet_e = spawn(r, bullet_def.key);
  give_life(r, bullet_e, pos, bullet_def.size);
  r.emplace<HasParentComponent>(bullet_e, HasParentComponent{ parent_e });

  auto fixture_e = get_fixture_by_tag(r, bullet_e, "fixture_bullet");
  r.emplace<BulletComponent>(fixture_e);

  r.emplace<TeamComponent>(bullet_e, bullet_def.team);
  r.emplace<EntityTimedLifecycle>(bullet_e, bullet_def.lifecycle);
  r.emplace<SetTransformRotationBasedOnPhysicsVelocity>(bullet_e);

  r.emplace<BulletDamage>(bullet_e, bullet_def.damage);
  r.emplace<BulletPierce>(bullet_e, bullet_def.pierce);
  r.emplace<BulletSize>(bullet_e, bullet_def.size);
  r.emplace<BulletSpeed>(bullet_e, bullet_def.speed);
  r.emplace<BulletKnockback>(bullet_e, bullet_def.knockback_force);
  if (bullet_def.bounces > 0)
    r.emplace<BulletBounce>(bullet_e, BulletBounce{ bullet_def.bounces });
  if (bullet_def.lifesteal > 0)
    r.emplace<BulletLifesteal>(bullet_e, bullet_def.lifesteal);
  if (bullet_def.crit_chance > 0) {
    const BulletCrit crit_c = {
      .crit_chance = bullet_def.crit_chance,
      .crit_damage = bullet_def.crit_damage,
    };
    r.emplace<BulletCrit>(bullet_e, crit_c);
  }

  // copy the weapon components to the bullet
  r.emplace<WeaponDamageTypeComponent>(bullet_e, WeaponDamageTypeComponent{ bullet_def.damage_type });

  // add all weapon traits to the bullet
  auto& wb_c = r.emplace<WeaponBehaviourComponent>(bullet_e);
  wb_c.behaviours.insert(bullet_def.wep_behaviours.begin(), bullet_def.wep_behaviours.end());

  set_z_index(r, bullet_e, ZLayer::PROJECTILE);
  set_colour(r, bullet_e, r.get<DefaultColour>(parent_e).colour);

  return bullet_e;
}

} // namespace game2d