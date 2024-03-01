#include "system.hpp"

#include "entt/helpers.hpp"
#include "events/helpers/mouse.hpp"
#include "maths/grid.hpp"
#include "modules/actor_cursor/components.hpp"
#include "modules/actor_hearth/components.hpp"
#include "modules/ai_pathfinding/helpers.hpp"
#include "modules/algorithm_procedural/cell_automata.hpp"
#include "modules/lifecycle/components.hpp"
#include "physics/components.hpp"

#include "imgui.h"
#include "magic_enum.hpp"

#include <iostream>
#include <vector>

namespace game2d {

void
display_grid(entt::registry& r, GridComponent& grid)
{
  for (int i = 0; i < grid.debug_generated_obstacles.size(); i++) {
    if (grid.debug_generated_obstacles[i] != entt::null)
      r.destroy(grid.debug_generated_obstacles[i]);
  }
  grid.debug_generated_obstacles.clear();

  const auto& map = grid.generated_obstacles;
  for (int i = 0; i < map.size(); i++) {

    entt::entity e = entt::null;
    if (map[i] == 1) {
      const auto gpos = engine::grid::index_to_grid_position(i, grid.width, grid.height);
      e = create_gameplay(r, EntityType::empty);
      auto& e_transform = r.get<TransformComponent>(e);
      e_transform.position = { gpos.x * grid.size, gpos.y * grid.size, 0.0 };
    }
    grid.debug_generated_obstacles.push_back(e);
  }
}

void
update_ui_grid_interaction_system(entt::registry& r)
{
  const auto& cursor = get_first_component<CursorComponent>(r);
  const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);

  // check grid exists
  const auto grid_e = get_first<GridComponent>(r);
  if (grid_e == entt::null)
    return;
  auto& grid = r.get<GridComponent>(grid_e);

  // check hearth exists
  // const auto hearth_e = get_first<HearthComponent>(r);
  // if (hearth_e == entt::null)
  //   return;
  // const auto& hearth_t = r.get<TransformComponent>(hearth_e);
  // const auto hearth_gridpos = engine::grid::world_space_to_grid_space(hearth_t.position, grid.size);
  // const auto hearth_idx = engine::grid::grid_position_to_index(hearth_gridpos, grid.width);

  ImGui::Begin("Debug Grid Interaction");

  if (cursor.grid_index != std::nullopt) {
    ImGui::Text("Cursor on index %i", cursor.grid_index.value());
    for (int i = 0; i < grid.grid[cursor.grid_index.value()].size(); i++) {
      ImGui::Text("On a thing");
    }
  }

  //
  // Interactions
  //

  bool something_was_placed = false;
  bool place = false;
  place |= get_mouse_lmb_press();
  place &= ri.viewport_hovered;

  bool see_grid_inventory = false;
  see_grid_inventory |= get_mouse_rmb_press();
  see_grid_inventory &= ri.viewport_hovered;

  bool valid_spot = cursor.grid_index != std::nullopt;
  bool valid_empty_spot = valid_spot && grid.grid[cursor.grid_index.value()].size() == 0;
  bool valid_full_spot = valid_spot && grid.grid[cursor.grid_index.value()].size() > 0;

  if (place && valid_empty_spot) {
    const int cursor_idx = cursor.grid_index.value();

    // put entity in that position
    const auto worldspace = engine::grid::index_to_world_position(cursor_idx, grid.width, grid.height, grid.size);
    auto e = create_gameplay(r, EntityType::actor_turret);
    auto& aabb = r.get<AABB>(e);
    aabb.center = { worldspace.x, worldspace.y };
    grid.grid[cursor_idx].push_back(e);

    // assume 0 is spawner
    // const auto path = generate_direct(r, grid, 0, hearth_idx);
    // bool allowed_to_place = true;

    // if (path.size() == 0)
    //   allowed_to_place = false;

    //   if (path.size() > 0) {
    //     // Check if the the final spot is the hearth...
    //     // if it isnt, the new block would block the path-flow
    //     auto end = path[path.size() - 1];
    //     auto end_idx = engine::grid::grid_position_to_index(end, grid.width);
    //     if (end_idx != hearth_idx)
    //       allowed_to_place = false;

    //     // Check the rest of the path onwards
    //     // from where we want to place the block.
    //     // If there's entities alive beyond the path,
    //     // there's a chance we trap them, breaking the pathing.
    //     // TODO: how to see if an enemy would be trapped?
    //     // Alternatively, have a list of pre-approved grid spots....
    //     //
    //     auto desired_grid_pos = engine::grid::index_to_grid_position(cursor_idx, grid.width, grid.height);
    //   } else
    //     std::cout << "Invalid place; no path" << std::endl;

    //   if (!allowed_to_place) {
    //     r.destroy(e);
    //     grid.grid[cursor_idx].clear();
    //   } else
    //     something_was_placed = true;
  }

  static bool show_inventory = false;
  static int show_inventory_idx = -1;

  if (see_grid_inventory && valid_full_spot) {
    show_inventory = true;
    show_inventory_idx = cursor.grid_index.value();
  }

