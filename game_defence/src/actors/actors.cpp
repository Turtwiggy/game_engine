#include "actors/actors.hpp"

#include "actors.hpp"
#include "actors/helpers.hpp"
#include "bags/bullets.hpp"
#include "bags/items.hpp"
#include "base.hpp"
#include "entt/helpers.hpp"
#include "lifecycle/components.hpp"
#include "maths/maths.hpp"
#include "modules/actor_breach_charge/components.hpp"
#include "modules/actor_breach_charge/helpers.hpp"
#include "modules/actor_enemy/components.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/actor_weapon_shotgun/components.hpp"
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

#include "magic_enum.hpp"
#include <fmt/core.h>

namespace game2d {

// helpers
//

const glm::ivec3 DEFAULT_SIZE{ 32, 32, 1 };
const glm::ivec3 HALF_SIZE{ 16, 16, 1 };
const glm::ivec2 SMALL_SIZE{ 4, 4 };

entt::entity
create_transform(entt::registry& r)
{
  EntityData desc;
  return Factory_BaseActor::create(r, desc);
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
  DataParticle pdesc;
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
add_components(entt::registry& r, const entt::entity e, const DataDungeonActor& desc)
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
add_components(entt::registry& r, const entt::entity e, const DataWeaponShotgun& desc)
{
  set_size(r, e, SMALL_SIZE);

  r.emplace<ShotgunComponent>(e);
  r.emplace<TeamComponent>(e, desc.team);

  if (desc.parent == entt::null)
    fmt::println("(ERROR) weaponshotgun creating with no parent");
  r.emplace<HasParentComponent>(e, desc.parent);

  if (desc.able_to_shoot)
    r.emplace<AbleToShoot>(e);
};

void
add_components(entt::registry& r, const entt::entity e, const DataBreachCharge& desc)
{
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
  r.emplace<BreachChargeComponent>(e);
  add_bomb_callback(r, e);
};

void
remove_components(entt::registry& r, const entt::entity e, const DataJetpackActor& desc)
{
  r.remove<MovementJetpackComponent>(e);
  r.remove<DefaultColour>(e);
  r.remove<TeamComponent>(e);

  auto& physics_c = get_first_component<SINGLE_Physics>(r);
  physics_c.world->DestroyBody(r.get<PhysicsBodyComponent>(e).body);
  r.remove<PhysicsBodyComponent>(e);
};

//
// factories
//

entt::entity
Factory_DataDungeonActor::create(entt::registry& r, const DataDungeonActor& desc)
{
  const auto e = Factory_BaseActor::create(r, desc);
  const auto size = DEFAULT_SIZE;

  DataDungeonActor mdesc; // copy?
  mdesc.size = size;

  add_components(r, e, desc);

  return e;
};

entt::entity
Factory_DataJetpackActor::create(entt::registry& r, const DataJetpackActor& desc)
{
  const auto e = Factory_BaseActor::create(r, desc);
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

    r.emplace<MovementJetpackComponent>(e);
  }

  r.emplace<DefaultColour>(e, engine::SRGBColour{ 255, 255, 117, 1.0f });
  add_particles(r, e); // requires default colour

  set_position(r, e, desc.pos);
  return e;
};

entt::entity
Factory_DataSpaceShipActor::create(entt::registry& r, const DataSpaceShipActor& desc)
{
  const auto e = Factory_BaseActor::create(r, desc);
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
Factory_DataSpaceCargoActor::create(entt::registry& r, const DataSpaceCargoActor& desc)
{
  const auto e = Factory_BaseActor::create(r, desc);
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
Factory_DataSpaceCapsuleActor::create(entt::registry& r, const DataSpaceCapsuleActor& desc)
{
  const auto e = Factory_BaseActor::create(r, desc);
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
Factory_DataParticle::create(entt::registry& r, const DataParticle& desc)
{
  const auto e = Factory_BaseActor::create(r, desc);
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
Factory_DataSolidWall::create(entt::registry& r, const DataSolidWall& desc)
{
  const auto e = Factory_BaseActor::create(r, desc);

  create_physics_actor_static(r, e, desc.pos, desc.size);
  r.emplace<LightOccluderComponent>(e);

  return e;
};

//
// hmm below
//

entt::entity
Factory_DataArmour::create(entt::registry& r, const DataArmour& desc)
{
  const auto e = Factory_BaseActor::create(r, desc);

  return e;
}

entt::entity
Factory_DataBullet::create(entt::registry& r, const DataBullet& desc)
{
  const auto e = Factory_BaseActor::create(r, desc);
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

  switch (desc.type) {

    case BulletType::BOUNCY: {
      bc.bounce_bullet_on_wall_collision = true;
      bc.destroy_bullet_on_wall_collision = false;
      break;
    }

    case BulletType::DEFAULT: {
      bc.bounce_bullet_on_wall_collision = false;
      bc.destroy_bullet_on_wall_collision = true;
      break;
    }
  }

  r.emplace<BulletComponent>(e, bc);
  fmt::println("bullet: {}", std::string(magic_enum::enum_name(desc.type)));

  return e;
};

entt::entity
Factory_DataWeaponShotgun::create(entt::registry& r, const DataWeaponShotgun& desc)
{
  const auto e = Factory_BaseActor::create(r, desc);

  add_components(r, e, desc);

  return e;
};

entt::entity
Factory_DataBreachCharge::create(entt::registry& r, const DataBreachCharge& desc)
{
  const auto e = Factory_BaseActor::create(r, desc);

  add_components(r, e, desc);

  return e;
};

entt::entity
Factory_DataMedkit::create(entt::registry& r, const DataMedkit& desc)
{
  const auto e = Factory_BaseActor::create(r, desc);

  // r.emplace<MedKitComponent>(e);

  return e;
};

entt::entity
Factory_DataScrap::create(entt::registry& r, const DataScrap& desc)
{
  const auto e = Factory_BaseActor::create(r, desc);
  const auto size = HALF_SIZE;

  return e;
};

} // namespace game2d