#include "system.hpp"

#include "actors/actors.hpp"

#include "components.hpp"
#include "entt/helpers.hpp"
#include "events/helpers/mouse.hpp"
#include "maths/grid.hpp"
#include "maths/maths.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/camera/helpers.hpp"
#include "modules/grid/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/ui_inventory/components.hpp"
#include "modules/ui_inventory/helpers.hpp"

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

  // only place bombs if viewport is hovered
  const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
  const bool viewport_hovered = ri.viewport_hovered;
  ImGui::Text("Viewport Hovered: %i", viewport_hovered);
  if (!viewport_hovered)
    return;

  const auto offset = glm::vec2{ map.tilesize / 2.0f, map.tilesize / 2.0f };
  glm::ivec2 mouse_pos_on_grid = engine::grid::worldspace_to_clamped_world_space(mouse_pos, map.tilesize);
  mouse_pos_on_grid += offset;

  static float lmb_held_time = 0.0f;
  static float lmb_time_to_place_bomb = 2.5f;
  if (get_mouse_lmb_held())
    lmb_held_time += dt;
  if (get_mouse_lmb_release())
    lmb_held_time = 0.0f;
  const bool place_bomb = lmb_held_time > lmb_time_to_place_bomb;
  if (place_bomb)
    lmb_held_time = 0.0f;
  ImGui::Text("Bomb Held time: %f", lmb_held_time);

  // Convert from [0, time_to_place_bomb] to [0, 2*PI]
  const float angle = engine::scale(lmb_held_time, 0.0f, lmb_time_to_place_bomb, 0.0f, 2.0f * engine::PI);
  DrawZeldaCursorWindow(r, mouse_pos, angle);

  bool imediately_place_bomb = false;
#if defined(_DEBUG)
  imediately_place_bomb = false;
#endif

  // debug spawn loads of bombs
  if (imediately_place_bomb && get_mouse_lmb_press()) {

    DataBreachCharge desc;
    desc.pos = glm::vec2(mouse_pos_on_grid);
    const auto charge_e = Factory_DataBreachCharge::create(r, desc);
  }

  if (imediately_place_bomb)
    return;

  // Check that you've got a breach charge equipped
  const auto& view = r.view<const PlayerComponent, DefaultBody>();
  for (const auto& [e, player_c, equipment_c] : view.each()) {
    bool able_to_use_breach_charge = false;

    // std::vector<entt::entity> equipment = equipment_c.body;

    const auto slots = get_slots(r, e, InventorySlotType::gun);
    if (slots.size() > 0) {
      const auto equipment_slot_e = slots[0];
      const auto equipment_slot = r.get<InventorySlotComponent>(equipment_slot_e);
      const auto has_item = equipment_slot.item_e != entt::null;
      if (!has_item)
        continue;

      // Is the item a breach charge?
      if (r.get<ItemTypeComponent>(equipment_slot.item_e).type == ItemType::bomb)
        able_to_use_breach_charge = true;
    }

    if (able_to_use_breach_charge && place_bomb) {
      fmt::println("spawning bomb!");

      // spawn the bomb!
      DataBreachCharge desc;
      desc.pos = glm::vec2(mouse_pos_on_grid);
      const auto charge_e = Factory_DataBreachCharge::create(r, desc);

      // remove bomb from inventory (a one use)
      // update equipment slot
      const auto equipment_slot_e = slots[0];
      auto& equipment_slot = r.get<InventorySlotComponent>(equipment_slot_e);
      r.destroy(equipment_slot.item_e);   // destroy item
      equipment_slot.item_e = entt::null; // reset slot

      // doesnt do anything, but just to be sure
      able_to_use_breach_charge = false;
    }
  }
};

} // namespace game2d