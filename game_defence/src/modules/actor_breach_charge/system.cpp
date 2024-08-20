#include "system.hpp"

#include "components.hpp"

#include "actors.hpp"
#include "entt/helpers.hpp"
#include "events/helpers/mouse.hpp"
#include "lifecycle/components.hpp"
#include "maths/grid.hpp"
#include "maths/maths.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/actors/helpers.hpp"
#include "modules/camera/helpers.hpp"
#include "modules/grid/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/system_particles/components.hpp"
#include "modules/ui_inventory/components.hpp"
#include "modules/ui_inventory/helpers.hpp"
#include "physics/components.hpp"

#include <fmt/core.h>

#include "imgui.h"

namespace game2d {

void
add_bomb_callback(entt::registry& r, const entt::entity e)
{
  OnDeathCallback callback;
  callback.callback = [](entt::registry& r, const entt::entity& e) {
    // create a boom effect
    const glm::vec2 pos = get_position(r, e);
    create_empty<RequestToSpawnParticles>(r, RequestToSpawnParticles{ pos });

    // open up any edges surrounding the boom
    // Did I accidentally create bomberman?
    auto& physics = get_first_component<SINGLE_Physics>(r);
    auto& map = get_first_component<MapComponent>(r);
    auto& dead = get_first_component<SINGLETON_EntityBinComponent>(r);
    const auto gp = engine::grid::worldspace_to_grid_space(pos, map.tilesize);
    // fmt::println("go boom at {}, {}. gp: {}, {}", pos.x, pos.y, gp.x, gp.y);

    // Have to use gridpos not index, because gridpos {-1, 12} is valid for a bomb to be placed
    // but if that was convertd to an index it would wrap round the value and be too large.
    const auto neighbours = engine::grid::get_neighbour_gridpos(gp, map.xmax, map.ymax);
    for (const auto& [dir, n_gp] : neighbours) {

      // Note: for a_idx in the edge representation,
      // a_idx is in bounds, and b_idx is out of bounds,
      // representing that an edge is on the border.
      // This means that one gp is in bounds, and one is out of bounds,
      // or both gp are in bounds.

      const auto gp_bomb = gp; // where the bomb is placed
      const bool gp_bomb_oob_x = gp_bomb.x < 0 || gp_bomb.x >= map.xmax;
      const bool gp_bomb_oob_y = gp_bomb.y < 0 || gp_bomb.y >= map.ymax;
      const bool bomb_out_of_bounds = gp_bomb_oob_x || gp_bomb_oob_y;

      const auto gp_neighbour = n_gp; // a neighbouring grid position
      const bool gp_neighbour_oob_x = gp_neighbour.x < 0 || gp_neighbour.x >= map.xmax;
      const bool gp_neighbour_oob_y = gp_neighbour.y < 0 || gp_neighbour.y >= map.ymax;
      const bool neighbour_out_of_bounds = gp_neighbour_oob_x || gp_neighbour_oob_y;

      const auto pred = [&](const Edge& other) -> bool {
        //
        // No edge (should) be valid with these conditions
        if (bomb_out_of_bounds && neighbour_out_of_bounds)
          return false;

        // one of the edges is out of bound, and we're on an edge that has an out of bound edge.
        if ((bomb_out_of_bounds || neighbour_out_of_bounds) && (other.b_idx == -1)) {

          // the edge's in-bound edge.
          const auto xy = engine::grid::index_to_grid_position(other.a_idx, map.xmax, map.ymax);

          glm::ivec2 inb_gp{ 0, 0 };
          glm::ivec2 oob_gb{ 0, 0 };
          if (bomb_out_of_bounds) {
            oob_gb = gp_bomb;
            inb_gp = gp_neighbour;
          }
          if (neighbour_out_of_bounds) {
            oob_gb = gp_neighbour;
            inb_gp = gp_bomb;
          }

          // get the other edge grid pos
          glm::ivec2 edge_oob{ 0, 0 };
          if (xy.x == 0)
            edge_oob = { -1, xy.y };
          if (xy.y == 0)
            edge_oob = { xy.x, -1 };
          if (xy.x == map.xmax)
            edge_oob = { xy.x + 1, xy.y };
          if (xy.y == map.ymax)
            edge_oob = { xy.x, xy.y + 1 };

          // Pull the lever, kronk!
          return xy == inb_gp && edge_oob == oob_gb;
        }

        // The only case we have left:
        // both bomb and neighbour grid positions are in bounds
        Edge ed;
        ed.a_idx = engine::grid::grid_position_to_index(gp, map.xmax);
        ed.b_idx = engine::grid::grid_position_to_index(n_gp, map.xmax);

        // do a swap because the map.edges() expects a < b, except for the case that b is -1
        if (ed.a_idx > ed.b_idx)
          std::swap(ed.a_idx, ed.b_idx);

        return ed == other;
      };

      const auto it = std::find_if(map.edges.begin(), map.edges.end(), pred);
      if (it != map.edges.end()) {
        const entt::entity instance = it->instance;
        if (instance == entt::null)
          continue;

        // destroy body immediately
        auto& physics_b = r.get<PhysicsBodyComponent>(instance).body;
        physics.world->DestroyBody(physics_b);
        r.remove<PhysicsBodyComponent>(instance);

        // destroy transform
        dead.dead.emplace(instance);

        // destroy edge
        map.edges.erase(it);
        fmt::println("removed edge..");
      }

      // TODO: everything in that room (or tunnel) gets sucked out
    }
  };
  r.emplace<OnDeathCallback>(e, callback);
};

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
}

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
}

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
  imediately_place_bomb = true;
#endif

  // debug spawn loads of bombs
  if (imediately_place_bomb && get_mouse_lmb_press()) {
    // spawn the bomb!
    const auto charge_e = create_gameplay(r, EntityType::actor_breach_charge, mouse_pos_on_grid);
    r.emplace<EntityTimedLifecycle>(charge_e, 3 * 1000);
    add_bomb_callback(r, charge_e);
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
      const auto& item_c = r.get<ItemComponent>(equipment_slot.item_e);
      if (item_c.type == ItemType::breach_charge) {
        able_to_use_breach_charge = true;
      }
    }

    if (able_to_use_breach_charge && place_bomb) {

      fmt::println("spawning bomb!");
      // spawn the bomb!
      const auto charge_e = create_gameplay(r, EntityType::actor_breach_charge, mouse_pos_on_grid);
      r.emplace<EntityTimedLifecycle>(charge_e, 3 * 1000);
      add_bomb_callback(r, charge_e);

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
}

} // namespace game2d