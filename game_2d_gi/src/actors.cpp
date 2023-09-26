#include "actors.hpp"

#include "colour/colour.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "modules/camera/orthographic.hpp"
#include "modules/lifecycle/components.hpp"
#include "modules/lighting/components.hpp"
#include "modules/renderer/components.hpp"
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

  std::string sprite = "EMPTY";

  if (type == EntityType::actor_player)
    sprite = "PERSON_25_0";

  // else
  // std::cerr << "warning! sprite not implemented: " << type_name << "\n";

  SpriteComponent sc;

  auto& anims = get_first_component<SINGLETON_Animations>(r);
  const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);

  // search spritesheet
  const auto anim = find_animation(anims.animations, sprite);
  sc.x = anim.animation_frames[0].x;
  sc.y = anim.animation_frames[0].y;

  if (anim.angle_degrees != 0.0f)
    sc.angle_radians = glm::radians(anim.angle_degrees);

  // sc.tex_unit = ri.tex_un
  sc.tex_unit = 0;

  // sc.tex_unit = ri.tex_unit_kennynl;
  // if (sc.tex_unit == ri.tex_unit_kennynl)
  {
    sc.sx = 48;
    sc.sy = 22;
  }
  // else
  //   std::cerr << "warning! spritesize not implemented for: " << type_name << "\n";

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

    case EntityType::actor_player: {
      create_physics_actor(r, e);
      // r.emplace<PhysicsSolidComponent>(e);
      r.emplace<VelocityComponent>(e);

      r.emplace<LightEmitterComponent>(e);

      break;
    }

      //
      // solids
      //

    case EntityType::solid_wall: {
      r.emplace<PhysicsTransformXComponent>(e);
      r.emplace<PhysicsTransformYComponent>(e);
      r.emplace<AABB>(e);
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