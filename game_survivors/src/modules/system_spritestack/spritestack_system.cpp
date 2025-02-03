#include "spritestack_system.hpp"

#include "engine/sprites/helpers.hpp"
#include "spritestack_components.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/imgui/helpers.hpp"
#include "engine/renderer/transform.hpp"
#include "engine/sprites/components.hpp"
#include "modules/camera/orthographic.hpp"

#include <entt/entt.hpp>
#include <imgui.h>

namespace game2d {

void
update_sprite_spritestack_system(entt::registry& r, const float dt)
{
  // TODO: replace with spritesheet info

  // note: values of 1 seem to distort the spritestack, but add depth
  // because it SHOULD be in top-down perspective
  // values of 0 are just top down perspective,
  // so a value of 0-1 is recommended between these two.
  static float scale_up_by = 0.6f;
  static int sprite_height = 1;

  static int parallax_offset_amount = 0;

  ImGui::Begin("DebugSpriteStack");
  imgui_draw_float("scale", scale_up_by);
  imgui_draw_int("sprite_height", sprite_height);
  imgui_draw_int("parallax_offset_amount", parallax_offset_amount);

  const auto camera_e = get_first<OrthographicCamera>(r);
  const auto camera_pos = get_position(r, camera_e);

  const auto& view = r.view<TransformComponent, SpritestackComponent, SpriteComponent, TagComponent>();
  for (const auto& [e, t, ssc, sprite, tag_c] : view.each()) {
    const auto idx = ssc.spritestack_index;

    const auto& anims = get_first_component<SINGLE_Animations>(r);
    const auto [spritesheet, anim] = find_animation(anims, tag_c.tag);

    const int sprite_scale_x = spritesheet.px;
    const int sprite_scale_y = spritesheet.py;

    // Set position for each child sprite
    if (ssc.root != entt::null) {
      const auto sprite_parent = ssc.root;
      const auto& sprite_parent_transform = r.get<TransformComponent>(sprite_parent);
      // set position to parents position
      t.position = sprite_parent_transform.position;
      // set rotation to parents rotation
      t.rotation_radians.z = sprite_parent_transform.rotation_radians.z;
    }

    t.position.y += scale_up_by * (sprite_height * idx);

    t.scale = { sprite_scale_x, sprite_scale_y, 1.0f };

    // adjust colour
    // float percent = ((ssc.spritestack_total - 1) - ssc.spritestack_index);
    // sprite.colour.a = (ssc.spritestack_index + 1.0f) / float(ssc.spritestack_total);

    // parallax effect
    // const glm::vec2 dir_raw = camera_pos - glm::vec2{ t.position.x, t.position.y };
    // const glm::vec2 dir_nrm = engine::normalize_safe(dir_raw);
    // const glm::vec2 offset = { -dir_nrm.x * idx * parallax_offset_amount, -dir_nrm.y * idx * parallax_offset_amount };
    // t.position.x += offset.x;
    // t.position.y += offset.y;
  }

  ImGui::End();
}

} // namespace game2d