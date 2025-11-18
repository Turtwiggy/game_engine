#include "pch.hpp"

#include "ui_label_system.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/imgui/ui_imgui_defaults.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/maths/grid.hpp"
#include "engine/maths/maths.hpp"
#include "engine/renderer/transform.hpp"
#include "engine/sprites/helpers.hpp"
#include "modules/actors/actor_island_cannon/island_cannon_components.hpp"
#include "modules/actors/actor_rock/rock_components.hpp"
#include "modules/combat/combat_core/components.hpp"
#include "modules/combat/combat_weapon_core/combat_weapon_core_components.hpp"
#include "modules/core/camera/helpers.hpp"
#include "modules/core/fonts/fonts_helpers.hpp"
#include "modules/core/renderer/components.hpp"
#include "modules/core/renderer/helpers.hpp"
#include "modules/core/ui/ui_draw_text_helpers.hpp"
#include "modules/systems/system_hardpoint_arcs/hulls_components.hpp"
#include "modules/systems/system_island_movement/island_movement_components.hpp"
#include "modules/systems/system_island_revive/island_revive_components.hpp"
#include "modules/ui/ui_scene_survive_upgrade/ui_survive_upgrade_components.hpp"
#include "resources/data.hpp"

namespace game2d {

void
draw_dashed_line(ImDrawList* draw_list, const ImVec2 p0, const ImVec2 p1, int n_dashes, const engine::SRGBColour& scol)
{
  const float width = 2.0f;
  const glm::vec2 raw_dir = glm::vec2{ p1.x, p1.y } - glm::vec2{ p0.x, p0.y };
  const glm::vec2 nrm_dir = engine::normalize_safe(raw_dir);

  // just draw a solid line
  if (n_dashes == 1) {
    const auto col = IM_COL32(scol.r, scol.g, scol.b, 255);
    draw_list->AddLine(p0, p1, col, width);
    return;
  }

  // calculate the length of the canvas
  const float length = glm::length(raw_dir);
  const float total_dash_length = length / n_dashes;
  const float dash_length = total_dash_length * 0.5f; // 0.5 = half gaps, half dash
  const float gap_length = total_dash_length - dash_length;

  // draw each dash
  glm::vec2 start = { p0.x, p0.y };
  for (int i = 0; i < n_dashes; i++) {
    const glm::vec2 end = start + nrm_dir * dash_length;

    // fade from 0 to 1 alpha
    const float percent = (i / float(n_dashes));
    const auto col = IM_COL32(scol.r, scol.g, scol.b, percent * 255);
    draw_list->AddLine({ start.x, start.y }, { end.x, end.y }, col, width);

    start = end + nrm_dir * gap_length;
  }
}

struct EdgeInfoOut
{
  std::vector<glm::ivec2> water_edges;
  std::vector<glm::ivec2> land_edges;
};
void
calculate_edges(entt::registry& r, entt::entity e, IslandLineInfo& cannon_c)
{
  if (cannon_c.initialized_edges)
    return;

  const auto tilesize = SINGLE_Islands::instance.tilesize;
  const auto& transform_c = r.get<TransformComponent>(e);
  const auto pos = glm::vec2{ transform_c.position.x, transform_c.position.y };
  const auto pos_adj = pos - glm::vec2{ tilesize * 0.5f, tilesize * 0.5f };
  const auto gp = engine::grid::worldspace_to_gridspace(pos_adj, tilesize);
  const auto id = engine::encode_cantor_pairing_function(gp.x, gp.y);
  const auto island_e = SINGLE_Islands::instance.id_to_island_eid.at(id);
  const auto& island_c = r.get<DebugContoursComponent>(island_e);

  // work out which way the edge points.
  std::vector<std::pair<engine::grid::GridDirection, glm::ivec2>> water_neighbours;
  std::vector<std::pair<engine::grid::GridDirection, glm::ivec2>> island_neighbours;
  {
    const auto neighbours = engine::grid::get_neighbour_gridpos(gp);
    const auto& all = island_c.all_island_xy;
    for (const auto& [n_dir, n_xy] : neighbours) {
      const auto it = std::find(all.begin(), all.end(), n_xy);
      if (it != all.end())
        island_neighbours.push_back({ n_dir, n_xy });
      else
        water_neighbours.push_back({ n_dir, n_xy });
    }
    // the cannon should've been spawned on the edge of an island (i.e. 3 neighbours, 1 nothing.)
    if (water_neighbours.size() == 0)
      throw std::runtime_error("all neighbours are land?");
  }

  cannon_c.water_edges = water_neighbours;
  cannon_c.land_edges = island_neighbours;
};

void
update_ui_label_system(entt::registry& r)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif

