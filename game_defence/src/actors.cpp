#include "actors.hpp"

#include "colour/colour.hpp"
#include "entt/helpers.hpp"
#include "lifecycle/components.hpp"
#include "modules/actor_bodypart_head/components.hpp"
#include "modules/actor_bodypart_legs/components.hpp"
#include "modules/actor_cursor/components.hpp"
#include "modules/actor_enemy/components.hpp"
#include "modules/actor_group/components.hpp"
#include "modules/actor_hearth/components.hpp"
#include "modules/actor_particle/components.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/actor_spawner/components.hpp"
#include "modules/actor_turret/components.hpp"
#include "modules/actor_weapon_shotgun/components.hpp"
#include "modules/actors/helpers.hpp"
#include "modules/animation/components.hpp"
#include "modules/combat_attack_cooldown/components.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/items_drop/components.hpp"
#include "modules/items_pickup/components.hpp"
#include "modules/lerp_to_target/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/helpers.hpp"
#include "modules/system_knockback/components.hpp"
#include "modules/system_spaceship_door/components.hpp"
#include "modules/ui_colours/helpers.hpp"
#include "modules/ui_spaceship_designer/components.hpp"
#include "modules/ux_hoverable/components.hpp"
#include "modules/ux_hoverable_change_colour/components.hpp"
#include "physics/components.hpp"
#include "renderer/transform.hpp"
#include "sprites/components.hpp"
#include "sprites/helpers.hpp"

#include "magic_enum.hpp"

#include <string>

namespace game2d {
using namespace std::literals;

//
SpriteComponent
create_sprite(entt::registry& r, const std::string& sprite, const EntityType& type)
{
  const auto type_name = std::string(magic_enum::enum_name(type));
  const auto& anims = get_first_component<SINGLE_Animations>(r);
  const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);

  SpriteComponent sc;
  sc.colour = get_lin_colour_by_tag(r, type_name);

  // search spritesheet
  const auto [spritesheet, anim] = find_animation(anims, sprite);
  sc.tex_pos.x = anim.animation_frames[0].x;
  sc.tex_pos.y = anim.animation_frames[0].y;

  if (anim.angle_degrees != 0.0f)
    sc.angle_radians = glm::radians(anim.angle_degrees);

  sc.tex_unit = search_for_texture_unit_by_spritesheet_path(ri, spritesheet.path)->unit;
  sc.total_sx = spritesheet.nx; // number of sprites
  sc.total_sy = spritesheet.ny;

  return sc;
};

