#include "actors/base.hpp"

#include "entt/helpers.hpp"
#include "modules/actors/helpers.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/helpers.hpp"
#include "modules/ui_colours/helpers.hpp"
#include "sprites/components.hpp"
#include "sprites/helpers.hpp"

#include "magic_enum.hpp"
#include <fmt/core.h>

namespace game2d {

std::string
sprite_type_to_sprite(const EntityType& type)
{
  const auto type_name = std::string(magic_enum::enum_name(type));

  std::string sprite = "EMPTY";

  if (type == EntityType::actor_breach_charge)
    sprite = "WEAPON_GRENADE";
  else if (type == EntityType::actor_jetpack_player)
    sprite = "PERSON_25_0";
  else if (type == EntityType::actor_space_ship)
    sprite = "SPACE_VEHICLE_1";
  else if (type == EntityType::actor_space_capsule)
    sprite = "EMPTY";
  else if (type == EntityType::actor_space_cargo)
    sprite = "DICE_DARK_X";
  else if (type == EntityType::weapon_shotgun)
    sprite = "EMPTY";
  else if (type == EntityType::bullet_default)
    sprite = "EMPTY";
  else if (type == EntityType::bullet_bouncy)
    sprite = "EMPTY";
  else if (type == EntityType::particle)
    sprite = "EMPTY";
  else if (type == EntityType::solid_wall)
    sprite = "EMPTY";
  // else
  //   fmt::println("warning! sprite set to empty: {}", type_name);

  return sprite;
};

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

entt::entity
Factory_BaseActor::create(entt::registry& r, const EntityType& type, const EntityDescription& desc)
{
  const auto type_name = std::string(magic_enum::enum_name(type));
  const auto e = r.create();

  r.emplace<TagComponent>(e, type_name);
  r.emplace<EntityTypeComponent>(e, type);
  r.emplace<WaitForInitComponent>(e);

  auto sprite = sprite_type_to_sprite(type);
  auto sc = create_sprite(r, sprite, type);
  r.emplace<SpriteComponent>(e, sc);

  TransformComponent tf;
  tf.position = { desc.pos.x, desc.pos.y, 0.0f };
  tf.scale = { desc.size.x, desc.size.y, 0.0f };
  r.emplace<TransformComponent>(e, tf);

  set_position(r, e, { tf.position.x, tf.position.y });
  set_colour(r, e, desc.colour);
  return e;
};

} // namespace game2d