  // dont show labels if players are upgrading.
  {
    const auto view = r.view<UpgradeResultsComponent>();
    if (view.size() > 0)
      return;
  }

  // wait for islands to be initialised.
  {
    if (SINGLE_Islands::instance.id_to_island_eid.empty())
      return;
  }

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
  const auto& ri = SINGLE_RendererInfo::instance;
  const auto screen_size = ImVec2{ (float)ri.viewport_size_render_at.x, (float)ri.viewport_size_render_at.y };
  ImGui::SetNextWindowPos({ 0, 0 }, ImGuiCond_Always, { 0.0f, 0.0f });
  ImGui::SetNextWindowSize(screen_size, ImGuiCond_Always);
  imgui_begin("overlay", ImGuiWindowFlags_NoInputs);
  const auto tl = ImGui::GetWindowPos();
  const auto wh = ImGui::GetWindowSize();
  auto* draw_list = ImGui::GetWindowDrawList();
  auto font_size = (float)FontSizes::SIZE_16;
  auto* font = get_inter_font(r);

  const auto tilesize = SINGLE_Islands::instance.tilesize;

  for (const auto& [e, t_c, cannon_c, clip_c] :
       r.view<const TransformComponent, IslandCannonComponent, const WeaponClipSize>().each()) {
    const auto ws_pos = glm::vec2{ t_c.position.x, t_c.position.y };
    const auto ss_pos = worldspace_to_screenspace(r, ws_pos);
    const auto im_ss_pos = ImVec2(ss_pos.x, ss_pos.y);

    const auto hits = cannon_c.hits_to_repair - cannon_c.hits_to_repair_left;
    const bool repaired = cannon_c.hits_to_repair_left == 0;
    auto text = std::format("{}/{}", hits, cannon_c.hits_to_repair);
    if (repaired)
      text = std::format("{}/{}", clip_c.bullets_cur, clip_c.bullets_max);

    // assuming this cannon is on an edge of the island,
    // work out which way is the edge to the ocean.
    auto& line_info_c = r.get_or_emplace<IslandLineInfo>(e);
    calculate_edges(r, e, line_info_c);
    const auto invalid_neighbours = line_info_c.water_edges;

    if (!line_info_c.initialized_edges) {
      auto cannon_e = e;
      SDL_Log("Assigning island cannon a hardpoint direction.");
      HardpointComponent hardpoint_c;
      hardpoint_c.data.key = "cannon";
      // note: in data format, 90degrees is up, 270 is down.
      // when load in, convert to engine, where 90 is down, 270 is up
      // here, we consider it in "data" format, as the arcs_system flips it.
      hardpoint_c.data.arc_mid = 0;
      hardpoint_c.data.arc = 360;
      hardpoint_c.data.x_rel_tl = 8;
      hardpoint_c.data.y_rel_tl = 8;
      r.emplace<HardpointComponent>(cannon_e, hardpoint_c);

      const auto& transform_c = r.get<TransformComponent>(cannon_e);
      const auto pos = glm::vec2{ transform_c.position.x, transform_c.position.y };
      const auto pos_adj = pos - glm::vec2{ tilesize * 0.5f, tilesize * 0.5f };
      const auto gp = engine::grid::worldspace_to_gridspace(pos_adj, tilesize);

      // aim the cannon at one of the water tiles
      const auto water_gp = line_info_c.water_edges[0].second;
      const auto you_gp = gp;
      const auto dir_gp = water_gp - you_gp;
      const auto angle = engine::dir_to_angle_radians(dir_gp);
      const auto cannon_par_e = r.get<HasParentComponent>(cannon_e).parent;
      set_rotation(r, cannon_par_e, angle);

      line_info_c.initialized_edges = true;
    }

    // if its north, bump right one.
    // if its south, bump right one.
    // if you're east, bump up one.
    // if you're west, bump up one.
    auto [empty_dir, empty_gp] = invalid_neighbours[0];
    auto offset = ImVec2{ 0, 0 };
    if (empty_dir == engine::grid::GridDirection::north)
      offset = ImVec2(1, 1);
    else if (empty_dir == engine::grid::GridDirection::south)
      offset = ImVec2(1, -1);
    else if (empty_dir == engine::grid::GridDirection::east)
      offset = ImVec2(1, 1);
    else if (empty_dir == engine::grid::GridDirection::west)
      offset = ImVec2(-1, 1);

    // point p1 off the island
    const auto text_size = font->CalcTextSizeA(font_size, FLT_MAX, -1, text.c_str());
    const auto p0 = im_ss_pos;
    const auto p1 = im_ss_pos + offset * ImVec2(16, 16);
    const auto p2 = p1 + ImVec2(offset.x * text_size.x, 0);

    auto im_col = im_line_col;
    if (repaired)
      im_col = im_greenish;

    auto my_col = my_line_col;
    if (repaired)
      my_col = my_greenish;

    draw_dashed_line(draw_list, p0, p1, 1, my_col);
    draw_dashed_line(draw_list, p1, p2, 1, my_col);

    // const auto text_pos = ImVec2{ glm::min(p1.x, p2.x), p1.y } + ImVec2(0, +text_size.y * 0.5f);
    const auto text_pos = ImVec2{ glm::min(p1.x, p2.x), p1.y };
    draw_list->AddText(font, font_size, text_pos, im_col, text.c_str());
  }

