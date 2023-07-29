#include "actors.hpp"

#include "colour/colour.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "lifecycle/components.hpp"
#include "modules/camera/orthographic.hpp"
#include "modules/enemy/components.hpp"
#include "modules/health/components.hpp"
#include "modules/physics_box2d/components.hpp"
#include "modules/player/components.hpp"
#include "modules/spawner/components.hpp"
#include "modules/turret/components.hpp"
#include "renderer/components.hpp"
#include "resources/colours.hpp"
#include "resources/textures.hpp"
#include "sprites/components.hpp"
#include "sprites/helpers.hpp"

#include "magic_enum.hpp"

namespace game2d {

static int SPRITE_SIZE = 16;

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
  else if (type == EntityType::actor_enemy)
    sprite = "PERSON_29_2";
  else if (type == EntityType::actor_turret)
    sprite = "EMPTY";
  else if (type == EntityType::actor_bullet)
    sprite = "EMPTY";
  else if (type == EntityType::spawner)
    sprite = "CASTLE_FLOOR";
  else
    std::cerr << "warning! sprite not implemented: " << type_name << "\n";

  RenderOrder order = RenderOrder::foreground;

  //   if (type == EntityType::tile_type_floor)
  //     order = RenderOrder::background;

  SpriteComponent sc;
  sc.render_order = order;

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
create_colour(entt::registry& r, const EntityType& type)
{
  const auto type_name = std::string(magic_enum::enum_name(type));
  const auto& colours = get_first_component<SINGLETON_ColoursComponent>(r);

  const lin_ptr& primary = colours.lin_primary;
  const lin_ptr& secondary = colours.lin_secondary;
  const lin_ptr& tertiary = colours.lin_tertiary;
  const lin_ptr& quaternary = colours.lin_quaternary;

  lin_ptr chosen_col = primary;

  SpriteColourComponent scc;
  scc.colour = chosen_col;
  return scc;
}

void
create_physics(entt::registry& r, b2World& world, const entt::entity& e)
{
  b2Body* body = nullptr;

  // create a body
  {
    b2BodyDef body_def;
    body_def.position.Set(0.0f, 0.0f);
    body_def.angle = 0.0f;
    // body_def.bullet = false;
    body_def.fixedRotation = true;
    body_def.type = b2_dynamicBody; // if you want body to move
    body = world.CreateBody(&body_def);
  }

  // create a fixture
  {
    b2PolygonShape box;
    box.SetAsBox(8.0f, 8.0f); // half-size

    b2FixtureDef fixture_def;
    fixture_def.friction = 0.0f;
    fixture_def.density = 1.0f;
    fixture_def.shape = &box;
    body->CreateFixture(&fixture_def);
  }

  ActorComponent pcomp;
  pcomp.body = body;
  r.emplace<ActorComponent>(e, pcomp);
}

entt::entity
create_gameplay(entt::registry& r, b2World& world, const EntityType& type)
{
  const auto type_name = std::string(magic_enum::enum_name(type));

  const auto& e = r.create();
  r.emplace<TagComponent>(e, type_name);
  r.emplace<EntityTypeComponent>(e, type);

  r.emplace<SpriteComponent>(e, create_sprite(r, type));
  r.emplace<SpriteColourComponent>(e, create_colour(r, type));
  r.emplace<TransformComponent>(e);
  auto& transform = r.get<TransformComponent>(e);
  transform.scale.x = SPRITE_SIZE;
  transform.scale.y = SPRITE_SIZE;

  switch (type) {
    case EntityType::empty: {
      break;
    }
    case EntityType::actor_player: {

      create_physics(r, world, e);
      // gameplay

      PlayerComponent pc;
      r.emplace<PlayerComponent>(e, pc);
      r.emplace<InputComponent>(e);
      r.emplace<KeyboardComponent>(e);
      r.emplace<ControllerComponent>(e);
      r.emplace<HealthComponent>(e);
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
      create_physics(r, world, e);
      r.emplace<EnemyComponent>(e);
      r.emplace<HealthComponent>(e);
      break;
    }
    case EntityType::actor_turret: {
      create_physics(r, world, e);
      r.emplace<TurretComponent>(e);
      break;
    }
    case EntityType::actor_bullet: {
      create_physics(r, world, e);
      r.emplace<EntityTimedLifecycle>(e);
      break;
    }
    case EntityType::spawner: {
      r.emplace<SpawnerComponent>(e);
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