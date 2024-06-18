#include "actors.hpp"

#include "colour/colour.hpp"
#include "entt/helpers.hpp"
#include "modules/actor_cursor/components.hpp"
#include "modules/actors/helpers.hpp"
#include "modules/lighting/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/helpers.hpp"
#include "physics/components.hpp"
#include "sprites/components.hpp"
#include "sprites/helpers.hpp"

#include "magic_enum.hpp"

namespace game2d {

SpriteComponent
create_sprite(entt::registry& r, const EntityType& type)
{
  const auto type_name = std::string(magic_enum::enum_name(type));
  const auto& anims = get_first_component<SINGLE_Animations>(r);
  const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);

  std::string sprite = "EMPTY";

  if (type == EntityType::actor_player)
    sprite = "PERSON_25_0";

  // else
  // std::cerr << "warning! sprite not implemented: " << type_name << "\n";

  const auto& colours = get_first_component<SINGLETON_ColoursComponent>(r);
  const auto& primary = colours.lin_primary;
  const auto& secondary = colours.lin_secondary;
  const auto& tertiary = colours.lin_tertiary;
  const auto& quaternary = colours.lin_quaternary;

  SpriteComponent sc;
  sc.colour = *primary;

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

  const int SPRITE_SIZE = 16;
  const glm::ivec2 DEFAULT_SIZE{ 16, 16 };
  const glm::ivec2 HALF_SIZE{ 8, 8 };

  const auto type_name = std::string(magic_enum::enum_name(type));

  const auto& e = r.create();
  r.emplace<TagComponent>(e, type_name);
  r.emplace<EntityTypeComponent>(e, type);

  r.emplace<SpriteComponent>(e, create_sprite(r, type));
  r.emplace<TransformComponent>(e);
  auto& transform = r.get<TransformComponent>(e);
  transform.scale.x = SPRITE_SIZE;
  transform.scale.y = SPRITE_SIZE;

  switch (type) {

    case EntityType::actor_cursor: {
      r.emplace<CursorComponent>(e);
      create_physics_actor(r, e);
      break;
    }

    case EntityType::actor_player: {
      create_physics_actor(r, e);
      r.emplace<LightEmitterComponent>(e);
      break;
    }

    case EntityType::solid_wall: {
      create_physics_actor(r, e);
      set_size(r, e, DEFAULT_SIZE);
      r.emplace<PhysicsSolidComponent>(e);
      r.emplace<LightOccluderComponent>(e);
      break;
    }

    default: {
      std::cout << "warning: no gameplay implemented for: " << type_name << std::endl;
    }
  } // end switch

  return e;
};

} // namespace game2d