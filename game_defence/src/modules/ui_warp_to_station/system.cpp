#include "system.hpp"

#include "entt/helpers.hpp"
#include "maths/grid.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/actor_spacestation/components.hpp"
#include "modules/actors/helpers.hpp"
#include "modules/algorithm_astar_pathfinding/components.hpp"
#include "modules/algorithm_astar_pathfinding/helpers.hpp"
#include "modules/grid/components.hpp"

#include "fmt/core.h"
#include "imgui.h"
#include "modules/grid/helpers.hpp"
#include "modules/lerp_to_target/components.hpp"
#include "modules/renderer/components.hpp"
#include "physics/components.hpp"

namespace game2d {

void
update_ui_warp_to_station_system(entt::registry& r)
{
  const auto& map_e = get_first<MapComponent>(r);
  if (map_e == entt::null)
    return;

  const auto& player_e = get_first<PlayerComponent>(r);
  if (player_e == entt::null)
    return;
  const glm::vec2 player_pos = get_position(r, player_e);

  const auto stop_spaceship_ai = [&r](const entt::entity& e) {
    // warp-speed, deactivate!
    auto& player_vel = r.get<VelocityComponent>(e);
    player_vel.base_speed = 100.0f;

    if (const auto* comp = r.try_get<GeneratedPathComponent>(e))
      r.remove<GeneratedPathComponent>(e);
    if (const auto* comp = r.try_get<HasTargetPositionComponent>(e))
      r.remove<HasTargetPositionComponent>(e);
    if (const auto* comp = r.try_get<SetVelocityToTargetComponent>(e))
      r.remove<SetVelocityToTargetComponent>(e);
  };

  // if player touches WASD, stop ai control if it is occuring
  const auto& player_input = r.get<InputComponent>(player_e);
  if (glm::abs(player_input.lx) > 0.0f)
    stop_spaceship_ai(player_e);
  if (glm::abs(player_input.ly) > 0.0f)
    stop_spaceship_ai(player_e);

  const auto update_player_to_spacestation = [](entt::registry& r, const entt::entity& src_e, const entt::entity& dst_e) {
    const auto& map = get_first_component<MapComponent>(r); // gets updated if units was dead
    const auto grid = map_to_grid(r);

    const auto src = get_position(r, src_e);
    const auto src_idx = convert_position_to_index(map, src);

    const auto dst = get_position(r, dst_e);
    const int dst_idx = convert_position_to_index(map, dst);

    if (src_idx == dst_idx)
      return;

    const auto path = generate_direct(r, grid, src_idx, dst_idx);

    const glm::ivec2 worldspace_tl = engine::grid::grid_space_to_world_space(path[path.size() - 1], map.tilesize);
    const glm::ivec2 worldspace_center = worldspace_tl + glm::ivec2{ map.tilesize / 2.0f, map.tilesize / 2.0f };
    const glm::ivec2 dst_pos = worldspace_center;

    GeneratedPathComponent path_c;
    path_c.path = path;
    path_c.src_pos = get_position(r, src_e);
    path_c.dst_pos = dst_pos;
    path_c.dst_ent = entt::null;
    path_c.required_to_clear_path = false;
    path_c.wait_at_destination = false;
    path_c.wait_time_ms = 0.0f;
    path_c.path_cleared.resize(path.size());
    r.emplace_or_replace<GeneratedPathComponent>(src_e, path_c);
  };

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoFocusOnAppearing;
  flags |= ImGuiWindowFlags_NoCollapse;
  flags |= ImGuiWindowFlags_NoResize;
  flags |= ImGuiWindowFlags_NoTitleBar;
  // flags |= ImGuiWindowFlags_NoBackground;

  const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
  const ImVec2 size{ 200, 250 };
  const ImVec2 pos{ ri.viewport_size_render_at.x - size.x, 0 };

  ImGui::SetNextWindowPos(pos, ImGuiCond_Always, { 0, 0 });
  ImGui::SetNextWindowSize(size, ImGuiCond_Always);

  ImGui::Begin("WarpToSpaceship", NULL, flags);

  ImGui::Separator();
  if (has_destination(r, player_e)) {
    ImGui::Text("Warp Drive: active");

    // Check when to deactivate warp drive...
    if (at_destination(r, player_e))
      stop_spaceship_ai(player_e);

  } else
    ImGui::Text("Warp Drive: inactive");
  ImGui::Separator();

  const auto& view = r.view<const SpacestationComponent>(entt::exclude<SpacestationUndiscoveredComponent>);
  for (const auto& [station_e, spaceship] : view.each()) {
    const auto eid = static_cast<uint32_t>(station_e);
    ImGui::PushID(eid);

    if (ImGui::Button("Warp")) {
      // warp-speed, activate!
      auto& player_vel = r.get<VelocityComponent>(player_e);
      player_vel.base_speed = 500.0f;

      update_player_to_spacestation(r, player_e, station_e);

      // player ship to automatically fly to destination
      r.emplace_or_replace<HasTargetPositionComponent>(player_e);
      r.emplace_or_replace<SetVelocityToTargetComponent>(player_e);
    }

    ImGui::SameLine();
    const std::string station_name = fmt::format("S{}", spaceship.idx);

    const glm::vec2 spacestation_pos = get_position(r, station_e);
    const glm::vec2 d = player_pos - spacestation_pos;
    const float d2 = (d.x * d.x + d.y * d.y) / 10000.0f;
    ImGui::Text("%s AU %.2f", station_name.c_str(), d2);

    ImGui::PopID();
  }

  for (const auto& [station_e, station, undiscovered] :
       r.view<const SpacestationComponent, const SpacestationUndiscoveredComponent>().each()) {
    ImGui::Text("Spacestation Undiscovered");

    const glm::vec2 spacestation_pos = get_position(r, station_e);
    const glm::vec2 d = player_pos - spacestation_pos;
    const float d2 = (d.x * d.x + d.y * d.y) / 10000.0f;
    if (d2 <= 8)
      r.remove<SpacestationUndiscoveredComponent>(station_e);
  }

  ImGui::End();
}

} // namespace game2d