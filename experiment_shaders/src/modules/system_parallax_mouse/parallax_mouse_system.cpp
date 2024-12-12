#include "modules/system_parallax_mouse/parallax_mouse_system.hpp"

#include "engine/maths/maths.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/system_parallax_mouse/parallax_mouse_components.hpp"

#include "imgui.h"

namespace game2d {

void
update_parallax_mouse_system(entt::registry& r, const float dt, const glm::ivec2 mouse_pos)
{
  const auto& view = r.view<TransformComponent, ParallaxMouseComponent>();
  for (const auto& [e, t_c, parallax_c] : view.each()) {

    const glm::vec2 diff = glm::vec2{ t_c.position.x, t_c.position.y } - glm::vec2(mouse_pos);

    const bool in_x = glm::abs(diff.x) <= t_c.scale.x / 2.0f;
    const bool in_y = glm::abs(diff.y) <= t_c.scale.y / 2.0f;
    const bool contains = in_x && in_y;

    float tgt_x = 0.0f;
    float tgt_y = 0.0f;

    // if (contains) {

    const float max_rot_x = 3.0f;
    const float max_rot_y = 3.0f;

    float x_rot = max_rot_x * -glm::clamp(diff.y / (t_c.scale.y / 2.0f), -1.0f, 1.0f);
    float y_rot = max_rot_y * glm::clamp(diff.x / (t_c.scale.x / 2.0f), -1.0f, 1.0f);
    ImGui::Text("diff: %f %f, rot: %f %f", diff.x, diff.y, x_rot, y_rot);

    tgt_x = glm::radians(x_rot);
    tgt_y = glm::radians(y_rot);
    // }

    const float speed = 10.0f;
    t_c.rotation_radians.x = engine::lerp(t_c.rotation_radians.x, tgt_x, speed * dt);
    t_c.rotation_radians.y = engine::lerp(t_c.rotation_radians.y, tgt_y, speed * dt);

    //
  }
}

} // namespace game2d