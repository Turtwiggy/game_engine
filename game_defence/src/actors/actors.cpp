#include "actors/actors.hpp"

#include "actors.hpp"
#include "actors/base.hpp"
#include "actors/helpers.hpp"
#include "bags/bullets.hpp"
#include "bags/items.hpp"
#include "entt/helpers.hpp"
#include "lifecycle/components.hpp"
#include "maths/grid.hpp"
#include "maths/maths.hpp"
#include "modules/actor_breach_charge/components.hpp"
#include "modules/actor_breach_charge/helpers.hpp"
#include "modules/actor_cover/components.hpp"
#include "modules/actor_enemy/components.hpp"
#include "modules/actor_lootbag/components.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/actor_weapon_shotgun/components.hpp"
#include "modules/algorithm_astar_pathfinding/components.hpp"
#include "modules/combat_attack_cooldown/components.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/combat_wants_to_shoot/components.hpp"
#include "modules/grid/components.hpp"
#include "modules/grid/helpers.hpp"
#include "modules/items_pickup/components.hpp"
#include "modules/lighting/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/system_change_gun_z_index/helpers.hpp"
#include "modules/system_drop_items/helpers.hpp"
#include "modules/system_move_to_target_via_lerp/components.hpp"
#include "modules/system_particles/components.hpp"
#include "modules/system_turnbased/components.hpp"
#include "modules/ui_colours/helpers.hpp"
#include "modules/ui_inventory/components.hpp"
#include "modules/ux_hoverable/components.hpp"
#include "physics/helpers.hpp"
#include "renderer/transform.hpp"

#include <box2d/b2_body.h>
#include <fmt/core.h>

namespace game2d {

// helpers
//

entt::entity
create_transform(entt::registry& r)
{
  EntityData desc;
  return Factory_BaseActor::create(r, desc, "create_transform()");
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
  desc.is_sensor = is_bullet; // for the moment, bullets only sensor
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
  pdesc.position = desc.pos;
  pdesc.size = desc.size;
  pdesc.is_sensor = true;
  create_physics_actor(r, e, pdesc);

  // add_components()
  // r.emplace<HoveredColour>(e, desc.hovered_colour);
  // r.emplace<HoverableComponent>(e);
  r.emplace<TurnBasedUnitComponent>(e);
  r.emplace<SpawnParticlesOnDeath>(e);
  r.emplace<MoveLimitComponent>(e, 1);
  r.emplace<HealthComponent>(e, desc.hp, desc.max_hp);
  r.emplace<DefenceComponent>(e, 0);     // should be determined by equipment
  r.emplace<PathfindComponent>(e, 1000); // pass through units if you must
  r.emplace<TeamComponent>(e, desc.team);
  r.emplace<DestroyBulletOnCollison>(e);
  r.emplace<DefaultColour>(e, desc.colour);

  // note: player inventory currently added to jetpack actor...

  if (desc.team == AvailableTeams::enemy) {
    r.emplace<EnemyComponent>(e);

    // give default body and inventory
    r.emplace<DefaultBody>(e, DefaultBody(r));

    // note: giving enemies smaller inventories
    r.emplace<DefaultInventory>(e, DefaultInventory(r, 2 * 6));
  }

  const auto& map_c = get_first_component<MapComponent>(r);
  const auto idx = engine::grid::worldspace_to_index(desc.pos, map_c.tilesize, map_c.xmax, map_c.ymax);
  add_entity_to_map(r, e, idx);
};

void
add_components(entt::registry& r, const entt::entity e, const DataWeaponShotgun& desc)
{
  // set_size(r, e, SMALL_SIZE);   // empty sprite
  set_size(r, e, DEFAULT_SIZE); // shotgun sprite
  set_z_index(r, e, ZLayer::PLAYER_GUN_ABOVE_PLAYER);

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
  const auto e = Factory_BaseActor::create(r, desc, typeid(desc).name());
  const auto size = DEFAULT_SIZE;

  DataDungeonActor mdesc; // copy?
  mdesc.size = size;

  add_components(r, e, desc);

  OnDeathCallback callback;
  callback.callback = [](entt::registry& r, const entt::entity e) {
    //
    drop_inventory_on_death_callback(r, e);
  };
  r.emplace<OnDeathCallback>(e, callback);

  return e;
};

entt::entity
Factory_DataDungeonLootbag::create(entt::registry& r, const DataDungeonLootbag& desc)
{
  const auto e = Factory_BaseActor::create(r, desc, typeid(desc).name());

  r.emplace<LootbagComponent>(e);
  r.emplace<DefaultInventory>(e, desc.inventory);
  r.emplace<PathfindComponent>(e, PathfindComponent{ 0 }); // free to move through

  set_z_index(r, e, ZLayer::BEHIND_PLAYER);

  const auto& map_c = get_first_component<MapComponent>(r);
  add_entity_to_map(r, e, engine::grid::worldspace_to_index(desc.pos, map_c.tilesize, map_c.xmax, map_c.ymax));
  return e;
}

entt::entity
Factory_DataDungeonCover::create(entt::registry& r, const DataDungeonCover& desc)
{
  const auto e = Factory_BaseActor::create(r, desc, typeid(desc).name());

  PhysicsDescription pdesc;
  pdesc.type = b2_staticBody;
  pdesc.is_bullet = false;
  pdesc.density = 1.0;
  pdesc.position = desc.pos;
  pdesc.size = desc.size;
  pdesc.is_sensor = true;
  create_physics_actor(r, e, pdesc);

  r.emplace<PathfindComponent>(e, -1); // cant pass through
  r.emplace<CoverComponent>(e);

  auto col = get_srgb_colour_by_tag(r, "solid_wall");
  r.emplace<DefaultColour>(e, col);
  set_colour(r, e, col);

  // only flash / scale size once per bullet
  r.emplace<DefenceHitListComponent>(e);

  const auto& map_c = get_first_component<MapComponent>(r);
  add_entity_to_map(r, e, engine::grid::worldspace_to_index(desc.pos, map_c.tilesize, map_c.xmax, map_c.ymax));

  set_position(r, e, desc.pos);
  return e;
}

entt::entity
Factory_DataJetpackActor::create(entt::registry& r, const DataJetpackActor& desc)
{
  const auto e = Factory_BaseActor::create(r, desc, typeid(desc).name());
  const auto size = DEFAULT_SIZE;

  PhysicsDescription pdesc;
  pdesc.type = b2_dynamicBody;
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

    auto col = engine::SRGBColour{ 255, 255, 117, 1.0f };
    r.emplace<DefaultColour>(e, col);
    set_colour(r, e, col);

    // add_particles()
    DataParticleEmitter pedesc;
    pedesc.parent = e;
    pedesc.colour = engine::SRGBColour{ 255, 255, 117, 1.0f };
    auto particle_e = Factory_DataParticleEmitter::create(r, pedesc);
  }