  for (const auto& [e, t_c, islander_c, revivable_c] :
       r.view<const TransformComponent, const MovementIslandComponent, const RevivableComponent>().each()) {
    const auto ws_pos = glm::vec2{ t_c.position.x, t_c.position.y };
    const auto ss_pos = worldspace_to_screenspace(r, ws_pos);
    const auto im_ss_pos = ImVec2(ss_pos.x, ss_pos.y);

    const auto hits_cur = revivable_c.hits_to_revive_cur;
    const auto hits_max = revivable_c.hits_to_revive;
    const auto text = std::format("Revive {}/{}", hits_cur, hits_max);

    // assuming this cannon is on an edge of the island,
    // work out which way is the edge to the ocean.
    auto& line_info_c = r.get_or_emplace<IslandLineInfo>(e);
    calculate_edges(r, e, line_info_c);
    if (!line_info_c.initialized_edges)
      line_info_c.initialized_edges = true;

    const auto invalid_neighbours = line_info_c.water_edges;
    // if its north, bump right one.
    // if its south, bump right one.
    // if you're east, bump up one.
    // if you're west, bump up one.
    auto [empty_dir, empty_gp] = invalid_neighbours[0];
    auto offset = ImVec2{ 0, 0 };
    if (empty_dir == engine::grid::GridDirection::north)
      offset = ImVec2(1, 1);
    else if (empty_dir == engine::grid::GridDirection::south)
      offset = ImVec2(1, -1);
    else if (empty_dir == engine::grid::GridDirection::east)
      offset = ImVec2(1, 1);
    else if (empty_dir == engine::grid::GridDirection::west)
      offset = ImVec2(-1, 1);

    // point p1 off the island
    const auto text_size = font->CalcTextSizeA(font_size, FLT_MAX, -1, text.c_str());
    const auto p0 = im_ss_pos;
    const auto p1 = im_ss_pos + offset * ImVec2(16, 16);
    const auto p2 = p1 + ImVec2(offset.x * text_size.x, 0);

    const auto my_col = my_line_col;
    const auto im_col = im_line_col;

    draw_dashed_line(draw_list, p0, p1, 1, my_col);
    draw_dashed_line(draw_list, p1, p2, 1, my_col);

    const auto text_pos = ImVec2{ glm::min(p1.x, p2.x), p1.y };
    draw_list->AddText(font, font_size, text_pos, im_col, text.c_str());
  }

  ImGui::End();
  ImGui::PopStyleVar(2);
}

} // namespace game2d