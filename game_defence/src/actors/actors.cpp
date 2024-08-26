#include "actors/actors.hpp"

#include "base.hpp"
#include "entt/helpers.hpp"
#include "lifecycle/components.hpp"
#include "maths/maths.hpp"
#include "modules/actor_enemy/components.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/actor_weapon_shotgun/components.hpp"
#include "modules/actors/helpers.hpp"
#include "modules/algorithm_astar_pathfinding/components.hpp"
#include "modules/combat_attack_cooldown/components.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/combat_wants_to_shoot/components.hpp"
#include "modules/lighting/components.hpp"
#include "modules/system_move_to_target_via_lerp/components.hpp"
#include "modules/system_particles/components.hpp"
#include "modules/system_turnbased/components.hpp"
#include "modules/ui_inventory/components.hpp"
#include "modules/ux_hoverable/components.hpp"
#include "physics/helpers.hpp"
#include "renderer/transform.hpp"

#include <fmt/core.h>

namespace game2d {

//
// helpers
//

entt::entity
create_transform(entt::registry& r)
{
  EntityDescription desc;
  return Factory_BaseActor::create(r, EntityType::empty_with_transform, desc);
};

void
add_particles(entt::registry& r, const entt::entity parent)
{
  const auto particle_emitter = create_transform(r);
  set_position(r, particle_emitter, get_position(r, parent));
  set_size(r, particle_emitter, { 0, 0 }); // no size just script, but need position

  r.emplace<SetPositionAtDynamicTarget>(particle_emitter);
  r.emplace<DynamicTargetComponent>(particle_emitter, parent);

  // This could be unique per add_particles but for the moment,
  // just give everything that need particles the same particle
  Particle pdesc;
  pdesc.start_size = 6;
  pdesc.end_size = 0;
  pdesc.colour = { 1.0f, 1.0f, 1.0f, 1.0f };
  pdesc.sprite = "EMPTY";

  // which particle to spawn?
  ParticleEmitterComponent pedesc;
  pedesc.particle_to_emit = pdesc;
  r.emplace<ParticleEmitterComponent>(particle_emitter, pedesc);

  // emit: particles
  AttackCooldownComponent cooldown;
  cooldown.time_between_attack = 0.1f;
  cooldown.time_between_attack_left = cooldown.time_between_attack;
  r.emplace<AttackCooldownComponent>(particle_emitter, cooldown);
};

//
// physics
//

void
create_physics_actor_static(entt::registry& r, const entt::entity e, const glm::ivec2& position, const glm::ivec2& size)
{
  PhysicsDescription desc;
  desc.type = b2_staticBody;
  desc.is_bullet = false;
  desc.density = 1.0;
  desc.position = position;
  desc.size = size;
  create_physics_actor(r, e, desc);
};

void
create_physics_actor_dynamic(entt::registry& r,
                             const entt::entity e,
                             const glm::ivec2& position,
                             const glm::ivec2& size,
                             bool is_bullet = false)
{
  PhysicsDescription desc;
  desc.type = b2_dynamicBody;
  desc.is_bullet = is_bullet;
  desc.density = 1.0;
  desc.position = position;
  desc.size = size;
  create_physics_actor(r, e, desc);
};

//
// component helpers
//

void
add_components(entt::registry& r, const entt::entity e, const ActorDungeon& desc)
{
  PhysicsDescription pdesc;
  pdesc.type = b2_kinematicBody;
  pdesc.is_bullet = false;
  pdesc.density = 1.0;
  pdesc.position = desc.pos;
  pdesc.size = desc.size;
  pdesc.is_sensor = true;
  create_physics_actor(r, e, pdesc);

  // add_components()
  r.emplace<HoveredColour>(e, desc.hovered_colour);
  r.emplace<HoverableComponent>(e);
  r.emplace<TurnBasedUnitComponent>(e);
  r.emplace<SpawnParticlesOnDeath>(e);
  r.emplace<MoveLimitComponent>(e, 1);
  r.emplace<HealthComponent>(e, desc.hp, desc.max_hp);
  r.emplace<DefenceComponent>(e, 0);     // should be determined by equipment
  r.emplace<PathfindComponent>(e, 1000); // pass through units if you must
  r.emplace<TeamComponent>(e, desc.team);

  if (desc.team == AvailableTeams::enemy)
    r.emplace<EnemyComponent>(e);
};

void
remove_components(entt::registry& r, const entt::entity e, const ActorJetpackPlayer& desc)
{
  r.remove<MovementJetpackComponent>(e);
  r.remove<DefaultColour>(e);
  r.remove<TeamComponent>(e);

  auto& physics_c = get_first_component<SINGLE_Physics>(r);
  physics_c.world->DestroyBody(r.get<PhysicsBodyComponent>(e).body);
  r.remove<PhysicsBodyComponent>(e);
}

//
// factories
//

const glm::ivec3 DEFAULT_SIZE{ 32, 32, 1 };
const glm::ivec3 HALF_SIZE{ 16, 16, 1 };
const glm::ivec2 SMALL_SIZE{ 4, 4 };

entt::entity
Factory_ActorBreachCharge::create(entt::registry& r, const ActorBreachCharge& desc)
{
  const auto type = Actor<EntityType::actor_breach_charge>::type;
  const auto e = Factory_BaseActor::create(r, type, desc);

  PhysicsDescription pdesc;
  pdesc.type = b2_dynamicBody;
  pdesc.is_bullet = false;
  pdesc.density = 1.0;
  pdesc.position = desc.pos;
  pdesc.size = { DEFAULT_SIZE.x, DEFAULT_SIZE.y };
  pdesc.is_sensor = false;
  create_physics_actor(r, e, pdesc);

  // bomb blows up in X seconds
  r.emplace<EntityTimedLifecycle>(e, 3 * 1000);

  return e;
};

entt::entity
Factory_ActorDungeon::create(entt::registry& r, const ActorDungeon& desc)
{
  const auto type = Actor<EntityType::actor_dungeon>::type;
  const auto e = Factory_BaseActor::create(r, type, desc);
  const auto size = DEFAULT_SIZE;

  ActorDungeon mdesc; // copy?
  mdesc.size = size;

  add_components(r, e, desc);

  return e;
};

entt::entity
Factory_ActorJetpackPlayer::create(entt::registry& r, const ActorJetpackPlayer& desc)
{
  const auto type = Actor<EntityType::actor_jetpack_player>::type;
  const auto e = Factory_BaseActor::create(r, type, desc);
  const auto size = DEFAULT_SIZE;

  PhysicsDescription pdesc;
  pdesc.type = b2_dynamicBody;
  pdesc.is_bullet = false;
  pdesc.density = 1.0;
  pdesc.position = desc.pos;
  pdesc.size = size;
  pdesc.is_sensor = false;
  create_physics_actor(r, e, pdesc);
  r.get<PhysicsBodyComponent>(e).base_speed = 100.0f;

  if (desc.team == AvailableTeams::player) {
    r.emplace<PlayerComponent>(e);
    r.emplace<TeamComponent>(e, AvailableTeams::player);
    r.emplace<InputComponent>(e);
    r.emplace<KeyboardComponent>(e);

    r.emplace<DefaultBody>(e, DefaultBody(r));
    r.emplace<DefaultInventory>(e, DefaultInventory(r, 6 * 5));
    r.emplace<InitBodyAndInventory>(e);

    auto& player_thrust = r.emplace<MovementAsteroidsComponent>(e);
    player_thrust.able_to_change_thrust = false;
    player_thrust.able_to_change_dir = true;
  }

  r.emplace<MovementJetpackComponent>(e);
  r.emplace<DefaultColour>(e, engine::SRGBColour{ 255, 255, 117, 1.0f });
  add_particles(r, e); // requires default colour

  set_position(r, e, desc.pos);
  return e;
};

entt::entity
Factory_ActorSpaceShip::create(entt::registry& r, const ActorSpaceShip& desc)
{
  const auto type = Actor<EntityType::actor_space_ship>::type;
  const auto e = Factory_BaseActor::create(r, type, desc);
  const auto size = HALF_SIZE;

  PhysicsDescription pdesc;
  pdesc.type = b2_dynamicBody;
  pdesc.is_bullet = false;
  pdesc.density = 1.0;
  pdesc.position = desc.pos;
  pdesc.size = size;
  pdesc.is_sensor = false;
  create_physics_actor(r, e, pdesc);

  r.emplace<DefaultColour>(e, desc.colour);

  if (desc.team == AvailableTeams::player) {

    r.emplace<PlayerComponent>(e);
    r.emplace<InputComponent>(e);
    r.emplace<KeyboardComponent>(e);
    auto& player_thrust = r.emplace<MovementAsteroidsComponent>(e);
    player_thrust.able_to_change_thrust = false;
    player_thrust.able_to_change_dir = true;
  }

  if (desc.team == AvailableTeams::enemy) {
    r.emplace<EnemyComponent>(e);
  }

  set_colour(r, e, desc.colour);
  r.get<TransformComponent>(e).position.z = 1; // above particles
  add_particles(r, e);

  return e;
};

entt::entity
Factory_ActorSpaceCargo::create(entt::registry& r, const ActorSpaceCargo& desc)
{
  const auto type = Actor<EntityType::actor_space_cargo>::type;
  const auto e = Factory_BaseActor::create(r, type, desc);
  const auto size = HALF_SIZE;

  PhysicsDescription pdesc;
  pdesc.type = b2_dynamicBody;
  pdesc.is_bullet = false;
  pdesc.density = 1.0;
  pdesc.position = desc.pos;
  pdesc.size = HALF_SIZE;
  pdesc.is_sensor = false;
  create_physics_actor(r, e, pdesc);

  return e;
};

entt::entity
Factory_ActorSpaceCapsule::create(entt::registry& r, const ActorSpaceCapsule& desc)
{
  const auto type = Actor<EntityType::actor_space_capsule>::type;
  const auto e = Factory_BaseActor::create(r, type, desc);
  const auto size = HALF_SIZE;

  PhysicsDescription pdesc;
  pdesc.type = b2_dynamicBody;
  pdesc.is_bullet = false;
  pdesc.density = 1.0;
  pdesc.position = desc.pos;
  pdesc.size = HALF_SIZE;
  pdesc.is_sensor = true;
  create_physics_actor(r, e, pdesc);

  return e;
};

entt::entity
Factory_BulletDefault::create(entt::registry& r, const BulletDefault& desc)
{
  const auto type = Actor<EntityType::bullet_default>::type;
  const auto e = Factory_BaseActor::create(r, type, desc);
  const auto size = SMALL_SIZE;
  create_physics_actor_dynamic(r, e, desc.pos, size, true);

  auto& t = r.get<TransformComponent>(e);
  t.rotation_radians.z = desc.rotation;
  t.scale = { size.x, size.y, 0.0f };

  const b2Vec2 vel = { desc.dir.x * desc.bullet_speed, desc.dir.y * desc.bullet_speed };
  auto& bullet_c = r.get<PhysicsBodyComponent>(e);
  bullet_c.base_speed = desc.bullet_speed;
  bullet_c.body->SetLinearVelocity(vel);

  r.emplace<AttackComponent>(e, AttackComponent{ desc.bullet_damage });
  r.emplace<AttackIdComponent>(e);
  r.emplace<EntityTimedLifecycle>(e);
  r.emplace<HasParentComponent>(e).parent = desc.parent;
  r.emplace<TeamComponent>(e, desc.team);

  BulletComponent bc;
  bc.bounce_bullet_on_wall_collision = false;
  bc.destroy_bullet_on_wall_collision = true;
  r.emplace<BulletComponent>(e, bc);

  return e;
};

entt::entity
Factory_BulletBouncy::create(entt::registry& r, const BulletBouncy& desc)
{
  const auto type = Actor<EntityType::bullet_bouncy>::type;
  const auto e = Factory_BaseActor::create(r, type, desc);
  const auto size = SMALL_SIZE;
  create_physics_actor_dynamic(r, e, desc.pos, size, true);

  auto& t = r.get<TransformComponent>(e);
  t.rotation_radians.z = desc.rotation;
  t.scale = { size.x, size.y, 0.0f };

  const b2Vec2 vel = { desc.dir.x * desc.bullet_speed, desc.dir.y * desc.bullet_speed };
  auto& bullet_c = r.get<PhysicsBodyComponent>(e);
  bullet_c.base_speed = desc.bullet_speed;
  bullet_c.body->SetLinearVelocity(vel);

  r.emplace<AttackComponent>(e, AttackComponent{ desc.bullet_damage });
  r.emplace<AttackIdComponent>(e);
  r.emplace<EntityTimedLifecycle>(e);
  r.emplace<HasParentComponent>(e).parent = desc.parent;
  r.emplace<TeamComponent>(e, desc.team);

  BulletComponent bc;
  bc.bounce_bullet_on_wall_collision = true;
  bc.destroy_bullet_on_wall_collision = false;
  r.emplace<BulletComponent>(e, bc);

  return e;
};

entt::entity
Factory_Particle::create(entt::registry& r, const Particle& desc)
{
  const auto type = Actor<EntityType::particle>::type;
  const auto e = Factory_BaseActor::create(r, type, desc);
  const auto size = HALF_SIZE;

  r.emplace<EntityTimedLifecycle>(e, desc.time_to_live_ms);
  r.emplace<VelocityTemporaryComponent>(e, VelocityTemporaryComponent{ desc.velocity.x, desc.velocity.y });

  // WARNING: random rotation here
  static engine::RandomState rnd(0);
  auto& t = r.get<TransformComponent>(e);
  t.rotation_radians.z = engine::rand_det_s(rnd.rng, 0.0f, 2.0f * engine::PI); // rnd rotation

  // make it shrink
  ScaleOverTimeComponent sotc;
  sotc.seconds_until_complete = desc.time_to_live_ms / 1000.0f;
  sotc.start_size = desc.start_size;
  sotc.end_size = desc.end_size;
  r.emplace<ScaleOverTimeComponent>(e, sotc);

  // update particle sprite to the correct sprite
  set_colour(r, e, desc.colour);

  return e;
};

entt::entity
Factory_SolidWall::create(entt::registry& r, const SolidWall& desc)
{
  const auto type = Actor<EntityType::solid_wall>::type;
  const auto e = Factory_BaseActor::create(r, type, desc);

  create_physics_actor_static(r, e, desc.pos, desc.size);
  r.emplace<LightOccluderComponent>(e);

  return e;
};

entt::entity
Factory_WeaponShotgun::create(entt::registry& r, const WeaponShotgun& desc)
{
  const auto type = Actor<EntityType::weapon_shotgun>::type;
  const auto e = Factory_BaseActor::create(r, type, desc);

  set_size(r, e, SMALL_SIZE);
  r.emplace<ShotgunComponent>(e);
  r.emplace<TeamComponent>(e, desc.team);

  if (desc.parent == entt::null)
    fmt::println("(ERROR) weaponshotgun creating with no parent");
  r.emplace<HasParentComponent>(e, desc.parent);

  if (desc.able_to_shoot)
    r.emplace<AbleToShoot>(e);

  return e;
};

} // namespace game2d