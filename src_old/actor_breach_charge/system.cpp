#include "system.hpp"

#include "actors/actors.hpp"

#include "engine/audio/components.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/events/helpers/mouse.hpp"
#include "engine/maths/grid.hpp"
#include "engine/maths/maths.hpp"
#include "modules/actor_breach_charge/helpers.hpp"
#include "modules/camera/helpers.hpp"
#include "modules/grid/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/ui_inventory/components.hpp"

#include <fmt/core.h>

#include "imgui.h"

namespace game2d {

void
DrawZeldaCursor(const ImVec2& pos, const ImVec2& window_size, float radius, float thickness, float angle, ImU32 color)
{
  const ImVec2 center = ImGui::GetCursorScreenPos(); // ImDrawList API uses screen coordinates!
  const float max_angle = 2.0f * engine::PI;

  if (angle > max_angle)
    angle = max_angle;
  const float start_angle = -engine::PI / 2; // Starting at the top (12 o'clock position)
  const float end_angle = start_angle + (angle / max_angle) * 2.0f * engine::PI;

  // ImDrawList* draw_list = ImGui::GetWindowDrawList();
  // Use the foreground draw list so it's drawn on top of everything
  ImDrawList* draw_list = ImGui::GetForegroundDrawList();

  ImVec2 window_center = { center.x + window_size.x / 2.0f, center.y + window_size.y / 2.0f };
  // window_center.y += radius / 2.0f; // move circle to center

  draw_list->PathArcTo(window_center, radius, start_angle, end_angle);
  draw_list->PathStroke(color, false, thickness);
};

void
DrawZeldaCursorWindow(entt::registry& r, const glm::ivec2& mouse_pos, const float angle)
{
  const float radius = 10.0f;
  const float thickness = 6.0f;
  const ImU32 color = IM_COL32(0, 255, 0, 255);

  const auto t_pos = glm::ivec2(mouse_pos.x, mouse_pos.y);
  const auto worldspace = position_in_worldspace(r, t_pos);
  const ImVec2 pos = { static_cast<float>(worldspace.x), static_cast<float>(worldspace.y) };
  const ImVec2 size = ImVec2(200, 200);
  ImGui::SetNextWindowSize(size);
  ImGui::SetNextWindowPos({ pos.x - size.x / 2.0f, pos.y - size.y / 2.0f });

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoInputs;
  flags |= ImGuiWindowFlags_NoDecoration;
  flags |= ImGuiWindowFlags_NoMove;
  flags |= ImGuiWindowFlags_NoBackground;
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0)); // Disable padding
  ImGui::Begin("cursor-timer", NULL, flags);
  ImGui::PopStyleVar();

  DrawZeldaCursor(pos, size, radius, thickness, angle, color);

  ImGui::End();
};

void
update_breach_charge_system(entt::registry& r, const glm::ivec2& mouse_pos, const float dt)
{
  const auto& map = get_first_component<MapComponent>(r);

  const auto& ri = get_first_component<SINGLE_RendererInfo>(r);
  if (!ri.viewport_hovered)
    return; // no ui

  const auto offset = glm::vec2{ map.tilesize / 2.0f, map.tilesize / 2.0f };
  glm::ivec2 mouse_pos_on_grid = engine::grid::worldspace_to_clamped_world_space(mouse_pos, map.tilesize);
  mouse_pos_on_grid += offset;

  // do the rest of the logic if not debugging this
  if (debug_spawn_bombs(r, mouse_pos_on_grid))
    return;

  // only place bombs if bomb equipped
  const auto bomb_e = bomb_equipped_in_inventory(r);
  if (bomb_e == entt::null)
    return;

  static float lmb_held_time = 0.0f;
  static float lmb_time_to_place_bomb = 2.5f;

  if (get_mouse_lmb_held())
    lmb_held_time += dt;

  if (get_mouse_lmb_release())
    lmb_held_time = 0.0f;

  const bool cursor_complete_so_place_bomb = lmb_held_time > lmb_time_to_place_bomb;
  if (cursor_complete_so_place_bomb)
    lmb_held_time = 0.0f;
  // ImGui::Text("Bomb Held time: %f", lmb_held_time);

  // Convert from [0, time_to_place_bomb] to [0, 2*PI]
  const float angle = engine::scale(lmb_held_time, 0.0f, lmb_time_to_place_bomb, 0.0f, 2.0f * engine::PI);
  DrawZeldaCursorWindow(r, mouse_pos, angle);

  if (cursor_complete_so_place_bomb) {
    fmt::println("spawning bomb!");

    // spawn the bomb!
    DataBreachCharge desc;
    desc.pos = glm::vec2(mouse_pos_on_grid);
    const auto charge_e = Factory_DataBreachCharge::create(r, desc);

    // remove bomb from inventory (a one use)
    // update equipment slot
    const auto equipment_slot_e = bomb_e;
    auto& equipment_slot = r.get<InventorySlotComponent>(equipment_slot_e);
    r.destroy(equipment_slot.item_e);   // destroy item
    equipment_slot.item_e = entt::null; // reset slot
  }
};

} // namespace game2d