#include "projectile_helpers.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "modules/core_colour/components.hpp"
#include "modules/core_raws/raws_components.hpp"
#include "modules/core_renderer/components.hpp"
#include "modules/core_renderer/helpers.hpp"
#include "modules/event_coll_bullet_other/event_coll_bullet_other_components.hpp"
#include "modules/system_traits/trait_components.hpp"


namespace game2d {

entt::entity
spawn_projectile(entt::registry& r, const BulletDef& bullet_def, glm::vec2 pos)
{
  auto parent_e = bullet_def.parent_e;

  auto bullet_e = spawn(r, bullet_def.key);
  give_life(r, bullet_e, pos, bullet_def.size);
  r.emplace<HasParentComponent>(bullet_e, HasParentComponent{ parent_e });

  r.emplace<TeamComponent>(bullet_e, bullet_def.team);
  r.emplace<EntityTimedLifecycle>(bullet_e, bullet_def.lifecycle);
  r.emplace<SetTransformRotationBasedOnPhysicsVelocity>(bullet_e);

  r.emplace<BulletComponent>(bullet_e);
  r.emplace<BulletDamage>(bullet_e, bullet_def.damage);
  r.emplace<BulletPierce>(bullet_e, bullet_def.pierce);
  r.emplace<BulletSize>(bullet_e, bullet_def.size);
  r.emplace<BulletSpeed>(bullet_e, bullet_def.speed);
  r.emplace<BulletKnockback>(bullet_e, bullet_def.knockback_force);
  if (bullet_def.bounces > 0)
    r.emplace<BulletBounce>(bullet_e, BulletBounce{ bullet_def.bounces });

  auto& bullet_trait_c = r.get<TraitComponent>(bullet_e);
  bullet_trait_c.traits.insert(bullet_def.traits.begin(), bullet_def.traits.end());

  set_z_index(r, bullet_e, ZLayer::PROJECTILE);
  set_colour(r, bullet_e, r.get<DefaultColour>(parent_e).colour);

  return bullet_e;
}

} // namespace game2d