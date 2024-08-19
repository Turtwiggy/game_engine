#include "actors.hpp"

#include "colour/colour.hpp"
#include "entt/helpers.hpp"
#include "lifecycle/components.hpp"
#include "modules/actor_cursor/components.hpp"
#include "modules/actor_weapon/components.hpp"
#include "modules/actor_weapon_shotgun/components.hpp"
#include "modules/actors/helpers.hpp"
#include "modules/algorithm_astar_pathfinding/components.hpp"
#include "modules/combat_attack_cooldown/components.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/combat_wants_to_shoot/components.hpp"
#include "modules/lighting/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/helpers.hpp"
#include "modules/system_move_to_target_via_lerp/components.hpp"
#include "modules/system_particles/components.hpp"
#include "modules/system_turnbased/components.hpp"
#include "modules/ui_colours/helpers.hpp"
#include "modules/ux_hoverable/components.hpp"
#include "physics/components.hpp"
#include "physics/helpers.hpp"
#include "renderer/components.hpp"
#include "renderer/transform.hpp"
#include "sprites/components.hpp"
#include "sprites/helpers.hpp"

#include "magic_enum.hpp"

#include <box2d/b2_body.h>
#include <fmt/core.h>
#include <string>

namespace game2d {
using namespace std::literals;

//
SpriteComponent
create_sprite(entt::registry& r, const std::string& sprite, const EntityType& type)
{
  const auto type_name = std::string(magic_enum::enum_name(type));

  SpriteComponent sc;

  // Check if this component exists.
  // if it does not, we may be running in a test-suite.
  const auto maybe_anims = get_first<SINGLE_Animations>(r);
  if (maybe_anims == entt::null)
    return sc;

  const auto& anims = get_first_component<SINGLE_Animations>(r);
  const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);

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
  // else if (type == EntityType::actor_hearth)
  //   sprite = "CAMPFIRE";
  else if (type == EntityType::actor_spaceship)
    sprite = "SPACE_VEHICLE_1";
  else if (type == EntityType::actor_capsule)
    sprite = "EMPTY";
  else if (type == EntityType::actor_cargo)
    sprite = "DICE_DARK_X";
  else if (type == EntityType::actor_breach_charge)
    sprite = "WEAPON_GRENADE";
  // else if (type == EntityType::actor_player)
  //   sprite = "EMPTY";
  // else if (type == EntityType::actor_enemy_patrol)
  //   sprite = "EMPTY";
  // else if (type == EntityType::actor_spawner)
  //   sprite = "CASTLE_FLOOR";
  // else if (type == EntityType::actor_turret)
  //   sprite = "EMPTY";
  else if (type == EntityType::actor_pickup_xp)
    sprite = "GEM";
  else if (type == EntityType::actor_pickup_doubledamage)
    sprite = "CARD_HEARTS_2";
  // else if (type == EntityType::actor_barricade)
  //   sprite = "WOOD_WALL_SMALL";
  // else if (type == EntityType::actor_barrel)
  //   sprite = "BARREL_0";

  // weapons...
  //
  // else if (type == EntityType::weapon_bow)
  //   sprite = "WEAPON_BOW_0";
  else if (type == EntityType::weapon_shotgun)
    sprite = "EMPTY";
  // else if (type == EntityType::weapon_shotgun)
  //   sprite = "WEAPON_SHOTGUN";
  // bullets...
  else if (type == EntityType::bullet_default)
    sprite = "EMPTY";
  else if (type == EntityType::bullet_bouncy)
    sprite = "EMPTY";
  // else if (type == EntityType::bullet_bow)
  //   sprite = "ARROW_1";
  // else if (type == EntityType::bullet_enemy)
  //   sprite = "EMPTY";

  // enemies...
  // else if (type == EntityType::enemy_dummy)
  //   sprite = "PERSON_25_1";
  // else if (type == EntityType::enemy_grunt)
  //   sprite = "PERSON_25_1";
  // else if (type == EntityType::enemy_ranged)
  //   sprite = "PERSON_25_6";
  // else if (type == EntityType::enemy_sniper)
  //   sprite = "PERSON_25_6";
  // else if (type == EntityType::enemy_shotgunner)
  //   sprite = "PERSON_28_1";

