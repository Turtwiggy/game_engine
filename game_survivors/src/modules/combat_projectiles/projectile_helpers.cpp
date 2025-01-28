#include "projectile_helpers.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "modules/event_coll_bullet_other/event_coll_bullet_other_components.hpp"
#include "modules/raws/raws_components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/helpers.hpp"
#include "modules/system_traits/trait_components.hpp"

namespace game2d {

entt::entity
spawn_projectile(entt::registry& r, const BulletDef& bullet_def)
{
  auto parent_e = bullet_def.parent_e;

  auto bullet_e = spawn(r, bullet_def.key);
  give_life(r, bullet_e, get_position(r, parent_e), bullet_def.size);
  r.emplace<TeamComponent>(bullet_e, bullet_def.team);
  r.emplace<EntityTimedLifecycle>(bullet_e, bullet_def.lifecycle);
  r.emplace<HasParentComponent>(bullet_e, parent_e);
  r.emplace_or_replace<TraitComponent>(bullet_e, bullet_def.traits);
  r.emplace<SetTransformRotationBasedOnPhysicsVelocity>(bullet_e);

  r.emplace<BulletComponent>(bullet_e);
  r.emplace<BulletDamage>(bullet_e, bullet_def.damage);
  r.emplace<BulletPierce>(bullet_e, bullet_def.pierce);
  r.emplace<BulletSpeed>(bullet_e, bullet_def.speed);

  set_z_index(r, bullet_e, ZLayer::PROJECTILE);

  return bullet_e;
}

} // namespace game2d