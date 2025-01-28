#include "modules/effects_parallax_mouse/parallax_mouse_system.hpp"

#include "modules/effects_parallax_mouse/parallax_mouse_components.hpp"

#include "engine/imgui/helpers.hpp"
#include "engine/maths/maths.hpp"
#include "engine/physics/physics_components.hpp"
#include "engine/renderer/transform.hpp"

#include "imgui.h"

namespace game2d {

void
update_parallax_mouse_system(entt::registry& r, const float dt, const glm::ivec2 mouse_pos)
{
  const auto& view = r.view<const TransformComponent, ParallaxMouseComponent, const PhysicsBodyComponent>();
  for (const auto& [e, t_c, parallax_c, body_c] : view.each()) {

    const auto diff = glm::vec2{ t_c.position.x, t_c.position.y } - glm::vec2(mouse_pos);
    const auto distance = engine::distance_squared({ diff.x, diff.y, 0.0f });

    static float velocity_x = 0.0f;
    static float velocity_y = 0.0f;
    velocity_x = body_c.body->GetLinearVelocity().x;
    velocity_y = body_c.body->GetLinearVelocity().y;
    // ImGui::Text("vel: %f %f", velocity_x, velocity_y);
    const glm::vec2 vel_impulse = { -velocity_x / 10.0f, -velocity_y / 10.0f };

    glm::vec2 mouse_impulse{ 0, 0 };

    const bool in_x = glm::abs(diff.x) <= t_c.scale.x / 2.0f;
    const bool in_y = glm::abs(diff.y) <= t_c.scale.y / 2.0f;
    const bool contains = in_x && in_y;
    if (contains) {
      mouse_impulse.x = diff.x / (t_c.scale.x / 2.0f);
      mouse_impulse.y = diff.y / (t_c.scale.y / 2.0f);
    }

    // parallax: affected by velocity and mouse position
    static float max_rot_x = 0.7f;
    static float max_rot_y = 0.7f;
#if defined(_DEBUG)
    imgui_draw_float("max_rot_x", max_rot_x);
    imgui_draw_float("max_rot_y", max_rot_x);
#endif
    float tgt_x_rot = max_rot_x * -glm::clamp(vel_impulse.x + mouse_impulse.x, -1.0f, 1.0f);
    float tgt_y_rot = max_rot_y * glm::clamp(vel_impulse.y + mouse_impulse.y, -1.0f, 1.0f);

    const float speed = 10.0f;
    parallax_c.rotation.x = engine::lerp(parallax_c.rotation.x, tgt_x_rot, speed * dt);
    parallax_c.rotation.y = engine::lerp(parallax_c.rotation.y, tgt_y_rot, speed * dt);

    ImGui::Text("diff: %f %f, rot: %f %f", diff.x, diff.y, parallax_c.rotation.x, parallax_c.rotation.y);

    //
  }
}

} // namespace game2d