#include "helpers.hpp"

#include "actors/actor_helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/physics/components.hpp"
#include "engine/sprites/components.hpp"
#include "engine/sprites/helpers.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/camera/components.hpp"
#include "modules/combat/components.hpp"
#include "modules/combat_gun_follow_player/gun_follow_player_components.hpp"
#include "modules/raws/raws_components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/helpers.hpp"
#include "modules/system_move_to_target_via_lerp/components.hpp"
#include "modules/ui_inventory/ui_inventory_components.hpp"

namespace game2d {

entt::entity
create_shotgun(entt::registry& r, entt::entity parent)
{
  const auto wep_e = spawn_item(r, "shotgun");
  r.emplace<HasWeaponComponent>(parent, HasWeaponComponent{ wep_e }); // parent <=> child
  r.emplace<HasParentComponent>(wep_e, HasParentComponent{ parent }); // child <=> parent
  r.emplace<TeamComponent>(wep_e, TeamComponent{ AvailableTeams::player });
  r.emplace<WeaponComponent>(wep_e);
  // r.emplace<AbleToShoot>(e);
  // r.emplace<ShotgunComponent>(e)
  set_size(r, wep_e, { 32, 32 }); // shotgun sprite
  set_z_index(r, wep_e, ZLayer::PLAYER_GUN_ABOVE_PLAYER);
  return wep_e;
}

entt::entity
create_bullet(entt::registry& r)
{
  // const auto e = Factory_BaseActor::create(r, desc, typeid(desc).name());
  // const auto size = SMALL_SIZE;
  // const bool is_bullet = true;
  // create_physics_actor_dynamic(r, e, desc.pos, size, is_bullet);
  // auto& t = r.get<TransformComponent>(e);
  // t.rotation_radians.z = desc.rotation;
  // t.scale = { size.x, size.y, 0.0f };
  // const b2Vec2 vel = { desc.dir.x * desc.bullet_speed, desc.dir.y * desc.bullet_speed };
  // auto& bullet_c = r.get<PhysicsBodyComponent>(e);
  // bullet_c.base_speed = desc.bullet_speed;
  // bullet_c.body->SetLinearVelocity(vel);
  // r.emplace<AttackComponent>(e, AttackComponent{ desc.bullet_damage });
  // r.emplace<AttackIdComponent>(e);
  // r.emplace<EntityTimedLifecycle>(e);
  // r.emplace<HasParentComponent>(e).parent = desc.parent;
  // r.emplace<TeamComponent>(e, desc.team);
  // BulletComponent bc;
  // bc.bounce_bullet_on_wall_collision = desc.type == BulletType::BOUNCY;
  // bc.type = desc.type;
  // r.emplace<BulletComponent>(e, bc);
  // set_position(r, e, desc.pos);
  return entt::null;
}

entt::entity
create_jetpack_player(entt::registry& r)
{
  auto e = spawn_mob(r, "dungeon_actor_hero", { 0, 0 });
  r.emplace<CameraFollow>(e);
  r.emplace<CircleComponent>(e);
  r.emplace<PlayerComponent>(e);
  r.emplace<TeamComponent>(e, AvailableTeams::player);
  r.get<PhysicsBodyComponent>(e).base_speed = 100.0f;
  r.emplace<MovementJetpackComponent>(e);
  r.emplace<DefaultBody>(e, DefaultBody(r));
  r.emplace<DefaultInventory>(e, DefaultInventory(r, 6 * 5));
  r.emplace<InitBodyAndInventory>(e);
  spawn_particle_emitter(r, "anything", { 0, 0 }, e);

  // give helmet to breathe
  const auto helmet_e = create_transform(r, "player helmet");
  r.emplace<SpriteComponent>(helmet_e);
  set_sprite(r, helmet_e, "HELMET_5");
  set_size(r, helmet_e, get_size(r, e));
  set_z_index(r, helmet_e, ZLayer::PLAYER_HELMET);
  set_colour(r, helmet_e, { 1.0f, 1.0f, 1.0f, 1.0f });
  r.emplace<DynamicTargetComponent>(helmet_e).target = e;
  r.emplace<SetRotationAsDynamicTarget>(helmet_e, SetRotationAsDynamicTarget{ 6.0f });
  r.emplace<TeamComponent>(helmet_e, r.get<TeamComponent>(e));

  return e;
}

entt::entity
create_dungeon_actor_enemy(entt::registry& r)
{
  // auto enemy_e = spawn_mob(r, "dungeon_actor_enemy_default");
  // r.emplace<TeamComponnent>(enemylymyymly);
  // give the enemy a piece of scrap in their inventory
  // create_inv_scrap(r, r.get<DefaultInventory>(dungeon_e).inv[0]);
  // give the enemy a 5% chance to have a medkit in their inventory...
  // TODO: medkits
  // r.emplace<DropItemsOnDeathComponent>(dungeon_e);
  // give enemies a shotgun
  // DataWeaponShotgun wdesc;
  // wdesc.able_to_shoot = true;
  // wdesc.parent = dungeon_e;
  // wdesc.team = desc.team;
  // const auto weapon_e = Factory_DataWeaponShotgun::create(r, wdesc);
  // link entity&weapon
  // HasWeaponComponent has_weapon_c;
  // has_weapon_c.instance = weapon_e;
  // r.emplace<HasWeaponComponent>(dungeon_e, has_weapon_c);
  // r.emplace<SpawnParticlesOnDeath>(e);
  // r.emplace<HealthComponent>(e, desc.hp, desc.max_hp);
  // r.emplace<DefenceComponent>(e, 0);     // should be determined by equipment
  // r.emplace<PathfindComponent>(e, 1000); // pass through units if you must
  // r.emplace<TeamComponent>(e, desc.team);
  // r.emplace<DestroyBulletOnCollison>(e);
  // r.emplace<DefaultColour>(e, desc.colour);
  return entt::null;
}

entt::entity
create_dungeon_actor_cover(entt::registry& r)
{
  // const auto e = Factory_BaseActor::create(r, desc, typeid(desc).name());
  // PhysicsDescription pdesc;
  // pdesc.type = b2_staticBody;
  // pdesc.is_bullet = false;
  // pdesc.density = 1.0;
  // pdesc.position = desc.pos;
  // pdesc.size = desc.size;
  // pdesc.is_sensor = true;
  // create_physics_actor(r, e, pdesc);
  // r.emplace<PathfindComponent>(e, -1); // cant pass through
  // r.emplace<CoverComponent>(e);
  // auto col = get_srgb_colour_by_tag(r, "solid_wall");
  // r.emplace<DefaultColour>(e, col);
  // set_colour(r, e, col);
  // r.emplace<DefenceHitListComponent>(e);
  // const auto& map_c = get_first_component<MapComponent>(r);
  // add_entity_to_map(r, e, engine::grid::worldspace_to_index(desc.pos, map_c.tilesize, map_c.xmax, map_c.ymax));
  return entt::null;
}

entt::entity
create_overworld_cargo(entt::registry& r)
{
  // auto e = spawn_item(r, "cargo");
  return entt::null;
}

} // namespace game2d