  set_position(r, e, desc.pos);
  return e;
};

entt::entity
Factory_DataSpaceShipActor::create(entt::registry& r, const DataSpaceShipActor& desc)
{
  const auto e = Factory_BaseActor::create(r, desc, typeid(desc).name());
  const auto size = HALF_SIZE;

  PhysicsDescription pdesc;
  pdesc.type = b2_dynamicBody;
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

  // add_particles()
  DataParticleEmitter pedesc;
  pedesc.parent = e;
  pedesc.colour = desc.colour;
  auto particle_e = Factory_DataParticleEmitter::create(r, pedesc);

  set_position(r, e, desc.pos);
  return e;
};

entt::entity
Factory_DataSpaceCargoActor::create(entt::registry& r, const DataSpaceCargoActor& desc)
{
  const auto e = Factory_BaseActor::create(r, desc, typeid(desc).name());
  const auto size = HALF_SIZE;

  PhysicsDescription pdesc;
  pdesc.type = b2_dynamicBody;
  pdesc.position = desc.pos;
  pdesc.size = HALF_SIZE;
  pdesc.is_sensor = false;
  create_physics_actor(r, e, pdesc);

  set_position(r, e, desc.pos);
  return e;
};

entt::entity
Factory_DataSpaceCapsuleActor::create(entt::registry& r, const DataSpaceCapsuleActor& desc)
{
  const auto e = Factory_BaseActor::create(r, desc, typeid(desc).name());
  const auto size = HALF_SIZE;

  PhysicsDescription pdesc;
  pdesc.type = b2_dynamicBody;
  pdesc.position = desc.pos;
  pdesc.size = HALF_SIZE;
  pdesc.is_sensor = true;
  create_physics_actor(r, e, pdesc);

  set_position(r, e, desc.pos);
  return e;
};

