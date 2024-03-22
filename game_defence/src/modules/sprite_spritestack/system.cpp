#include "system.hpp"

#include "components.hpp"
#include "imgui/helpers.hpp"
#include "maths/maths.hpp"
#include "modules/lerp_to_target/components.hpp"
#include "modules/lifecycle/components.hpp"
#include "physics/components.hpp"
#include "renderer/transform.hpp"

#include <entt/entt.hpp>
#include <glm/gtx/compatibility.hpp> // lerp
#include <imgui.h>

namespace game2d {

void
update_sprite_spritestack_system(entt::registry& r, const float dt)
{
  static float scale_up_by = 2.0f;
  static int sprite_height = 1;
  static int center_y_offset = 0;

  static bool first_time = true;

  ImGui::Begin("DebugSpriteStack");
  imgui_draw_float("scale", scale_up_by);
  imgui_draw_int("sprite_height", sprite_height);
  imgui_draw_int("center_y_offset", center_y_offset);

  const auto& view = r.view<TransformComponent, SpritestackComponent, HasTargetPositionComponent>();
  for (const auto& [e, t, ssc, target] : view.each()) {

    auto* sprite_parent_maybe = r.try_get<HasParentComponent>(e);
    if (sprite_parent_maybe) {
      const auto& sprite_parent = sprite_parent_maybe->parent;
      const auto& sprite_parent_pos = r.get<AABB>(sprite_parent);
      const auto& sprite_parent_transform = r.get<TransformComponent>(sprite_parent);

      // set position to parents position
      const glm::vec3 parent_pos = { sprite_parent_pos.center.x, sprite_parent_pos.center.y, 0 };
      t.position = parent_pos;

      // set rotation to parents rotation
      t.rotation_radians.z = sprite_parent_transform.rotation_radians.z - engine::HALF_PI;
    } else {

      // Set root position for each sprite
      t.position.x = target.position.x;
      t.position.y = target.position.y;

      // just spin
      t.rotation_radians.z += dt;
    }

    // const float lerpspeed = 0.15f;
    // const glm::vec3 current_pos = t.position;
    // t.position = glm::lerp(current_pos, parent_pos, lerpspeed);

    t.position.y += scale_up_by * (sprite_height * ssc.spritestack_index);

    // TODO: work out why this is the middle index
    // middle-index is the middle-layer, or the visually center layer?

    // set first frame, but then every frame after let ui control it
    if (first_time) {
      const int middle_index = 5;
      center_y_offset = -1 * scale_up_by * middle_index;
      first_time = false;
    }
    t.position.y += center_y_offset;

    // TODO: work out how to get the 5 and 16 from the spritesheet info
    t.scale.x = scale_up_by * 5;
    t.scale.y = scale_up_by * 16;
  }

  ImGui::End();
}

} // namespace game2d