std::string
sprite_type_to_sprite(entt::registry& r, const EntityType& type)
{
  const auto type_name = std::string(magic_enum::enum_name(type));

  std::string sprite = "EMPTY";

  if (type == EntityType::cursor)
    sprite = "EMPTY";
  else if (type == EntityType::actor_hearth)
    sprite = "CAMPFIRE";
  // else if (type == EntityType::actor_player)
  //   sprite = "PERSON_25_0";
  else if (type == EntityType::actor_player_ally)
    sprite = "PERSON_26_0";
  else if (type == EntityType::actor_spawner)
    sprite = "CASTLE_FLOOR";
  else if (type == EntityType::actor_turret)
    sprite = "EMPTY";
  else if (type == EntityType::actor_pickup_xp)
    sprite = "GEM";
  else if (type == EntityType::actor_pickup_doubledamage)
    sprite = "CARD_HEARTS_2";
  else if (type == EntityType::actor_unitgroup)
    sprite = "SHIELD_2_2";
  else if (type == EntityType::actor_barricade)
    sprite = "WOOD_WALL_SMALL";

  // weapons...
  //
  // else if (type == EntityType::weapon_bow)
  //   sprite = "WEAPON_BOW_0";
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

  // else
  //   std::cout << "warning! sprite set to empty: " << type_name << ".\n";

  return sprite;
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

  if (type != EntityType::empty_no_transform) {
    const auto sprite_name = sprite_type_to_sprite(r, type);
    const auto sc = create_sprite(r, sprite_name, type);
    r.emplace<SpriteComponent>(e, sc);
    r.emplace<TransformComponent>(e);
    set_size(r, e, DEFAULT_SIZE);

    switch (type) {
      case EntityType::bullet_default: {
        set_size(r, e, SMALL_SIZE);
        break;
      }
      case EntityType::bullet_enemy: {
        set_size(r, e, SMALL_SIZE);
        break;
      }
      case EntityType::particle: {
        set_size(r, e, HALF_SIZE);
        break;
      }
    }
  }

  switch (type) {

    case EntityType::empty_no_transform: {
      break;
    }
    case EntityType::empty_with_transform: {
      break;
    }
    case EntityType::empty_with_physics: {
      create_physics_actor(r, e);
      break;
    }

      //
      // actors with only one type
      //

    case EntityType::actor_spaceship: {
      r.emplace<SpaceshipComponent>(e);
      // spaceship is a container for other sprites
      r.remove<TransformComponent>(e);
      r.remove<SpriteComponent>(e);
      break;
    }
    case EntityType::solid_spaceship_point: {
      create_physics_actor(r, e);
      r.emplace<PhysicsSolidComponent>(e);

      r.emplace<SpaceshipPointComponent>(e);
      r.emplace<HoverableComponent>(e);
      r.emplace<ChangeColourOnHoverComponent>(e);
      break;
    }
    case EntityType::solid_spaceship_door: {
      create_physics_actor(r, e);
      r.emplace<PhysicsSolidComponent>(e);

      r.emplace<HoverableComponent>(e);
      r.emplace<ChangeColourOnHoverComponent>(e);

      r.emplace<SpaceshipDoorComponent>(e);
      break;
    }

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

    case EntityType::actor_bodypart_head: {
      r.emplace<HeadComponent>(e);
      break;
    }
    case EntityType::actor_bodypart_leg: {
      LegsComponent legs;
      legs.lines.push_back(create_gameplay(r, EntityType::empty_with_transform));
      legs.lines.push_back(create_gameplay(r, EntityType::empty_with_transform));
      legs.lines.push_back(create_gameplay(r, EntityType::empty_with_transform));
      legs.lines.push_back(create_gameplay(r, EntityType::empty_with_transform));

      for (const auto& leg : legs.lines) {
        const auto sprite_name = sprite_type_to_sprite(r, EntityType::actor_bodypart_leg);
        const auto sc = create_sprite(r, sprite_name, EntityType::actor_bodypart_leg);
        r.emplace_or_replace<SpriteComponent>(leg, sc);
      }

      auto& leg_transform = r.get<TransformComponent>(e);
      leg_transform.scale.x = 0;
      leg_transform.scale.y = 0;

      r.emplace<LegsComponent>(e, legs);
      break;
    }
    case EntityType::actor_player: {
      create_physics_actor(r, e);
      set_size(r, e, { 12, 20 });

      auto& vel = r.get<VelocityComponent>(e);
      vel.base_speed = 50000.0f;
      r.emplace<KnockbackComponent>(e);

      // r.emplace<PhysicsSolidComponent>(e);
      r.emplace<TeamComponent>(e, AvailableTeams::player);

      // gameplay
      r.emplace<PlayerComponent>(e);
      r.emplace<InputComponent>(e);
      r.emplace<KeyboardComponent>(e);
      r.emplace<ControllerComponent>(e);

      // movement
      // r.emplace<HasTargetPositionComponent>(e);
      // r.emplace<SetVelocityToTargetComponent>(e);

      r.emplace<HealthComponent>(e, 200, 200);
      // r.emplace<InventoryLimit>(e);
      r.emplace<HoverableComponent>(e);
      // r.emplace<CircleComponent>(e);
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

    case EntityType::actor_player_ally: {
      create_physics_actor(r, e);
      auto& vel = r.get<VelocityComponent>(e);
      vel.base_speed = 50.0f;
      r.emplace<KnockbackComponent>(e);

      // movement
      r.emplace<HasTargetPositionComponent>(e);
      r.emplace<SetVelocityToTargetComponent>(e);

      // gameplay
      r.emplace<PartOfGroupComponent>(e);
      r.emplace<TeamComponent>(e, AvailableTeams::player);
      r.emplace<HealthComponent>(e, 100, 100);
      r.emplace<HoverableComponent>(e);

      break;
    }

    case EntityType::actor_unitgroup: {
      create_physics_actor(r, e);
      r.emplace<GroupComponent>(e);

      r.emplace<AbleToBePickedUp>(e);

      // if picked up as a banner, we pretty much want it to be stuck to the holder
      auto& vel = r.get<VelocityComponent>(e);
      vel.base_speed = 200.0f;
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

      // if make turret solid,
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

    case EntityType::actor_pickup_doubledamage: {
      create_physics_actor(r, e);
      r.emplace<AbleToBePickedUp>(e);
      r.emplace<ItemComponent>(e);
      break;
    }

    case EntityType::actor_barricade: {
      create_physics_actor(r, e);
      // r.emplace<PhysicsSolidComponent>(e);

      // can be killed
      const int hp = 100;
      r.emplace<HealthComponent>(e, hp, hp);

      // assume all barricades are player barricades?
      r.emplace<TeamComponent>(e, AvailableTeams::player);

      r.emplace<KnockbackComponent>(e);

      break;
    }

      //
      // solids
      //

    case EntityType::solid_wall: {
      create_physics_actor(r, e);
      r.emplace<PhysicsSolidComponent>(e);

      // can be killed
      r.emplace<HealthComponent>(e, 10, 10);

      break;
    }

      //
      // actor_weapons
      //

      // case EntityType::weapon_bow: {
      //   create_physics_actor(r, e);
      //   r.emplace<BowComponent>(e);
      //   r.emplace<HasTargetPositionComponent>(e);
      //   r.emplace<SetVelocityToTargetComponent>(e);
      //   r.emplace<AttackCooldownComponent>(e);
      //   r.emplace<HasParentComponent>(e);
      //   break;
      // }

    case EntityType::weapon_shotgun: {
      create_physics_actor(r, e);
      r.emplace<ShotgunComponent>(e);

      r.emplace<AttackCooldownComponent>(e, 1.2f); // seconds between shooting
      r.emplace<HasParentComponent>(e);
      r.emplace<WeaponBulletTypeToSpawnComponent>(e);

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
      set_size(r, e, SMALL_SIZE);

      r.emplace<TeamComponent>(e, AvailableTeams::player);
      r.emplace<SetTransformAngleToVelocity>(e);
      r.emplace<EntityTimedLifecycle>(e);
      r.emplace<BulletComponent>(e);
      break;
    }

    case EntityType::bullet_enemy: {
      create_physics_actor(r, e);
      set_size(r, e, SMALL_SIZE);

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
      r.emplace<KnockbackComponent>(e);
      break;
    }
    case EntityType::enemy_grunt: {
      create_physics_actor(r, e);
      float base_speed = 50.0f; // influences velocity
      auto& vel = r.get<VelocityComponent>(e);
      vel.base_speed = base_speed;

      r.emplace<EnemyComponent>(e);
      r.emplace<HoverableComponent>(e);

      // items
      r.emplace<AbleToDropItem>(e);

      // movement
      r.emplace<HasTargetPositionComponent>(e);
      r.emplace<SetVelocityToTargetComponent>(e);

      // combat
      r.emplace<TeamComponent>(e, AvailableTeams::enemy);
      r.emplace<HealthComponent>(e, 10, 10);
      r.emplace<MeleeComponent>(e);
      r.emplace<KnockbackComponent>(e);
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
      r.emplace<KnockbackComponent>(e);
      // r.emplace<AttackComponent>(e, 10); // on the equipped weapon?
      break;
    }

      //
      // misc
      //

    case EntityType::cursor: {
      r.emplace<CursorComponent>(e);
      create_physics_actor(r, e);
      break;
    }

    case EntityType::particle: {
      r.emplace<ParticleComponent>(e);
      r.emplace<VelocityComponent>(e);
      r.emplace<EntityTimedLifecycle>(e, 3 * 1000);
      break;
    }

    default: {
      std::cout << "warning: no gameplay implemented for: " << type_name << std::endl;
    }
  } // end switch

  return e;
};

} // namespace game2d