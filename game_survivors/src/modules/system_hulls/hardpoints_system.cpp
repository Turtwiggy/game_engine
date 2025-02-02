#include "hardpoints_system.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/colour/colour.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/maths/maths.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/camera/orthographic.hpp"
#include "modules/colour/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/system_autofire/autofire_components.hpp"
#include "modules/system_hulls/hulls_components.hpp"

#include <imgui.h>

namespace game2d {

glm::vec2
worldspace_to_screenspace(entt::registry& r, glm::vec2 worldspace)
{
  const auto& ri = get_first_component<SINGLE_RendererInfo>(r);
  const auto camera_e = get_first<OrthographicCamera>(r);
  const auto& camera_c = r.get<OrthographicCamera>(camera_e);

  const auto camera_pos = get_position(r, camera_e);
  const auto screen_size = glm::vec2{ (float)ri.viewport_size_render_at.x, (float)ri.viewport_size_render_at.y };
  // const auto screen_size = ImGui::GetContentRegionAvail();
  // const auto avail = ImGui::GetContentRegionAvail();
  // const auto avail_max = ImGui::GetContentRegionMax();

  // 1.4 is zoom in
  // 1 is no zoom
  // 0.7 is zoomout
  auto camera_pos_space = worldspace - camera_pos;
  camera_pos_space.x /= camera_c.zoom_nonlinear;
  camera_pos_space.y /= camera_c.zoom_nonlinear;

  return camera_pos_space + (0.5f * glm::vec2{ screen_size.x, screen_size.y });
};

void
DrawArc(const glm::vec2& screenspace_pos,
        float radius,
        float center_angle_deg,
        float arc_angle_deg,
        float thickness,
        ImU32 color,
        bool rounded = true)
{
  const ImVec2 center = ImGui::GetCursorScreenPos();
  const ImVec2 pos = { center.x + screenspace_pos.x, center.y + screenspace_pos.y };

  // ImVec2 pos = { center.x + (avail.x / 2.0f), center.y + (avail.y / 2.0f) };
  // pos.y -= radius / 2.0f; // move circle to center
  // ImVec2 pos = { screenspace_pos.x, screenspace_pos.y };

  ImDrawList* draw_list = ImGui::GetWindowDrawList();

  const float center_angle_rad = center_angle_deg * engine::Deg2Rad;
  const float arc_angle_rad = arc_angle_deg * engine::Deg2Rad;
  const float half_arc_angle_rad = arc_angle_rad / 2.0f;
  const float angle_start_rad = center_angle_rad - half_arc_angle_rad;
  const float angle_end_rad = center_angle_rad + half_arc_angle_rad;

  // debug center where imgui
  // ImVec2 max = ImVec2(pos.x + 5.0f, pos.y + 5.0f);
  // draw_list->AddRectFilled(pos, max, ImColor(1.0f, 0.0f, 0.0f, 1.0f));

  draw_list->PathArcTo(pos, radius, angle_start_rad, angle_end_rad);
  draw_list->PathStroke(color, false, thickness);

  // if (rounded) {
  //   const ImVec2 start_point = ImVec2(center.x + cosf(angle_start_rad) * radius, center.y + sinf(angle_start_rad) *
  //   radius); const ImVec2 end_point = ImVec2(center.x + cosf(angle_end_rad) * radius, center.y + sinf(angle_end_rad) *
  //   radius);
  //   draw_list->AddCircleFilled(start_point, thickness * 0.5f, color);
  //   draw_list->AddCircleFilled(end_point, thickness * 0.5f, color);
  // }
}

void
update_ship_draw_arcs_system(entt::registry& r)
{
  auto& dead = get_first_component<SINGLE_EntityBinComponent>(r);

  const auto& ri = get_first_component<SINGLE_RendererInfo>(r);
  const auto camera_e = get_first<OrthographicCamera>(r);
  const auto& camera_c = r.get<OrthographicCamera>(camera_e);
  const auto zoom = camera_c.zoom_nonlinear;

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoDecoration;
  flags |= ImGuiWindowFlags_NoMove;
  flags |= ImGuiWindowFlags_NoBackground;
  flags |= ImGuiWindowFlags_NoFocusOnAppearing;
  flags |= ImGuiWindowFlags_NoDocking;
  flags |= ImGuiWindowFlags_NoInputs;
  flags |= ImGuiWindowFlags_NoSavedSettings;

  std::unordered_map<entt::entity, int> entity_to_guncount;

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

  const auto screen_size = ImVec2{ (float)ri.viewport_size_render_at.x, (float)ri.viewport_size_render_at.y };
  ImGui::SetNextWindowPos({ 0, 0 }, ImGuiCond_Always, { 0.0f, 0.0f });
  ImGui::SetNextWindowSize(screen_size, ImGuiCond_Always);

  ImGui::Begin("cursors_ui", NULL, flags);

  const auto& view =
    r.view<const HasParentComponent, HardpointComponent, const TransformComponent, const AutofireComponent>();
  for (const auto& [e, parent_c, hardpoint_c, weapon_t, autofire_c] : view.each()) {

    const auto p = parent_c.parent;
    if (p == entt::null || !r.valid(p)) {
      dead.dead.emplace(e); // kill this parentless entity (soz)
      continue;
    }

    auto eid = static_cast<uint32_t>(e);
    ImGui::PushID(eid);

    const auto& t_c = r.get<TransformComponent>(p);
    const auto pos = get_position(r, p);

    // note: in data format, 90degrees is up, 270 is down.
    // when load in, convert to engine, where 90 is down, 270 is up
    const auto mid = hardpoint_c.data.arc_mid;
    const auto arc = hardpoint_c.data.arc;
    const auto half_arc_radians = (arc / 2.0f) * engine::Deg2Rad;
    const auto adj_arc_mid = engine::angle_degrees_flip_y_axis(mid);
    const auto adj_arc_mid_rad = adj_arc_mid * engine::Deg2Rad;
    const auto tl_offset = glm::vec2{ hardpoint_c.data.x_rel_tl, hardpoint_c.data.y_rel_tl };

    // gunpoint base
    const float fwd = t_c.rotation_radians.z; // parents dir, could be gun dir?
    const auto size = glm::vec2{ t_c.scale.x, t_c.scale.y };
    const auto tl = pos - (0.5f * size);
    const auto rel_tl = (tl - pos) + tl_offset;
    const auto rotated_point = engine::rotate_point(rel_tl, fwd);
    const auto hardpoint_pos = pos + rotated_point;

    // direction gunpoint is facing
    const float angle = fwd + adj_arc_mid_rad;
    const float angle_l = fwd + adj_arc_mid_rad - half_arc_radians;
    const float angle_r = fwd + adj_arc_mid_rad + half_arc_radians;
    const auto dir_l = engine::angle_radians_to_direction(angle_l);
    const auto dir = engine::angle_radians_to_direction(angle);
    const auto dir_r = engine::angle_radians_to_direction(angle_r);
    hardpoint_c.dir_arc_left = dir_l;
    hardpoint_c.dir_arc_center = dir;
    hardpoint_c.dir_arc_right = dir_r;

    if (arc >= 360) {
      const float epsilon = 0.001f;
      hardpoint_c.dir_arc_left = engine::angle_radians_to_direction(angle + epsilon);
      hardpoint_c.dir_arc_center = engine::angle_radians_to_direction(angle);
      hardpoint_c.dir_arc_right = engine::angle_radians_to_direction(angle - epsilon);
    }

    // the more guns, onionskin the debug
    entity_to_guncount[p] += 1;

    // draw the xp-zone arc. this shouldnt be here.
    float zone_radius = (50) / zoom;
    const auto screenspace = worldspace_to_screenspace(r, pos);
    auto col = r.get<DefaultColour>(p).colour;
    col.a = (int)(0.12f * 255);
    const ImU32 im_col = IM_COL32(col.r, col.g, col.b, col.a);
    // auto grey = ImColor(0.3f, 0.3f, 0.3f, 1.0f);
    DrawArc(screenspace, zone_radius, 0, 360, 2, im_col, true);

    // draw the gun arc.
    float thickness = 2;
    // float radius = (50 + entity_to_guncount[p] * 2) / zoom;
    float radius = (50 + 2) / zoom;
    auto arc_col = r.get<DefaultColour>(p).colour;
    col.a = (int)(1.0f * 255);
    const ImU32 arc_im_col = IM_COL32(arc_col.r, arc_col.g, arc_col.b, arc_col.a);
    float center_angle_deg = engine::dir_to_angle_radians(dir) * engine::Rad2Deg;
    DrawArc(screenspace, radius, center_angle_deg, arc, thickness, arc_im_col, true);

    // draw the arc where the gun cant shoot.
    // float thickness = 0.5;
    // float radius = (50 + entity_to_guncount[p] * 2) / zoom;
    // const auto col = engine::SRGBColour(1.0f, 0.0f, 0.0f, 1.0f);
    // const ImU32 im_col = IM_COL32(col.r, col.g, col.b, col.a);
    // float center_angle_deg = engine::dir_to_angle_radians(dir) * engine::Rad2Deg;
    // float opposite_angle = center_angle_deg + 180;
    // opposite_angle >= 360 ? opposite_angle -= 360 : opposite_angle;
    // DrawArc(screenspace, radius, opposite_angle, 360 - arc, thickness, im_col, true);

    ImGui::PopID();
  }

  ImGui::End();
  ImGui::PopStyleVar(2);
}

} // namespace game2d