#include "system.hpp"

#include "entt/helpers.hpp"
#include "lifecycle/components.hpp"
#include "maths/grid.hpp"
#include "modules/actor_enemy_patrol/components.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/actor_spacestation/components.hpp"
#include "modules/actor_turret/helpers.hpp"
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
  const auto check_if_player_pressed_key = [&r, &stop_spaceship_ai](const entt::entity& e) {
    if (const auto* input_c = r.try_get<InputComponent>(e)) {
      if (glm::abs(input_c->lx) > 0.0f)
        stop_spaceship_ai(e);
      if (glm::abs(input_c->ly) > 0.0f)
        stop_spaceship_ai(e);
    }
  };

  const auto update_player_to_spacestation = [](entt::registry& r, const entt::entity& src_e, const entt::entity& dst_e) {
    const auto& map = get_first_component<MapComponent>(r); // gets updated if units was dead

    const auto src = get_position(r, src_e);
    const auto src_idx = convert_position_to_index(map, src);

    const auto dst = get_position(r, dst_e);
    const int dst_idx = convert_position_to_index(map, dst);

    if (src_idx == dst_idx)
      return;

    const auto path = generate_direct(r, map, src_idx, dst_idx);

    glm::ivec2 dst_pos = engine::grid::index_to_world_position(dst_idx, map.xmax, map.ymax, map.tilesize);
    const glm::ivec2 offset = { map.tilesize / 2, map.tilesize / 2 };
    dst_pos += offset;

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

  // Players able to warp to stations?
  for (const auto& [e, player_c] : r.view<PlayerComponent>().each()) {
    ImGui::Text("Warp drive... ");
    ImGui::SameLine();

    // Warp Drive status
    if (has_destination(r, e)) {
      ImGui::Text("active");

      // warp-speed, activate!
      auto& player_vel = r.get<VelocityComponent>(e);
      player_vel.base_speed = 500.0f;

      // When to stop warp?
      // ...on reaching destination
      if (at_destination(r, e))
        stop_spaceship_ai(e);
      // ...on key press
      check_if_player_pressed_key(e);

    } else
      ImGui::Text("inactive");
  }

  // configs
  const float range_to_discover_station = 0.8f * 100'000;

  ImGui::SeparatorText("Stations");

  const auto& discovered_spacestations_view =
    r.view<const SpacestationComponent>(entt::exclude<SpacestationUndiscoveredComponent>);
  for (const auto& [station_e, station_c] : discovered_spacestations_view.each()) {
    ImGui::PushID(static_cast<uint32_t>(station_e));
    ImGui::Text("Station %i", station_c.idx);

    // Let players dock
    const auto in_range = get_within_range<PlayerComponent>(r, station_e, 100 * 100);
    for (const auto& [player_e, d2] : in_range) {
      ImGui::PushID(static_cast<uint32_t>(player_e));

      const float fake_au = d2 / 10000.0f;
      ImGui::Text("Entity.. %f AU", fake_au);

      const auto* docked_comp = r.try_get<DockedAtStationComponent>(player_e);
      if (docked_comp && ImGui::Button("Undock")) {
        r.remove<DockedAtStationComponent>(player_e);
        r.emplace_or_replace<InputComponent>(player_e);
      } else if (!docked_comp && ImGui::Button("Dock")) {
        r.emplace_or_replace<DockedAtStationComponent>(player_e, DockedAtStationComponent{ station_e });
        r.remove<InputComponent>(player_e);

        // seems like a bad idea
        // auto& dead = get_first_component<SINGLETON_EntityBinComponent>(r);
        // dead.dead.emplace(first_player_e);
      }

      ImGui::PopID();
    }

    // temporary: first player warp to stations
    const auto first_player_e = get_first<PlayerComponent>(r);
    if (first_player_e != entt::null) {
      const auto& e = first_player_e;
      const bool warping = has_destination(r, e) && !at_destination(r, e);
      const bool docked = r.try_get<DockedAtStationComponent>(e) != nullptr;

      // todo: show station list...
      // select station to warp to...

      if (!warping && !docked) {
        ImGui::SameLine();
        if (ImGui::Button("Warp")) {

          // set path to spacestation
          update_player_to_spacestation(r, e, station_e);

          // player ship to automatically fly to destination
          r.emplace_or_replace<HasTargetPositionComponent>(e);
          r.emplace_or_replace<SetVelocityToTargetComponent>(e);
        }
      }
    }

    ImGui::PopID();
  }

  const auto undiscovered_stations_view = r.view<const SpacestationComponent, const SpacestationUndiscoveredComponent>();
  for (const auto& [station_e, station, undiscovered] : undiscovered_stations_view.each()) {
    ImGui::Text("Undiscovered");

    const auto in_range = get_within_range<PlayerComponent>(r, station_e, range_to_discover_station);
    if (in_range.size() > 0)
      r.remove<SpacestationUndiscoveredComponent>(station_e);
  }

  ImGui::End();
}

} // namespace game2d