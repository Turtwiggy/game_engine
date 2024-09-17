#include "system.hpp"

#include "components.hpp"

#include "actors/helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/maths/grid.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/gen_dungeons/components.hpp"
#include "modules/gen_dungeons/helpers.hpp"
#include "modules/grid/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/system_entered_new_room/components.hpp"
#include "modules/ui_event_console/components.hpp"

#include "imgui.h"

#include <fmt/core.h>

namespace game2d {
using namespace std::literals;

void
update_entered_new_room_system(entt::registry& r, const float dt)
{
  if (get_first<MapComponent>(r) == entt::null)
    return;
  if (get_first<DungeonGenerationResults>(r) == entt::null)
    return;
  if (get_first<SINGLE_EventConsoleLogComponent>(r) == entt::null)
    return;
  const auto& map = get_first_component<MapComponent>(r);
  const auto& dungeon = get_first_component<DungeonGenerationResults>(r);

  const auto& view = r.view<const PlayerComponent>();
  for (const auto& [e, player] : view.each()) {
    const auto eid = static_cast<uint32_t>(e);
    const auto player_pos = get_position(r, e);
    const auto player_gridspace = engine::grid::worldspace_to_grid_space(player_pos, map.tilesize);

    const auto rooms = inside_room(r, player_gridspace);
    const bool in_room = rooms.size() > 0;

    auto& player_in_room_c = r.get_or_emplace<PlayerInRoomComponent>(e);
    if (!in_room)
      player_in_room_c.room_e = entt::null;

    const bool player_in_room = player_in_room_c.room_e != entt::null;
    if (in_room && !player_in_room) {
      const auto& room_c = r.get<Room>(rooms[0]);

      fmt::println("player {} entered new room (a)", eid);
      PlayerEnteredNewRoom data;
      data.player = e;
      data.room_e = rooms[0];
      create_empty<PlayerEnteredNewRoom>(r, data);

      player_in_room_c.room_e = rooms[0];
    }

    if (in_room && player_in_room) {
      const auto& room_c = r.get<Room>(player_in_room_c.room_e);

      if (player_in_room_c.room_e != rooms[0]) {
        fmt::println("player {} entered new room (b)", eid);
        PlayerEnteredNewRoom data;
        data.player = e;
        data.room_e = rooms[0];
        create_empty<PlayerEnteredNewRoom>(r, data);

        player_in_room_c.room_e = rooms[0];
      }
    }
  }

  //
  // TEMPORARY
  // Display imgui popup if an entered new room popup is fired.
  //
  auto& evts = get_first_component<SINGLE_EventConsoleLogComponent>(r);

  const float time_to_display_room_name = 6.0f;
  static float time_displaying_room_name = 0.0f;
  static std::optional<std::string> room_name = std::nullopt;

  const auto& reqs = r.view<PlayerEnteredNewRoom>();
  for (const auto& [req_e, req] : reqs.each()) {

    // get the eid of the room sent in req...
    room_name = std::to_string(static_cast<uint32_t>(req.room_e));

    const std::string label = "Entered room "s + room_name.value();
    evts.events.push_back(label);

    break; // only first event
  }

  // count all reqs as handled...
  // this could be an issue if multiple players walk in to multiple new rooms
  // r.destroy(reqs.begin(), reqs.end());
  // reqs not destroyed here... reqs destroyed in quips system

  if (room_name.has_value()) {
    time_displaying_room_name += dt;
    if (time_displaying_room_name > time_to_display_room_name) {
      room_name = std::nullopt;
      time_displaying_room_name = 0.0f;
    }
  }

  const bool show_room_ui = time_displaying_room_name <= time_to_display_room_name && time_displaying_room_name != 0.0f;
  if (show_room_ui) {
    const auto& ri = get_first_component<SINGLE_RendererInfo>(r);
    const ImVec2 viewport_pos = { (float)ri.viewport_pos.x, (float)ri.viewport_pos.y };
    const ImVec2 viewport_size_half = ImVec2(ri.viewport_size_current.x * 0.5f, ri.viewport_size_current.y * 0.5f);

    // text size
    std::string label = "N/A";
    if (room_name.has_value())
      label = "Entered Room "s + room_name.value();
    ImGuiStyle& style = ImGui::GetStyle();
    const float alignment = 0.5f;
    const float size = ImGui::CalcTextSize(label.c_str()).x + style.FramePadding.x * 2.0f;

    // window size
    const float forced_padding = size / 2.0f;
    const float w = size + forced_padding;
    const float h = 16.0f;
    ImGui::SetNextWindowSizeConstraints(ImVec2(w, h), ImVec2(w, h));

    // position
    const float center_x = viewport_pos.x + viewport_size_half.x;
    const float bottom_y = viewport_pos.y + ri.viewport_size_current.y - h;
    const auto pos = ImVec2(center_x, bottom_y);
    ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

    ImGuiWindowFlags flags = 0;
    flags |= ImGuiWindowFlags_NoMove;
    flags |= ImGuiWindowFlags_NoCollapse;
    flags |= ImGuiWindowFlags_NoTitleBar;
    flags |= ImGuiWindowFlags_NoResize;

    ImGui::Begin("Last Room Entered", NULL, flags);

    float avail = ImGui::GetContentRegionAvail().x;
    float off = (avail - size) * alignment;
    if (off > 0.0f)
      ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);
    ImGui::Text("%s", label.c_str());

    ImGui::End();
  }
};

} // namespace game2d
