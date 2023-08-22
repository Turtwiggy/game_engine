#include "actors.hpp"

#include "colour/colour.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "lifecycle/components.hpp"
#include "modules/animation/components.hpp"
#include "modules/camera/orthographic.hpp"
#include "modules/combat/components.hpp"
#include "modules/enemy/components.hpp"
#include "modules/hearth/components.hpp"
#include "modules/items/components.hpp"
#include "modules/physics/components.hpp"
#include "modules/player/components.hpp"
#include "modules/respawn/components.hpp"
#include "modules/spawner/components.hpp"
#include "modules/turret/components.hpp"
#include "renderer/components.hpp"
#include "resources/textures.hpp"
#include "sprites/components.hpp"
#include "sprites/helpers.hpp"

#include "magic_enum.hpp"
#include <thread>

namespace game2d {

void
to_json(json& j, const EntityTypeComponent& et)
{
  j = json{ { "type", static_cast<int>(et.type) } };
};

void
from_json(const json& j, EntityTypeComponent& et)
{
  j.at("type").get_to(et.type);
};

SpriteComponent
create_sprite(entt::registry& r, const EntityType& type)
{
  const auto type_name = std::string(magic_enum::enum_name(type));

  std::string sprite = "EMPTY";
  if (type == EntityType::empty)
    sprite = "EMPTY";
  else if (type == EntityType::actor_player)
    sprite = "PERSON_25_0";
  else if (type == EntityType::actor_turret)
    sprite = "EMPTY";
  else if (type == EntityType::actor_bullet)
    sprite = "EMPTY";
  else if (type == EntityType::spawner)
    sprite = "CASTLE_FLOOR";
  else if (type == EntityType::actor_hearth)
    sprite = "CAMPFIRE";
  else if (type == EntityType::pickup_xp)
    sprite = "GEM";
  // else
  // std::cerr << "warning! sprite not implemented: " << type_name << "\n";

  SpriteComponent sc;

  auto& textures = get_first_component<SINGLETON_Textures>(r);
  auto& anims = get_first_component<SINGLETON_Animations>(r);

  // search spritesheet
  const auto anim = find_animation(anims.animations, sprite);
  sc.x = anim.animation_frames[0].x;
  sc.y = anim.animation_frames[0].y;

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

  if (type == EntityType::empty) {
    engine::LinearColour off;
    off.r = 1.0f;
    off.g = 1.0f;
    off.b = 1.0f;
    off.a = 0.5f;
    scc.colour = std::make_shared<engine::LinearColour>(off);
  }
  if (type == EntityType::actor_bullet) {
    engine::LinearColour off;
    off.r = 1.0f;
    off.g = 1.0f;
    off.b = 1.0f;
    off.a = 0.25f;
    scc.colour = std::make_shared<engine::LinearColour>(off);
  }
  if (type == EntityType::pickup_zone) {
    engine::LinearColour off;
    off.r = 1.0f;
    off.g = 1.0f;
    off.b = 1.0f;
    off.a = 0.1f;
    scc.colour = std::make_shared<engine::LinearColour>(off);
  }

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
    case EntityType::empty: {
      break;
    }

    case EntityType::cursor: {
      // r.emplace<PlayerCursor>(e);
      break;
    }

    case EntityType::particle: {
      r.emplace<VelocityComponent>(e);
      r.emplace<EntityTimedLifecycle>(e, 1 * 1000);
      break;
    }

    case EntityType::pickup_zone: {
      transform.scale.y = 100;
      transform.scale.x = 100;
      r.emplace<PickupZone>(e);
      create_physics_actor(r, e);
      break;
    }

    case EntityType::pickup_xp: {
      create_physics_actor(r, e);
      r.emplace<AbleToBePickedUp>(e);
      break;
    }

    case EntityType::actor_player: {
      create_physics_actor(r, e);
      r.emplace<PhysicsSolidComponent>(e);
      r.emplace<VelocityComponent>(e);

      // gameplay
      PlayerComponent pc;
      pc.debug_gun_spot = create_gameplay(r, EntityType::empty);
      pc.pickup_area = create_gameplay(r, EntityType::pickup_zone);
      r.emplace<PlayerComponent>(e, pc);
      r.emplace<InputComponent>(e);
      r.emplace<KeyboardComponent>(e);
      r.emplace<ControllerComponent>(e);

      // more hmm
      r.emplace<InfiniteLivesComponent>(e);

      // hmm
      r.emplace<HealthComponent>(e, 100);
      r.emplace<RangeComponent>(e, 10);

      // r.emplace<TakeDamageComponent>(e);
      // r.emplace<XpComponent>(e, 0);
      // StatsComponent stats;
      // stats.con_level = 1;
      // stats.agi_level = 1;
      // stats.str_level = 1;
      // r.emplace<StatsComponent>(e, stats);
      break;
    }

    case EntityType::actor_enemy: {
      create_physics_actor(r, e);
      r.emplace<EnemyComponent>(e);
      r.emplace<VelocityComponent>(e);
      // health, attack, range set on class
      break;
    }

    case EntityType::actor_turret: {
      create_physics_actor(r, e);
      r.emplace<TurretComponent>(e);

      // todo: if make turret solid,
      // spawn bullets outside of turret
      // r.emplace<PhysicsSolidComponent>(e);
      break;
    }

    case EntityType::actor_bullet: {
      transform.scale.x = HALF_SIZE.x;
      transform.scale.y = HALF_SIZE.y;

      r.emplace<AttackComponent>(e, 3);

      create_physics_actor(r, e);
      r.emplace<VelocityComponent>(e);
      r.emplace<SetTransformAngleToVelocity>(e);
      r.emplace<EntityTimedLifecycle>(e);
      break;
    }

    case EntityType::actor_hearth: {
      create_physics_actor(r, e);
      r.emplace<HearthComponent>(e);
      r.emplace<HealthComponent>(e, 50);

      // spawn the player at the hearth
      SpawnerComponent hearth_spawner;
      hearth_spawner.type_to_spawn = EntityType::actor_player;
      hearth_spawner.continuous_spawn = false;
      r.emplace<SpawnerComponent>(e, hearth_spawner);

      break;
    }

    case EntityType::spawner: {
      create_physics_actor(r, e);
      r.emplace<SpawnerComponent>(e);
      r.emplace<HealthComponent>(e, 10);
      break;
    }

    case EntityType::camera: {
      r.emplace<OrthographicCamera>(e);
      break;
    }

    case EntityType::line: {
      break;
    }
    default: {
      std::cout << "warning: no gameplay implemented for: " << type_name;
    }
  }

  return e;
};

} // namespace game2d