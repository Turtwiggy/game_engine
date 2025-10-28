#include "pch.hpp"

#include "engine/sprites/components.hpp"
#include "engine/sprites/helpers.hpp"
#include "modules/core/raws/raws_components.hpp"
#include "modules/core/renderer/helpers.hpp"
#include "modules/systems/system_spritestack/spritestack_components.hpp"
#include "spritestack_helpers.hpp"

namespace game2d {
using namespace std::literals;

bool
add_spritestack(entt::registry& r, entt::entity e, std::string sprite)
{
  const std::vector<std::string> supported_spritestacks{
    "dinghy",
    "rhib",
    "pbr",
  };

  // i.e. which layer makes mose sense to have as the hitbox?
  // note: ignoring {0, 0}. so if dinghy_1 is frame {0, 1} = 0,
  // the int value 1 in this vector represents 1 frame after that.
  const std::vector<int> spritestack_base_layer{
    1,
    9,
    26,
  };

  auto it = std::find(supported_spritestacks.begin(), supported_spritestacks.end(), sprite);
  if (it == supported_spritestacks.end())
    return false; // oops! spritestack not implemented
  const auto idx = static_cast<int>(it - supported_spritestacks.begin());

  const auto& anims = SINGLE_Animations::instance;
  const auto [spritesheet, anim] = find_animation(anims, sprite + "_1"s);
  const int sprites_for_total_sprite = spritesheet.ny - 1; // note: -1 because {0, 0} should be empty

  entt::entity root_entity = entt::null;
  glm::vec2 pos{ 0, 0 };

  // iterate from e.g. [-26, 12] for a ydepth of 38, where the center is 26 now
  const int root_spritestack_img_idx = -spritestack_base_layer[idx];
  const int max = sprites_for_total_sprite + root_spritestack_img_idx;

  // iterate through the spritestack frames
  // note: {0, 0} is an empty frame, so start the counter at 1.
  int counter = 1;

  for (int i = root_spritestack_img_idx; i < max; i++) {
    const auto i_as_str = std::to_string(counter++);
    const auto tag_str = sprite + "_"s + i_as_str;

    entt::entity spawned_e = entt::null;

    // this sets the SpriteComponent on the player
    if (i == 0)
      spawned_e = e;

    else
      spawned_e = create_transform(r, tag_str);

    // needs to be emplaced in order to maintain spritestack
    r.emplace<SpriteComponent>(spawned_e);

    set_sprite(r, spawned_e, sprite + "_"s + i_as_str);
    set_z_index(r, spawned_e, ZLayer::DEFAULT);

    //
    // i goes from e.g. [-26, 12] on a 38 ydepth.
    // that works pretty well for z-index, where 0 represents "default"
    // however, z-index is sorted a.z_idx < b.z_idx,
    // but here i represents a sprite index, where -26 is the top sprite, not bottom.
    // hence, flip it, yo
    //
    // e.g. -26 should be 26
    // e.g. 0 should be 0
    // e.g. 12 should be -12
    //
    const int flipped_i = -1 * i;
    auto& t_c = r.get<TransformComponent>(spawned_e);
    t_c.z_index = flipped_i;

    SpritestackComponent spritestack_c(i);
    spritestack_c.spritestack_total = sprites_for_total_sprite;
    spritestack_c.root = e;
    spritestack_c.tag = tag_str;
    r.emplace<SpritestackComponent>(spawned_e, spritestack_c);
  }

  return true;
};

} // namespace game2d