  // else
  //   fmt::println("warning! sprite set to empty: " << type_name << ".\n";

  return sprite;
};

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
}

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

[[nodiscard]] entt::entity
create_transform(entt::registry& r)
{
  return create_gameplay(r, EntityType::empty_with_transform, { 0, 0 });
};

void
add_particles(entt::registry& r, const entt::entity parent)
{
  const auto particle_emitter = create_transform(r);
  set_position(r, particle_emitter, get_position(r, parent));
  set_size(r, particle_emitter, { 0, 0 }); // no size just script, but need position

  r.emplace<SetPositionAtDynamicTarget>(particle_emitter);
  r.emplace<DynamicTargetComponent>(particle_emitter, parent);

  // which particle to spawn?
  ParticleDescription desc;
  desc.sprite = "EMPTY";
  desc.default_colour = engine::SRGBColour(1.0f, 1.0f, 1.0f, 1.0f);
  desc.start_size = 6;
  desc.end_size = 0;
  desc.default_colour = r.get<DefaultColour>(parent).colour;
  r.emplace<ParticleEmitterComponent>(particle_emitter, desc);

  // emit: particles
  AttackCooldownComponent cooldown;
  cooldown.time_between_attack = 0.1f;
  cooldown.time_between_attack_left = cooldown.time_between_attack;
  r.emplace<AttackCooldownComponent>(particle_emitter, cooldown);
};

