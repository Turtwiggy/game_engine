#include "system.hpp"

#include "components.hpp"

#include "imgui/helpers.hpp"
#include "maths/maths.hpp"
#include "modules/lifecycle/components.hpp"
#include "physics/components.hpp"

#include "imgui.h"
#include "renderer/transform.hpp"
#include <glm/gtx/compatibility.hpp> // lerp

namespace game2d {

void
update_actor_bodypart_head_system(entt::registry& r, const float dt, const glm::ivec2& mouse_position)
{
  ImGui::Begin("HeadBob - Bob?! Where's my Head?");

  const auto& head_view = r.view<HeadComponent, const HasParentComponent, TransformComponent>();
  for (const auto& [e, head, p, t] : head_view.each()) {
    head.time += dt;

    const auto& parent = p.parent;
    const auto& parent_aabb = r.get<AABB>(parent);

    ImGui::Text("BodySize");
    auto& parent_transform = r.get<TransformComponent>(parent);
    int parent_scale_x = parent_transform.scale.x;
    int parent_scale_y = parent_transform.scale.y;
    imgui_draw_ivec2("bodysize", parent_scale_x, parent_scale_y);
    parent_transform.scale.x = parent_scale_x;
    parent_transform.scale.y = parent_scale_y;
    ImGui::Separator();

    const auto& parent_velocity = r.get<VelocityComponent>(parent);
    const auto normalized_vel = engine::normalize_safe({ parent_velocity.x, parent_velocity.y });
    ImGui::Text("NormalisedVel: %f %f", normalized_vel.x, normalized_vel.y);

    // root position to put the head at
    float root_y = parent_aabb.center.y - parent_aabb.size.y;
    float root_x = parent_aabb.center.x;

    // move the head?
    imgui_draw_float("offset_y", head.offset_y);
    root_y += head.offset_y;

    // bob the head up and down
    imgui_draw_float("headbob_amplitude", head.headbob_amplitude);
    imgui_draw_float("headbob_speed", head.headbob_speed);

    // always bob y regardless
    root_y += head.headbob_amplitude * cos(head.headbob_speed * head.time);

    t.position.x = root_x;
    t.position.y = root_y;
    // const float lerpspeed = 0.15f;
    // t.position.y = glm::lerp(float(t.position.y), root_y, lerpspeed);

    imgui_draw_float("head_size_x", head.head_size_x);
    imgui_draw_float("head_size_y", head.head_size_y);
    t.scale = { head.head_size_x, head.head_size_y, 1 }; // hack: head-size
  }

  ImGui::End();
}

} // namespace game2d
