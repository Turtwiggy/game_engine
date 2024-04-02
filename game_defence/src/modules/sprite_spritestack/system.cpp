#include "system.hpp"

#include "components.hpp"
#include "entt/helpers.hpp"
#include "imgui/helpers.hpp"
#include "lifecycle/components.hpp"
#include "maths/maths.hpp"
#include "modules/actors/helpers.hpp"
#include "modules/camera/orthographic.hpp"
#include "modules/lerp_to_target/components.hpp"
#include "physics/components.hpp"
#include "renderer/transform.hpp"
#include "sprites/components.hpp"


#include <entt/entt.hpp>
#include <glm/gtx/compatibility.hpp> // lerp
#include <imgui.h>

namespace game2d {

void
update_sprite_spritestack_system(entt::registry& r, const float dt)
{
  static float scale_up_by = 1.0f;
  static int sprite_height = -6;
  static int sprite_scale_x = 64;
  static int sprite_scale_y = 64;
  static int parallax_offset_amount = 6;

  ImGui::Begin("DebugSpriteStack");
  imgui_draw_float("scale", scale_up_by);
  imgui_draw_int("sprite_height", sprite_height);
  imgui_draw_int("sprite_scale_x", sprite_scale_x);
  imgui_draw_int("sprite_scale_y", sprite_scale_y);
  imgui_draw_int("parallax_offset_amount", parallax_offset_amount);

  const auto camera_e = get_first<OrthographicCamera>(r);
  const auto camera_pos = get_position(r, camera_e);

  const auto& view = r.view<TransformComponent, SpritestackComponent, SpriteComponent>();
  for (const auto& [e, t, ssc, sprite] : view.each()) {
    const auto idx = ssc.spritestack_index;

    // Set position for each child sprite
    if (ssc.root != entt::null) {
      const auto sprite_parent = ssc.root;
      const auto& sprite_parent_transform = r.get<TransformComponent>(sprite_parent);
      // set position to parents position
      t.position = sprite_parent_transform.position;
      // set rotation to parents rotation
      t.rotation_radians.z = sprite_parent_transform.rotation_radians.z - engine::HALF_PI;
    }

    t.position.y += int(scale_up_by) * (sprite_height * idx);

    t.scale = { sprite_scale_x, sprite_scale_y, 1.0f };

    // adjust colour
    // float percent = ((ssc.spritestack_total - 1) - ssc.spritestack_index);
    sprite.colour.a = (ssc.spritestack_index + 1.0f) / float(ssc.spritestack_total);

    // parallax effect
    const glm::vec2 dir_raw = camera_pos - glm::vec2{ t.position.x, t.position.y };
    const glm::vec2 dir_nrm = engine::normalize_safe(dir_raw);
    const glm::vec2 offset = { -dir_nrm.x * idx * parallax_offset_amount, -dir_nrm.y * idx * parallax_offset_amount };
    t.position.x += offset.x;
    t.position.y += offset.y;
  }

  ImGui::End();
}

} // namespace game2d