entt::entity
create_gameplay(entt::registry& r, const EntityType& type, const glm::vec2& position, const std::optional<glm::vec2> size)
{
  const auto type_name = std::string(magic_enum::enum_name(type));

  const auto& e = r.create();
  r.emplace<TagComponent>(e, type_name);
  r.emplace<EntityTypeComponent>(e, type);
  r.emplace<WaitForInitComponent>(e);

  const auto sprite_name = sprite_type_to_sprite(r, type);
  const auto sc = create_sprite(r, sprite_name, type);
  r.emplace<SpriteComponent>(e, sc);

  const glm::ivec3 DEFAULT_SIZE{ 32, 32, 1 };
  const glm::ivec3 HALF_SIZE{ 16, 16, 1 };
  const glm::ivec2 SMALL_SIZE{ 4, 4 };

  glm::vec2 size_final = size.has_value() ? size.value() : DEFAULT_SIZE;
  if (type == EntityType::actor_spaceship)
    size_final = HALF_SIZE;
  else if (type == EntityType::bullet_bouncy)
    size_final = SMALL_SIZE;
  else if (type == EntityType::bullet_default)
    size_final = SMALL_SIZE;
  else if (type == EntityType::particle)
    size_final = HALF_SIZE;

  TransformComponent tf{ { position.x, position.y, 0.0f }, glm::vec3(0.0f), { size_final.x, size_final.y, 0.0f } };
  r.emplace<TransformComponent>(e, tf);

  switch (type) {
    case EntityType::empty_with_transform:
      break;
    case EntityType::empty_with_physics: {
      create_physics_actor_dynamic(r, e, position, size_final);
      break;
    }

      //
      // actors with only one type
      //

    case EntityType::actor_breach_charge: {
      PhysicsDescription desc;
      desc.type = b2_dynamicBody;
      desc.is_bullet = false;
      desc.density = 1.0;
      desc.position = position;
      desc.size = size_final;
      desc.is_sensor = false;
      create_physics_actor(r, e, desc);

      break;
    }

    case EntityType::actor_spaceship: {
      PhysicsDescription desc;
      desc.type = b2_dynamicBody;
      desc.is_bullet = false;
      desc.density = 1.0;
      desc.position = position;
      desc.size = size_final;
      desc.is_sensor = false;
      create_physics_actor(r, e, desc);

      r.emplace<HoverableComponent>(e);
      break;
    }

    case EntityType::actor_capsule: {
      PhysicsDescription desc;
      desc.type = b2_dynamicBody;
      desc.is_bullet = false;
      desc.density = 1.0;
      desc.position = position;
      desc.size = HALF_SIZE;
      desc.is_sensor = true;
      create_physics_actor(r, e, desc);
      break;
    }

    case EntityType::actor_cargo: {
      PhysicsDescription desc;
      desc.type = b2_dynamicBody;
      desc.is_bullet = false;
      desc.density = 1.0;
      desc.position = position;
      desc.size = HALF_SIZE;
      desc.is_sensor = false;
      create_physics_actor(r, e, desc);
      break;
    }

    case EntityType::actor_dungeon: {
      PhysicsDescription desc;
      desc.type = b2_kinematicBody;
      desc.is_bullet = false;
      desc.density = 1.0;
      desc.position = position;
      desc.size = size_final;
      desc.is_sensor = true;
      create_physics_actor(r, e, desc);

      r.emplace<HoverableComponent>(e);
      r.emplace<TurnBasedUnitComponent>(e);
      r.emplace<SpawnParticlesOnDeath>(e);

      const int move_limit = 1;
      r.emplace<MoveLimitComponent>(e, move_limit);

      const int hp = 100; // player hp
      r.emplace<HealthComponent>(e, hp, hp);
      r.emplace<DefenceComponent>(e, 0);     // should be determined by equipment
      r.emplace<PathfindComponent>(e, 1000); // pass through units if you must
      // r.emplace<TeamComponent>(e, AvailableTeams::neutral);
      break;
    }

      //
      // solids
      //

    case EntityType::solid_wall: {
      create_physics_actor_static(r, e, position, size_final);
      set_collision_filters(r, e);
      r.emplace<LightOccluderComponent>(e);
      break;
    }

      //
      // actor_weapons
      //

    case EntityType::weapon_shotgun: {
      // create_physics_actor_dynamic(r, e, DEFAULT_SIZE);
      set_size(r, e, SMALL_SIZE);

      r.emplace<WeaponComponent>(e);
      r.emplace<ShotgunComponent>(e);

      r.emplace<HasParentComponent>(e);
      // r.emplace<AttackCooldownComponent>(e, 1.2f); // seconds between shooting
      // r.emplace<AbleToShoot>(e);

      WeaponBulletTypeToSpawnComponent bullet_info;
      bullet_info.bullet_damage = 12;
      bullet_info.bullet_speed = 250.0f;
      bullet_info.bullet_type = EntityType::bullet_default;
      r.emplace<WeaponBulletTypeToSpawnComponent>(e, bullet_info);

      break;
    }

      //
      // actors_bullets
      //

    case EntityType::bullet_default: {
      create_physics_actor_dynamic(r, e, position, SMALL_SIZE, true);

      r.emplace<TeamComponent>(e, AvailableTeams::player);
      r.emplace<EntityTimedLifecycle>(e);

      BulletComponent bc;
      bc.bounce_bullet_on_wall_collision = false;
      bc.destroy_bullet_on_wall_collision = true;
      r.emplace<BulletComponent>(e, bc);
      break;
    }

    case EntityType::bullet_bouncy: {
      create_physics_actor_dynamic(r, e, position, SMALL_SIZE, true);

      r.emplace<TeamComponent>(e, AvailableTeams::player);
      r.emplace<EntityTimedLifecycle>(e);

      BulletComponent bc;
      bc.bounce_bullet_on_wall_collision = true;
      bc.destroy_bullet_on_wall_collision = false;
      r.emplace<BulletComponent>(e, bc);
      break;
    }

      //
      // misc
      //

    case EntityType::cursor: {
      r.emplace<CursorComponent>(e);
      // create_physics_actor(r, e);
      set_size(r, e, { 0, 0 });
      break;
    }

    case EntityType::particle: {
      r.emplace<VelocityTemporaryComponent>(e);
      r.emplace<EntityTimedLifecycle>(e, 3 * 1000);
      break;
    }

    default: {
      fmt::println("warning: no gameplay implemented for: {}", type_name);
    }
  } // end switch

  return e;
};

} // namespace game2d