entt::entity
Factory_DataParticle::create(entt::registry& r, const DataParticle& desc)
{
  const auto e = Factory_BaseActor::create(r, desc, typeid(desc).name());
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
Factory_DataParticleEmitter::create(entt::registry& r, const DataParticleEmitter& desc)
{
  const auto e = Factory_BaseActor::create(r, desc, typeid(desc).name());
  const auto parent = desc.parent;

  r.emplace<SetPositionAtDynamicTarget>(e);
  r.emplace<DynamicTargetComponent>(e, parent);

  // This could be unique per add_particles but for the moment,
  // just give everything that need particles the same particle
  DataParticle pdesc;
  pdesc.start_size = desc.start_size;
  pdesc.end_size = desc.end_size;
  pdesc.colour = desc.colour;
  pdesc.sprite = "EMPTY";
  pdesc.velocity = desc.velocity;
  pdesc.time_to_live_ms = 1 * 1000;

  // which particle to spawn?
  ParticleEmitterComponent pedesc;
  pedesc.particle_to_emit = pdesc;
  r.emplace<ParticleEmitterComponent>(e, pedesc);

  // emit: particles
  AttackCooldownComponent cooldown;
  cooldown.time_between_attack = 0.1f;
  cooldown.time_between_attack_left = cooldown.time_between_attack;
  r.emplace<AttackCooldownComponent>(e, cooldown);

  set_size(r, e, { 0, 0 }); // no size for particle emitter
  return e;
};

entt::entity
Factory_DataSolidWall::create(entt::registry& r, const DataSolidWall& desc)
{
  const auto e = Factory_BaseActor::create(r, desc, typeid(desc).name());

  create_physics_actor_static(r, e, desc.pos, desc.size);

  r.emplace<LightOccluderComponent>(e);
  r.emplace<DestroyBulletOnCollison>(e);
  r.emplace<RequestParticleOnCollision>(e);

  set_position(r, e, desc.pos);
  return e;
};

//
// not sure if correct approach below
//

entt::entity
Factory_DataArmour::create(entt::registry& r, const DataArmour& desc)
{
  const auto e = Factory_BaseActor::create(r, desc, typeid(desc).name());

  set_position(r, e, desc.pos);
  return e;
}

entt::entity
Factory_DataBullet::create(entt::registry& r, const DataBullet& desc)
{
  const auto e = Factory_BaseActor::create(r, desc, typeid(desc).name());
  const auto size = SMALL_SIZE;

  const bool is_bullet = true;
  create_physics_actor_dynamic(r, e, desc.pos, size, is_bullet);

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
  bc.bounce_bullet_on_wall_collision = desc.type == BulletType::BOUNCY;
  bc.type = desc.type;
  r.emplace<BulletComponent>(e, bc);

  set_position(r, e, desc.pos);
  return e;
};

entt::entity
Factory_DataWeaponShotgun::create(entt::registry& r, const DataWeaponShotgun& desc)
{
  const auto e = Factory_BaseActor::create(r, desc, typeid(desc).name());

  add_components(r, e, desc);

  set_position(r, e, get_position(r, desc.parent));
  return e;
};

entt::entity
Factory_DataBreachCharge::create(entt::registry& r, const DataBreachCharge& desc)
{
  const auto e = Factory_BaseActor::create(r, desc, typeid(desc).name());

  add_components(r, e, desc);

  set_position(r, e, desc.pos);
  return e;
};

entt::entity
Factory_DataMedkit::create(entt::registry& r, const DataMedkit& desc)
{
  const auto e = Factory_BaseActor::create(r, desc, typeid(desc).name());

  r.emplace<AbleToBePickedUp>(e);
  // r.emplace<MedKitComponent>(e);

  return e;
};

entt::entity
Factory_DataScrap::create(entt::registry& r, const DataScrap& desc)
{
  const auto e = Factory_BaseActor::create(r, desc, typeid(desc).name());
  const auto size = HALF_SIZE;

  r.emplace<AbleToBePickedUp>(e);
  r.emplace<ItemTypeComponent>(e, ItemType::scrap);

  set_position(r, e, desc.pos);
  return e;
};

} // namespace game2d