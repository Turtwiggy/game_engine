#include "actors.hpp"

#include "colour/colour.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "modules/actor_cursor/components.hpp"
#include "modules/actor_dropoff_zone/components.hpp"
#include "modules/actor_enemy/components.hpp"
#include "modules/actor_hearth/components.hpp"
#include "modules/actor_pickup_zone/components.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/actor_spawner/components.hpp"
#include "modules/actor_turret/components.hpp"
#include "modules/actor_weapon_bow/components.hpp"
#include "modules/actor_weapon_shotgun/components.hpp"
#include "modules/animation/components.hpp"
#include "modules/camera/components.hpp"
#include "modules/camera/orthographic.hpp"
#include "modules/combat_attack_cooldown/components.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/combat_wants_to_shoot/components.hpp"
#include "modules/gameplay_circle/components.hpp"
#include "modules/items_drop/components.hpp"
#include "modules/items_pickup/components.hpp"
#include "modules/lerp_to_target/components.hpp"
#include "modules/lifecycle/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/helpers.hpp"
#include "modules/respawn/components.hpp"
#include "modules/ui_colours/helpers.hpp"
#include "modules/ux_hoverable/components.hpp"
#include "physics/components.hpp"
#include "sprites/components.hpp"
#include "sprites/helpers.hpp"

#include "magic_enum.hpp"
#include <thread>

namespace game2d {

SpriteComponent
create_sprite(entt::registry& r, const EntityType& type)
{
  const auto type_name = std::string(magic_enum::enum_name(type));
  const auto& anims = get_first_component<SINGLE_Animations>(r);
  const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);

  std::string sprite = "EMPTY";

  if (type == EntityType::cursor)
    sprite = "EMPTY";

  else if (type == EntityType::actor_hearth)
    sprite = "CAMPFIRE";
  else if (type == EntityType::actor_player)
    sprite = "PERSON_25_0";
  else if (type == EntityType::actor_spawner)
    sprite = "CASTLE_FLOOR";
  else if (type == EntityType::actor_turret)
    sprite = "EMPTY";
  else if (type == EntityType::actor_pickup_xp)
    sprite = "GEM";
  // else if (type == EntityType::actor_pickup_zone)
  //   sprite = "EMPTY";
  // weapons...
  else if (type == EntityType::weapon_bow)
    sprite = "WEAPON_BOW_0";
  else if (type == EntityType::weapon_shotgun)
    sprite = "WEAPON_SHOTGUN";
  // bullets...
  else if (type == EntityType::bullet_default)
    sprite = "EMPTY";
  else if (type == EntityType::bullet_bow)
    sprite = "ARROW_1";
  else if (type == EntityType::bullet_enemy)
    sprite = "EMPTY";

  // enemies...
  else if (type == EntityType::enemy_dummy)
    sprite = "PERSON_25_1";
  else if (type == EntityType::enemy_grunt)
    sprite = "PERSON_25_1";
  else if (type == EntityType::enemy_ranged)
    sprite = "PERSON_25_6";
  // else if (type == EntityType::enemy_sniper)
  //   sprite = "PERSON_25_6";
  // else if (type == EntityType::enemy_shotgunner)
  //   sprite = "PERSON_28_1";

  else if (type == EntityType::vfx_muzzleflash)
    sprite = "MUZZLE_FLASH";

  // else
  // std::cerr << "warning! sprite not implemented: " << type_name << "\n";

  SpriteComponent sc;
  sc.colour = get_lin_colour_by_tag(r, type_name);

  // search spritesheet
  const auto anim = find_animation(anims, sprite);
  sc.tex_pos.x = anim.animation_frames[0].x;
  sc.tex_pos.y = anim.animation_frames[0].y;

  if (anim.angle_degrees != 0.0f)
    sc.angle_radians = glm::radians(anim.angle_degrees);

  //
  // kennynl texture
  //
  sc.tex_unit = search_for_texture_unit_by_path(ri, "monochrome")->unit;
  sc.total_sx = 48;
  sc.total_sy = 22;

  //
  // muzzleflash texture
  //
  if (type == EntityType::vfx_muzzleflash) {
    sc.tex_unit = search_for_texture_unit_by_path(ri, "muzzle")->unit;
    sc.total_sx = 5;
    sc.total_sy = 1;
    sc.tex_pos.x = anim.animation_frames[0].x;
    sc.tex_pos.y = anim.animation_frames[0].y;
  }

  return sc;
};

void
create_physics_actor(entt::registry& r, const entt::entity& e)
{
  r.emplace<PhysicsTransformXComponent>(e);
  r.emplace<PhysicsTransformYComponent>(e);
  r.emplace<AABB>(e);
  r.emplace<PhysicsActorComponent>(e);
  r.emplace<VelocityComponent>(e);
};

