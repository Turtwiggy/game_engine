#include "system.hpp"

#include "components.hpp"

#include "entt/helpers.hpp"
#include "maths/grid.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/actors/helpers.hpp"
#include "modules/gen_dungeons/components.hpp"
#include "modules/gen_dungeons/helpers.hpp"
#include "modules/grid/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/system_entered_new_room/components.hpp"
#include "modules/ui_event_console/components.hpp"

#include "imgui.h"

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
    const auto player_pos = get_position(r, e);
    const auto player_gridspace = engine::grid::worldspace_to_grid_space(player_pos, map.tilesize);
    const auto [in_room, room] = inside_room(map, dungeon.rooms, player_gridspace);
    if (in_room) {
      auto& player_in_room = r.get_or_emplace<PlayerInRoomComponent>(e);
      // player was never in a room
      if (!player_in_room.room_tl.has_value())
        player_in_room.room_tl = room->tl;
      // player entered a new room...
      if (player_in_room.room_tl.has_value() && player_in_room.room_tl.value() != room->tl) {
        // boom! entered a new room
        PlayerEnteredNewRoom data;
        data.player = e;
        data.room = room.value();
        create_empty<PlayerEnteredNewRoom>(r, data);
        player_in_room.room_tl = room.value().tl;
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

    // get the eid of the room... could be sent in req...
    for (const auto& [room_e, room] : r.view<Room>().each()) {
      if (req.room == room) {
        room_name = std::to_string(static_cast<uint32_t>(room_e));

        const std::string label = "Entered room "s + room_name.value();
        evts.events.push_back(label);

        break;
      }
    }

    break; // only show first
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
    const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
    const auto& viewport_pos = ImVec2(ri.viewport_pos.x, ri.viewport_pos.y);
    const auto& viewport_size_half = ImVec2(ri.viewport_size_current.x * 0.5f, ri.viewport_size_current.y * 0.5f);

    // text size
    std::string label = "N/A";
    if (room_name.has_value())
      label = "Entered Room "s + room_name.value();
    ImGuiStyle& style = ImGui::GetStyle();
    const float alignment = 0.5f;
    const float size = ImGui::CalcTextSize(label.c_str()).x + style.FramePadding.x * 2.0f;

    // window size
    const float forced_padding = size / 2.0;
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
