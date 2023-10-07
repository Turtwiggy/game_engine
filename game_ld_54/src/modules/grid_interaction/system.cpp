#include "system.hpp"

#include "entt/helpers.hpp"
#include "events/helpers/mouse.hpp"
#include "maths/grid.hpp"
#include "modules/actor_cursor/components.hpp"
#include "modules/actor_hearth/components.hpp"
#include "modules/ai_pathfinding/helpers.hpp"
#include "modules/lifecycle/components.hpp"
#include "modules/procedural/cell_automata.hpp"

#include "imgui.h"

#include <iostream> // temp
#include <vector>

namespace game2d {

void
update_grid_interaction_system(entt::registry& r)
{
  const auto& cursor = get_first_component<CursorComponent>(r);

  // check grid exists
  const auto grid_e = get_first<GridComponent>(r);
  if (grid_e == entt::null)
    return;
  auto& grid = r.get<GridComponent>(grid_e);

  // check hearth exists
  const auto hearth_e = get_first<HearthComponent>(r);
  if (hearth_e == entt::null) {
    return;
  }
  const auto& hearth_t = r.get<TransformComponent>(hearth_e);
  const auto hearth_gridpos = engine::grid::world_space_to_grid_space(hearth_t.position, grid.size);
  const auto hearth_idx = engine::grid::grid_position_to_index(hearth_gridpos, grid.width);

  ImGui::Begin("Debug Grid Interaction");

  //
  // Interactions
  //

  bool place = false;
  // place |= get_mouse_lmb_press();

  bool see_grid_inventory = false;
  see_grid_inventory |= get_mouse_rmb_press();

  bool valid_empty_spot = cursor.grid_index != -1 && grid.grid[cursor.grid_index].size() == 0;
  bool valid_full_spot = cursor.grid_index != -1 && grid.grid[cursor.grid_index].size() > 0;

  if (place && valid_empty_spot) {

    // put a dummy entity in that position
    // and check if a path can be generated from start to end
    auto parent = r.create();
    r.emplace<PathfindComponent>(parent, -1);
    grid.grid[cursor.grid_index].push_back(parent);

    // assume 0 is spawner
    const auto path = generate_direct(r, grid, 0, hearth_idx);

    bool allowed_to_place = true;

    if (path.size() == 0) {
      std::cout << "no path!" << std::endl;
      allowed_to_place = false;
    } else {

      // Check if the the final spot is the hearth...
      // if it isnt, the new block would block the path-flow
      auto end = path[path.size() - 1];
      auto end_idx = engine::grid::grid_position_to_index(end, grid.width);
      if (end_idx != hearth_idx)
        allowed_to_place = false;

      // Check the rest of the path onwards
      // from where we want to place the block.
      // If there's entities alive beyond the path,
      // there's a chance we trap them, breaking the pathing.
      //
      auto desired_grid_idx = engine::grid::index_to_grid_position(cursor.grid_index, grid.width, grid.height);

      // TODO: how to see if an enemy would be trapped?
      // Alternatively, have a list of pre-approved grid spots....
    }

    if (allowed_to_place) {
      std::cout << "creating turret..." << std::endl;

      // valid request!
      const auto worldspace = engine::grid::index_to_world_position(cursor.grid_index, grid.width, grid.height, grid.size);
      CreateEntityRequest req;
      req.type = EntityType::actor_turret;
      TransformComponent t;
      t.position = { worldspace.x, worldspace.y, 0 };
      req.transform = t;
      req.parent = parent;
      r.emplace<CreateEntityRequest>(r.create(), req);

      // generate new flowfield towards hearth
      std::cout << "generating flow field" << std::endl;
      grid.flow_field = generate_flow_field(r, grid, hearth_idx);

    } else {

      // remove parent
      r.destroy(parent);
      grid.grid[cursor.grid_index].clear();
    }
  } // end place

  if (see_grid_inventory && valid_full_spot) {
  }

  if (ImGui::Button("Generate Random Obstacles")) {

    static int seed = 0;
    seed++;
    grid.cell_grid = generate_50_50({ grid.width, grid.height }, seed);

    // update visuals
    r.destroy(grid.debug_cell_ents.begin(), grid.debug_cell_ents.end());
    grid.debug_cell_ents.clear();

    const auto& map = grid.cell_grid;
    for (int i = 0; i < map.size(); i++) {
      if (map[i] == 1) {
        const auto gpos = engine::grid::index_to_grid_position(i, grid.width, grid.height);
        const auto e = create_gameplay(r, EntityType::empty);
        auto& e_transform = r.get<TransformComponent>(e);
        e_transform.position = { gpos.x * grid.size, gpos.y * grid.size, 0.0 };
        grid.debug_cell_ents.push_back(e);
      }
    }
  }

  if (ImGui::Button("Iterate Cell Automata")) {
    grid.cell_grid = iterate_with_cell_automata(grid.cell_grid, { grid.width, grid.height });

    // update visuals
    r.destroy(grid.debug_cell_ents.begin(), grid.debug_cell_ents.end());
    grid.debug_cell_ents.clear();

    const auto& map = grid.cell_grid;
    for (int i = 0; i < map.size(); i++) {
      if (map[i] == 1) {
        const auto gpos = engine::grid::index_to_grid_position(i, grid.width, grid.height);
        const auto e = create_gameplay(r, EntityType::empty);
        auto& e_transform = r.get<TransformComponent>(e);
        e_transform.position = { gpos.x * grid.size, gpos.y * grid.size, 0.0 };
        grid.debug_cell_ents.push_back(e);
      }
    }
  }

  ImGui::End();
}

} // namespace game2d