entt::entity
create_gameplay(entt::registry& r, const EntityType& type)
{
  const glm::ivec3 DEFAULT_SIZE{ 32, 32, 1 };
  const glm::ivec3 HALF_SIZE{ 16, 16, 1 };
  const glm::ivec2 SMALL_SIZE{ 4, 4 };

  const auto type_name = std::string(magic_enum::enum_name(type));

  const auto& e = r.create();
  r.emplace<TagComponent>(e, type_name);
  r.emplace<EntityTypeComponent>(e, type);
  r.emplace<WaitForInitComponent>(e);

  r.emplace<SpriteComponent>(e, create_sprite(r, type));
  r.emplace<TransformComponent>(e);
  auto& transform = r.get<TransformComponent>(e);
  transform.scale = DEFAULT_SIZE;

  switch (type) {

    case EntityType::empty: {
      break;
    }
    case EntityType::empty_with_physics: {
      create_physics_actor(r, e);
      break;
    }

      //
      // actors with only one type
      //

    case EntityType::actor_hearth: {
      create_physics_actor(r, e);
      r.emplace<HearthComponent>(e);
      r.emplace<HealthComponent>(e, 50, 50);
      r.emplace<TeamComponent>(e, AvailableTeams::player);
      r.emplace<HoverableComponent>(e);

      // spawn the player at the hearth
      // SpawnerComponent hearth_spawner;
      // hearth_spawner.type_to_spawn = EntityType::actor_player;
      // hearth_spawner.continuous_spawn = false;
      // r.emplace<SpawnerComponent>(e, hearth_spawner);

      break;
    }

    case EntityType::actor_player: {
      const float player_speed = 100.0f;

      create_physics_actor(r, e);
      // r.emplace<PhysicsSolidComponent>(e);
      r.emplace<TeamComponent>(e, AvailableTeams::player);

      // gameplay
      r.emplace<PlayerComponent>(e);
      r.emplace<InputComponent>(e);
      r.emplace<KeyboardComponent>(e);
      r.emplace<ControllerComponent>(e);

      // movement
      r.emplace<HasTargetPositionComponent>(e);
      r.emplace<SetVelocityToTargetComponent>(e, player_speed);

      r.emplace<HealthComponent>(e, 1000, 1000);
      // r.emplace<InventoryLimit>(e);
      r.emplace<CameraFollow>(e);
      r.emplace<HoverableComponent>(e);
      r.emplace<CircleComponent>(e);
      // r.emplace<InfiniteLivesComponent>(e);
      // r.emplace<GeneratePickupZoneComponent>(e);

      // r.emplace<TakeDamageComponent>(e);
      // r.emplace<XpComponent>(e, 0);
      // StatsComponent stats;
      // stats.con_level = 1;
      // stats.agi_level = 1;
      // stats.str_level = 1;
      // r.emplace<StatsComponent>(e, stats);

      break;
    }

    case EntityType::actor_spawner: {
      create_physics_actor(r, e);
      r.emplace<HealthComponent>(e, 10, 10);

      // X seconds between spawning
      r.emplace<AttackCooldownComponent>(e, 5.0f);

      // if spawning enemies...
      // its on the enemy team
      // remember to update this?
      r.emplace<SpawnerComponent>(e);
      r.emplace<TeamComponent>(e, AvailableTeams::enemy);
      r.emplace<HoverableComponent>(e);

      break;
    }

    case EntityType::actor_turret: {
      create_physics_actor(r, e);
      r.emplace<TurretComponent>(e);
      r.emplace<AttackCooldownComponent>(e);

      // todo: if make turret solid,
      // spawn bullets outside of turret
      // r.emplace<PhysicsSolidComponent>(e);
      break;
    }

    case EntityType::actor_pickup_xp: {
      create_physics_actor(r, e);
      r.emplace<AbleToBePickedUp>(e);
      r.emplace<ItemComponent>(e);
      break;
    }

      // case EntityType::actor_pickup_zone: {
      //   transform.scale.y = 100;
      //   transform.scale.x = 100;
      //   r.emplace<PickupZoneComponent>(e);
      //   create_physics_actor(r, e);
      //   break;
      // }

    case EntityType::actor_dispencer: {
      create_physics_actor(r, e);
      r.emplace<PickupZoneComponent>(e);
      break;
    }

    case EntityType::actor_customer_area: {
      create_physics_actor(r, e);
      r.emplace<DropoffZoneComponent>(e);
      break;
    }

      //
      // item
      //

      // case EntityType::item: {
      //   // should have a HasParent already attached
      //   r.emplace<ItemComponent>(e);
      //   break;
      // }

      //
      // solids
      //

    case EntityType::solid_wall: {
      r.emplace<PhysicsTransformXComponent>(e);
      r.emplace<PhysicsTransformYComponent>(e);
      r.emplace<AABB>(e);
      r.emplace<PhysicsSolidComponent>(e);
      break;
    }

      //
      // actor_weapons
      //

    case EntityType::weapon_bow: {
      create_physics_actor(r, e);
      r.emplace<BowComponent>(e);
      r.emplace<HasTargetPositionComponent>(e);
      r.emplace<SetVelocityToTargetComponent>(e);
      r.emplace<AttackCooldownComponent>(e);
      r.emplace<HasParentComponent>(e);
      break;
    }

    case EntityType::weapon_shotgun: {
      create_physics_actor(r, e);
      r.emplace<ShotgunComponent>(e);
      r.emplace<HasTargetPositionComponent>(e);
      r.emplace<SetVelocityToTargetComponent>(e);
      r.emplace<AttackCooldownComponent>(e, 1.2f); // seconds between spawning
      r.emplace<HasParentComponent>(e);
      break;
    }

      //
      // actors_bullets
      //

    case EntityType::bullet_bow: {
      create_physics_actor(r, e);
      r.emplace<TeamComponent>(e, AvailableTeams::player);
      // no attack component as arrows are inactive sometimes
      break;
    }

    case EntityType::bullet_default: {
      create_physics_actor(r, e);
      auto& aabb = r.get<AABB>(e);
      aabb.size = SMALL_SIZE;
      transform.scale.x = SMALL_SIZE.x;
      transform.scale.y = SMALL_SIZE.y;
      r.emplace<TeamComponent>(e, AvailableTeams::player);
      r.emplace<SetTransformAngleToVelocity>(e);
      r.emplace<EntityTimedLifecycle>(e);
      r.emplace<AttackComponent>(e, 3);
      break;
    }

    case EntityType::bullet_enemy: {
      create_physics_actor(r, e);
      auto& aabb = r.get<AABB>(e);
      aabb.size = SMALL_SIZE;
      transform.scale.x = SMALL_SIZE.x;
      transform.scale.y = SMALL_SIZE.y;
      r.emplace<TeamComponent>(e, AvailableTeams::enemy);
      r.emplace<SetTransformAngleToVelocity>(e);
      // r.emplace<EntityTimedLifecycle>(e);
      // r.emplace<AttackComponent>(e, 3);
      break;
    }

    //
    // actors_enemies
    //
    case EntityType::enemy_dummy: {
      create_physics_actor(r, e);
      r.emplace<EnemyComponent>(e);
      r.emplace<TeamComponent>(e, AvailableTeams::enemy);
      r.emplace<HealthComponent>(e, 100, 100);
      r.emplace<HoverableComponent>(e);
      r.emplace<InfiniteLivesComponent>(e);
      break;
    }
    case EntityType::enemy_grunt: {
      create_physics_actor(r, e);
      r.emplace<EnemyComponent>(e);
      r.emplace<HoverableComponent>(e);

      // items
      r.emplace<AbleToDropItem>(e);

      // movement
      r.emplace<HasTargetPositionComponent>(e);
      // r.emplace<LerpToTargetComponent>(e, speed);

      // combat
      r.emplace<TeamComponent>(e, AvailableTeams::enemy);
      r.emplace<HealthComponent>(e, 10, 10);
      r.emplace<MeleeComponent>(e);
      // r.emplace<AttackComponent>(e, 10); // on the equipped weapon?
      break;
    }

    case EntityType::enemy_ranged: {
      create_physics_actor(r, e);
      r.emplace<EnemyComponent>(e);
      r.emplace<HoverableComponent>(e);

      // items
      r.emplace<AbleToDropItem>(e);

      // movement
      r.emplace<HasTargetPositionComponent>(e);
      // r.emplace<LerpToTargetComponent>(e, speed);

      // combat
      r.emplace<TeamComponent>(e, AvailableTeams::enemy);
      r.emplace<HealthComponent>(e, 10, 10);
      r.emplace<RangedComponent>(e);
      r.emplace<AttackCooldownComponent>(e, 1.2f);
      // r.emplace<AttackComponent>(e, 10); // on the equipped weapon?
      break;
    }

      // case EntityType::enemy_shotgunner: {
      //   create_physics_actor(r, e);
      //   r.emplace<EnemyComponent>(e);
      //   r.emplace<TeamComponent>(e, AvailableTeams::enemy);
      //   r.emplace<HealthComponent>(e, 4, 4);
      //   r.emplace<AttackComponent>(e, 50);
      //   break;
      // }

      //
      // misc
      //

    case EntityType::cursor: {
      r.emplace<CursorComponent>(e);
      create_physics_actor(r, e);
      break;
    }

    case EntityType::particle: {
      transform.scale.x = HALF_SIZE.x;
      transform.scale.y = HALF_SIZE.y;
      r.emplace<VelocityComponent>(e);
      r.emplace<EntityTimedLifecycle>(e, 1 * 1000);

      auto& sc = r.get<SpriteComponent>(e);
      sc.colour.a = 0.1f;
      break;
    }

    case EntityType::vfx_muzzleflash: {
      SpriteAnimationComponent anim;
      anim.playing_animation_name = "MUZZLE_FLASH";
      anim.duration = 0.2f;
      anim.looping = false;
      r.emplace<SpriteAnimationComponent>(e, anim);
      break;
    }

    default: {
      std::cout << "warning: no gameplay implemented for: " << type_name << std::endl;
    }
  } // end switch

  return e;
};

} // namespace game2d