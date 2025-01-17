#include "ship_draw_arcs_system.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/maths/maths.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/camera/orthographic.hpp"
#include "modules/colour/components.hpp"
#include "modules/renderer/components.hpp"
#include "ship_draw_arcs_components.hpp"

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

  const auto& view = r.view<const HasParentComponent, ShipArcComponent, const TransformComponent>();
  for (const auto& [e, parent_c, arc_c, weapon_t] : view.each()) {

    const auto p = parent_c.parent;
    if (p == entt::null || !r.valid(p)) {
      dead.dead.emplace(e); // kill this parentless entity (soz)
      continue;
    }

    auto eid = static_cast<uint32_t>(e);
    ImGui::PushID(eid);

    // note: in data format, 90degrees is up, 270 is down.
    // when load in, convert to engine, where 90 is down, 270 is up
    const auto angle_mid = arc_c.arc_mid;
    const auto arc = arc_c.arc;
    const auto half_arc_radians = (arc / 2.0f) * engine::Deg2Rad;
    const auto adj_arc_mid = engine::angle_degrees_flip_y_axis(angle_mid);
    const auto adj_arc_mid_rad = adj_arc_mid * engine::Deg2Rad;
    const auto tl_offset = glm::vec2{ arc_c.x_rel_tl, arc_c.y_rel_tl };

    // gunpoint base
    const auto& t_c = r.get<TransformComponent>(p);
    const float fwd = t_c.rotation_radians.z; // parents dir, could be gun dir?
    const auto pos = get_position(r, p);
    const auto size = glm::vec2{ t_c.scale.x, t_c.scale.y };
    const auto tl = pos - (0.5f * size);
    const auto rel_tl = (tl - pos) + tl_offset;
    const auto rotated_point = engine::rotate_point(rel_tl, fwd);
    const auto hardpoint_pos = pos + rotated_point;

    // direction gunpoint is facing
    const float angle = fwd + adj_arc_mid_rad;
    const float angle_l = fwd + adj_arc_mid_rad - half_arc_radians;
    const float angle_r = fwd + adj_arc_mid_rad + half_arc_radians;
    const auto dir = engine::angle_radians_to_direction(angle);
    const auto dir_l = engine::angle_radians_to_direction(angle_l);
    const auto dir_r = engine::angle_radians_to_direction(angle_r);
    arc_c.dir_arc_left = dir_l;
    arc_c.dir_arc_center = dir;
    arc_c.dir_arc_right = dir_r;

    // draw the hardpoint
    // Sprite tl_s;
    // tl_s.sprite = "EMPTY";
    // tl_s.pos = hardpoint_pos;
    // tl_s.size = { 5, 5 };
    // tl_s.z_rotation = 0;
    // tl_s.col = { 0.0f, 0.0f, 1.0f, 1.0f };
    // draw_sprite(r, tl_s);

    const auto screenspace = worldspace_to_screenspace(r, pos);

    float zone_radius = 30;
    zone_radius /= zoom;

    // the more guns, onionskin the debug
    entity_to_guncount[p] += 1;
    float radius = 30 + entity_to_guncount[p] * 10;
    radius /= zoom;

    // draw the xp-zone arc. this shouldnt be here.
    if (entity_to_guncount[p] == 1)
      DrawArc(screenspace, zone_radius, 0, 360, 3, ImColor(0.3f, 0.3f, 0.3f, 1.0f), true);

    const auto col = r.get<DefaultColour>(p).colour;
    const ImU32 im_col = IM_COL32(col.r, col.g, col.b, col.a);

    float center_angle_deg = engine::dir_to_angle_radians(dir) * engine::Rad2Deg;
    DrawArc(screenspace, radius, center_angle_deg, arc_c.arc, 2, im_col, true);

    // const auto draw_line = [&r](const LineInfo& l) {
    //   Sprite s;
    //   s.sprite = "EMPTY";
    //   s.pos = l.position;
    //   s.size = l.scale;
    //   s.z_rotation = l.rotation;
    //   s.col = engine::SRGBColour{ 1.0f, 1.0f, 1.0f, 1.0f };
    //   draw_sprite(r, s);
    // };

    // Draw lines to left and right
    // auto arc_mid_pos = pos + dir * 100.0f;
    // auto arc_l_pos = pos + dir_l * 100.0f;
    // auto arc_r_pos = pos + dir_r * 100.0f;
    // draw_line(generate_line(pos, arc_mid_pos, 4.0));
    // draw_line(generate_line(pos, arc_l_pos, 4.0f));
    // draw_line(generate_line(pos, arc_r_pos, 4.0f));

    // Draw the arc
    // const int segments = 32;
    // const float arc_angle_step = (arc * engine::Deg2Rad) / (float)segments;
    // glm::vec2 prev_vert = pos + radius * glm::vec2(cos(angle_l), sin(angle_l));
    // for (int i = 1; i <= segments; i++) {
    //   float a = angle_l + i * arc_angle_step;
    //   glm::vec2 cur_vert = pos + radius * glm::vec2{ cos(a), sin(a) };
    //   draw_line(generate_line(cur_vert, prev_vert, 5.0f));
    //   prev_vert = cur_vert;
    // }

    ImGui::PopID();
  }

  ImGui::End();
  ImGui::PopStyleVar(2);
}

} // namespace game2d