#include "system.hpp"

#include "maths/grid.hpp"

#include "imgui.h"

namespace game2d {

void
update_grid_interaction_system(entt::registry& r, const glm::ivec2& mouse_pos)
{
  auto& grid = get_first_component<GridComponent>(r);

  // offset from center to tl of grid
  const glm::ivec2 offset = { grid.size / 2, grid.size / 2 };

  const auto mouse_clamped_pos = engine::grid::world_space_to_clamped_world_space(mouse_pos + offset, grid.size);

  ImGui::Begin("Debug Grid Interaction");

  auto grid_xy = engine::grid::world_space_to_grid_space(mouse_clamped_pos, grid.size);
  auto grid_idx = engine::grid::grid_position_to_index(grid_xy, grid.width);

  ImGui::Text("grid_idx: %i", grid_idx);

  ImGui::End();
}

} // namespace game2d