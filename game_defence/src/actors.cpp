#include "actors.hpp"

#include "colour/colour.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "lifecycle/components.hpp"
#include "modules/actor_bow/components.hpp"
#include "modules/actor_cursor/components.hpp"
#include "modules/actor_enemy/components.hpp"
#include "modules/actor_hearth/components.hpp"
#include "modules/actor_pickup_zone/components.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/actor_spawner/components.hpp"
#include "modules/actor_turret/components.hpp"
#include "modules/animation/components.hpp"
#include "modules/camera/orthographic.hpp"
#include "modules/combat_attack_cooldown/components.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/items_pickup/components.hpp"
#include "modules/lerp_to_target/components.hpp"
#include "modules/respawn/components.hpp"
#include "physics//components.hpp"
#include "renderer/components.hpp"
#include "resources/textures.hpp"
#include "sprites/components.hpp"
#include "sprites/helpers.hpp"

#include "magic_enum.hpp"
#include <thread>

namespace game2d {

SpriteComponent
create_sprite(entt::registry& r, const EntityType& type)
{
  const auto type_name = std::string(magic_enum::enum_name(type));

  std::string sprite = "EMPTY";

  if (type == EntityType::actor_hearth)
    sprite = "CAMPFIRE";
  else if (type == EntityType::actor_player)
    sprite = "PERSON_25_0";
  else if (type == EntityType::actor_spawner)
    sprite = "CASTLE_FLOOR";
  else if (type == EntityType::actor_turret)
    sprite = "EMPTY";
  else if (type == EntityType::actor_pickup_xp)
    sprite = "GEM";
  else if (type == EntityType::actor_pickup_zone)
    sprite = "EMPTY";
  // weapons...
  else if (type == EntityType::weapon_bow)
    sprite = "WEAPON_BOW_0";
  // bullets...
  else if (type == EntityType::bullet_default)
    sprite = "EMPTY";
  else if (type == EntityType::bullet_bow)
    sprite = "ARROW_1";
  // enemies...
  else if (type == EntityType::enemy_grunt)
    sprite = "PERSON_25_1";
  else if (type == EntityType::enemy_sniper)
    sprite = "PERSON_25_6";
  else if (type == EntityType::enemy_shotgunner)
    sprite = "PERSON_28_1";

  // else
  // std::cerr << "warning! sprite not implemented: " << type_name << "\n";

  SpriteComponent sc;

  auto& textures = get_first_component<SINGLETON_Textures>(r);
  auto& anims = get_first_component<SINGLETON_Animations>(r);

  // search spritesheet
  const auto anim = find_animation(anims.animations, sprite);
  sc.x = anim.animation_frames[0].x;
  sc.y = anim.animation_frames[0].y;

  if (anim.angle_degrees != 0.0f)
    sc.angle_radians = glm::radians(anim.angle_degrees);

  sc.tex_unit = get_tex_unit(textures, AvailableTexture::kenny);
  if (sc.tex_unit == get_tex_unit(textures, AvailableTexture::kenny)) {
    sc.sx = 48;
    sc.sy = 22;
  } else
    std::cerr << "warning! spritesize not implemented for: " << type_name << "\n";

  return sc;
};

SpriteColourComponent
create_colour(const SINGLETON_ColoursComponent& colours, const EntityType& type)
{
  const auto& primary = colours.lin_primary;
  const auto& secondary = colours.lin_secondary;
  const auto& tertiary = colours.lin_tertiary;
  const auto& quaternary = colours.lin_quaternary;

  SpriteColourComponent scc;
  scc.colour = primary;

  if (type == EntityType::actor_hearth)
    scc.colour = secondary;
  else if (type == EntityType::actor_pickup_zone) {
    engine::LinearColour off;
    off.r = 1.0f;
    off.g = 1.0f;
    off.b = 1.0f;
    off.a = 0.5f;
    scc.colour = std::make_shared<engine::LinearColour>(off);
  }
  // bullets...
  else if (type == EntityType::bullet_default)
    scc.colour = secondary;
  else if (type == EntityType::bullet_bow)
    scc.colour = primary;
  // enemies...
  else if (type == EntityType::enemy_grunt)
    scc.colour = colours.lin_yellow;
  else if (type == EntityType::enemy_sniper)
    scc.colour = colours.lin_blue;
  else if (type == EntityType::enemy_shotgunner)
    scc.colour = colours.lin_orange;

  return scc;
}

void
create_physics_actor(entt::registry& r, const entt::entity& e)
{
  r.emplace<PhysicsTransformXComponent>(e);
  r.emplace<PhysicsTransformYComponent>(e);
  r.emplace<AABB>(e);
  r.emplace<PhysicsActorComponent>(e);
};

entt::entity
create_gameplay(entt::registry& r, const EntityType& type)
{
  const auto& colours = get_first_component<SINGLETON_ColoursComponent>(r);

  const int SPRITE_SIZE = 16;
  const glm::ivec2 DEFAULT_SIZE{ 16, 16 };
  const glm::ivec2 HALF_SIZE{ 8, 8 };

  const auto type_name = std::string(magic_enum::enum_name(type));

  const auto& e = r.create();
  r.emplace<TagComponent>(e, type_name);
  r.emplace<EntityTypeComponent>(e, type);

  r.emplace<SpriteComponent>(e, create_sprite(r, type));
  r.emplace<SpriteColourComponent>(e, create_colour(colours, type));
  r.emplace<TransformComponent>(e);
  auto& transform = r.get<TransformComponent>(e);
  transform.scale.x = SPRITE_SIZE;
  transform.scale.y = SPRITE_SIZE;

  switch (type) {

      //
      // actors with only one type
      //

    case EntityType::actor_hearth: {
      create_physics_actor(r, e);
      r.emplace<HearthComponent>(e);
      r.emplace<HealthComponent>(e, 50);
      r.emplace<TeamComponent>(e, AvailableTeams::player);

      // spawn the player at the hearth
      SpawnerComponent hearth_spawner;
      hearth_spawner.type_to_spawn = EntityType::actor_player;
      hearth_spawner.continuous_spawn = false;
      r.emplace<SpawnerComponent>(e, hearth_spawner);

      break;
    }

    case EntityType::actor_player: {
      create_physics_actor(r, e);
      r.emplace<PhysicsSolidComponent>(e);
      r.emplace<VelocityComponent>(e);
      r.emplace<TeamComponent>(e, AvailableTeams::player);

      // gameplay
      r.emplace<PlayerComponent>(e);
      r.emplace<InputComponent>(e);
      r.emplace<KeyboardComponent>(e);
      r.emplace<ControllerComponent>(e);

      r.emplace<HealthComponent>(e, 100);
      r.emplace<InfiniteLivesComponent>(e);

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
      r.emplace<HealthComponent>(e, 10);

      // 1 second between spawning
      r.emplace<AttackCooldownComponent>(e, 1.0f);

      // if spawning enemies...
      // its on the enemy team
      // remember to update this?
      r.emplace<SpawnerComponent>(e);
      r.emplace<TeamComponent>(e, AvailableTeams::enemy);

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
      break;
    }

    case EntityType::actor_pickup_zone: {
      transform.scale.y = 100;
      transform.scale.x = 100;
      r.emplace<PickupZoneComponent>(e);
      create_physics_actor(r, e);
      break;
    }

      //
      // actor_weapons
      //

    case EntityType::weapon_bow: {
      r.emplace<BowComponent>(e);
      r.emplace<HasTargetPositionComponent>(e);
      r.emplace<LerpToTargetComponent>(e);
      r.emplace<AttackCooldownComponent>(e);
      break;
    }

      //
      // actors_bullets
      //

    case EntityType::bullet_bow: {
      create_physics_actor(r, e);
      r.emplace<VelocityComponent>(e);
      r.emplace<TeamComponent>(e, AvailableTeams::player);
      // no attack component as arrows are inactive sometimes
      break;
    }

    case EntityType::bullet_default: {
      transform.scale.x = HALF_SIZE.x;
      transform.scale.y = HALF_SIZE.y;
      create_physics_actor(r, e);
      r.emplace<TeamComponent>(e, AvailableTeams::player);
      r.emplace<VelocityComponent>(e);
      r.emplace<SetTransformAngleToVelocity>(e);
      r.emplace<EntityTimedLifecycle>(e);
      r.emplace<AttackComponent>(e, 3);
      break;
    }

    //
    // actors_enemies
    //
    case EntityType::enemy_grunt: {
      create_physics_actor(r, e);
      r.emplace<EnemyComponent>(e);
      r.emplace<TeamComponent>(e, AvailableTeams::enemy);
      r.emplace<VelocityComponent>(e);
      r.emplace<HealthComponent>(e, 3);
      r.emplace<AttackComponent>(e, 10);
      break;
    }

    case EntityType::enemy_sniper: {
      create_physics_actor(r, e);
      r.emplace<EnemyComponent>(e);
      r.emplace<TeamComponent>(e, AvailableTeams::enemy);
      r.emplace<VelocityComponent>(e);
      r.emplace<HealthComponent>(e, 3);
      r.emplace<AttackComponent>(e, 20);
      break;
    }

    case EntityType::enemy_shotgunner: {
      create_physics_actor(r, e);
      r.emplace<EnemyComponent>(e);
      r.emplace<TeamComponent>(e, AvailableTeams::enemy);
      r.emplace<VelocityComponent>(e);
      r.emplace<HealthComponent>(e, 4);
      r.emplace<AttackComponent>(e, 50);
      break;
    }

      //
      // misc
      //

    case EntityType::cursor: {
      r.emplace<CursorComponent>(e);
      create_physics_actor(r, e);
      r.emplace<VelocityComponent>(e);
      break;
    }

    case EntityType::particle: {
      r.emplace<VelocityComponent>(e);
      r.emplace<EntityTimedLifecycle>(e, 1 * 1000);
      break;
    }

    default: {
      std::cout << "warning: no gameplay implemented for: " << type_name << std::endl;
    }
  } // end switch

  return e;
};

} // namespace game2d