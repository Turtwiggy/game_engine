#pragma once

namespace game2d {

// void
// update_enemy_get_new_target(const GridComponent& grid, const TransformComponent& transform, EnemyComponent& enemy)
// {

//   const glm::ivec2 offset = { grid.size / 2, grid.size / 2 };
//   const glm::vec2 offset_pos = { transform.position.x + offset.x, transform.position.y + offset.y };
//   const auto grid_xy = engine::grid::world_space_to_grid_space(offset_pos, grid.size);

//   // auto grid_idx = engine::grid::grid_position_to_index(grid_xy, grid.width);
//   // grid_idx = glm::clamp(grid_idx, 0, (grid.width * grid.height) - 1);
//   // auto clamped_grid_pos = engine::grid::index_to_world_position(grid_idx, grid.width, grid.height, grid.size);
//   // auto clamped_grid_xy = engine::grid::index_to_grid_position(grid_idx, grid.width, grid.height);

//   std::vector<std::pair<engine::grid::GridDirection, int>> results;
//   engine::grid::get_neighbour_indicies(grid_xy.x, grid_xy.y, grid.width, grid.height, results);

//   int best_distance = INT_MAX;
//   auto best_dir = results[0].first;
//   auto best_idx = results[0].second;
//   const auto& field = grid.flow_field;

//   for (const auto& [dir, idx] : results) {
//     const int distance = field[idx].distance;
//     if (distance < best_distance) {
//       best_distance = distance;
//       best_dir = dir;
//       best_idx = idx;
//     }
//   }

//   std::cout << "getting new target..." << std::endl;

//   // center of the next grid square
//   glm::ivec3 target_position{ grid_xy.x * grid.size, grid_xy.y * grid.size, 0 };

//   // update the spot
//   if (best_dir == engine::grid::GridDirection::east)
//     target_position.x += grid.size;
//   if (best_dir == engine::grid::GridDirection::west)
//     target_position.x -= grid.size;
//   if (best_dir == engine::grid::GridDirection::north)
//     target_position.y -= grid.size;
//   if (best_dir == engine::grid::GridDirection::south)
//     target_position.y += grid.size;

//   enemy.target = target_position;
//   enemy.has_target = true;
// }

// void
// enemy_ai::enemy_arc_angles_to_player(GameObject2D& obj, GameObject2D& player, float delta_time_s)
// {
//   // calculate a vector ab
//   glm::vec2 ab = player.pos - obj.pos;
//   // calculate the point halfway between ab
//   glm::vec2 half_point = obj.pos + (ab / 2.0f);
//   // calculate the vector at a right angle
//   glm::vec2 normal = glm::vec2(-ab.y, ab.x);

//   // expensive(?) distance calc
//   float distance = glm::distance(obj.pos, player.pos);
//   float half_distance = distance / 2.0f;

//   // offset the midpoint via normal
//   float amplitude = half_distance * sin(glm::radians(obj.approach_theta_degrees));
//   half_point += (glm::normalize(normal) * amplitude);

//   glm::vec2 dir = glm::normalize(half_point - obj.pos);

//   move_along_vector(obj, dir, delta_time_s);
// };

} // namespace game2d