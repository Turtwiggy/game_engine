#include "actors/base.hpp"

#include "actors/helpers.hpp"
#include "colour/colour.hpp"
#include "entt/helpers.hpp"
#include "lifecycle/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/helpers.hpp"
#include "renderer/components.hpp"
#include "sprites/components.hpp"
#include "sprites/helpers.hpp"

namespace game2d {

SpriteComponent
create_sprite(entt::registry& r, const EntityData& desc)
{
  const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);

  SpriteComponent sc;
  sc.colour = engine::SRGBToLinear(desc.colour);

  // search spritesheet
  const auto& anims = get_first_component<SINGLE_Animations>(r);
  const auto [spritesheet, anim] = find_animation(anims, desc.sprite);

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
Factory_BaseActor::create(entt::registry& r, const EntityData& desc, const std::string& tag)
{
  const auto e = r.create();

  r.emplace<TagComponent>(e, cleanup_tag_str(tag));
  r.emplace<WaitForInitComponent>(e);
  r.emplace<SpriteComponent>(e, create_sprite(r, desc));

  TransformComponent tf;
  tf.position = { desc.pos.x, desc.pos.y, 0.0f };
  tf.scale = { desc.size.x, desc.size.y, 0.0f };
  r.emplace<TransformComponent>(e, tf);

  set_position(r, e, { tf.position.x, tf.position.y });
  set_colour(r, e, desc.colour);
  return e;
};

} // namespace game2d