  if (ImGui::Button("Generate Random Obstacles")) {
    static int seed = 0;
    seed++;
    grid.generated_obstacles = generate_50_50({ grid.width, grid.height }, seed);
    display_grid(r, grid);
  }

  if (ImGui::Button("Iterate Cell Automata")) {
    grid.generated_obstacles = iterate_with_cell_automata(grid.generated_obstacles, { grid.width, grid.height });
    display_grid(r, grid);
  }

  // if (ImGui::Button("Begin & Generate Flow Field")) {

  //   // Add generated_obstacles to grid
  //   for (int i = 0; i < grid.generated_obstacles.size(); i++) {
  //     if (grid.generated_obstacles[i] == 1) {
  //       const auto e = grid.debug_generated_obstacles[i];

  //       // make it an obstacle
  //       r.emplace_or_replace<PathfindComponent>(e, -1);

  //       grid.grid[i].push_back(e);
  //     }
  //   }

  //   grid.flow_field = generate_flow_field(r, grid, hearth_idx);
  // }

  // if (something_was_placed)
  //   grid.flow_field = generate_flow_field(r, grid, hearth_idx);

  ImGui::End();

  if (show_inventory) {
    const auto idx = show_inventory_idx;
    const auto& ents = grid.grid[idx];

    ImGui::Begin("Inventory");

    for (int i = 0; i < ents.size(); i++) {
      const auto& e = ents[i];
      const auto type = r.get<EntityTypeComponent>(e).type;
      const auto type_name = std::string(magic_enum::enum_name(type));
      ImGui::Text("Backpack for: %s", type_name.c_str());

      // auto* stats = r.try_get<BaseTurretStatsComponent>(e);
      // if (stats) {
      //   ImGui::Text("CON %i", stats->con_level);
      //   ImGui::Text("STR %i", stats->str_level);
      //   ImGui::Text("AGI %i", stats->agi_level);

      //   if (ImGui::Button("Increase CON"))
      //     stats->con_level += 1;
      //   if (ImGui::Button("Increase STR"))
      //     stats->str_level += 1;
      //   if (ImGui::Button("Increase AGI"))
      //     stats->agi_level += 1;
      // }

      // enum Mode
      // {
      //   Mode_Copy,
      //   Mode_Move,
      //   Mode_Swap
      // };
      // static int mode = 0;
      // if (ImGui::RadioButton("Copy", mode == Mode_Copy)) {
      //   mode = Mode_Copy;
      // }
      // ImGui::SameLine();
      // if (ImGui::RadioButton("Move", mode == Mode_Move)) {
      //   mode = Mode_Move;
      // }
      // ImGui::SameLine();
      // if (ImGui::RadioButton("Swap", mode == Mode_Swap)) {
      //   mode = Mode_Swap;
      // }
      // static const char* names[9] = {
      //   //
      //   "Block 1", "Melee 1",      "Ranged 1",     "Ammo 1",      "Damage 1",
      //   "HP 1",    "Gen Resource", "Gen Resource", "Gen Resource"
      //   //
      // };
      // for (int n = 0; n < IM_ARRAYSIZE(names); n++) {
      //   ImGui::PushID(n);
      //   if ((n % 3) != 0)
      //     ImGui::SameLine();
      //   ImGui::Button(names[n], ImVec2(60, 60));
      //   // Our buttons are both drag sources and drag targets here!
      //   if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
      //     // Set payload to carry the index of our item (could be anything)
      //     ImGui::SetDragDropPayload("DND_DEMO_CELL", &n, sizeof(int));
      //     // Display preview (could be anything, e.g. when dragging an image we could decide to display
      //     // the filename and a small preview of the image, etc.)
      //     if (mode == Mode_Copy) {
      //       ImGui::Text("Copy %s", names[n]);
      //     }
      //     if (mode == Mode_Move) {
      //       ImGui::Text("Move %s", names[n]);
      //     }
      //     if (mode == Mode_Swap) {
      //       ImGui::Text("Swap %s", names[n]);
      //     }
      //     ImGui::EndDragDropSource();
      //   }
      //   if (ImGui::BeginDragDropTarget()) {
      //     if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_DEMO_CELL")) {
      //       IM_ASSERT(payload->DataSize == sizeof(int));
      //       int payload_n = *(const int*)payload->Data;
      //       if (mode == Mode_Copy) {
      //         names[n] = names[payload_n];
      //       }
      //       if (mode == Mode_Move) {
      //         names[n] = names[payload_n];
      //         names[payload_n] = "";
      //       }
      //       if (mode == Mode_Swap) {
      //         const char* tmp = names[n];
      //         names[n] = names[payload_n];
      //         names[payload_n] = tmp;
      //       }
      //     }
      //     ImGui::EndDragDropTarget();
      //   }
      //   ImGui::PopID();
      // }
      //
    }

    if (ImGui::Button("Destroy")) {

      // destroy entity
      r.destroy(grid.grid[idx].begin(), grid.grid[idx].end());
      grid.grid[idx].clear();

      show_inventory = false;
      show_inventory_idx = -1;
    }

    if (ImGui::Button("Close##inventory")) {
      show_inventory = false;
      show_inventory_idx = -1;
    }

    ImGui::End();
  }
}

} // namespace game2d