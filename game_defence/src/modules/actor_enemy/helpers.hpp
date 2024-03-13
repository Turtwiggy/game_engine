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

//
//  Try to perform reciprical velocity obstacle (RVO) avoidance
//  https://gamma.cs.unc.edu/HRVO/HRVO-T-RO.pdf
//
void
update_velocity_based_on_rvo()
{
  // const auto& vel_view = r.view<const AABB, VelocityComponent, const EnemyComponent>();

  // for (const auto& [e, enemy, target_position, enemy_t, vel, aabb] : view.each()) {

  // future optimisation: we do not need to take all the other agents in to account when
  // selecting a new velocity, as the penalty of the velocities will
  // not depend on all agents that are far away.

  // {
  //   for (const auto& [b_e, b_aabb, b_vel, b_enemy] : vel_view.each()) {
  //     if (e == b_e)
  //       continue; // avoid yourself, bruh

  //     const auto pos_a = glm::vec2(aabb.center.x, aabb.center.y);     // x: 100
  //     const auto vel_a = glm::vec2(vel.x, vel.y);                     // x: -10
  //     const auto pos_b = glm::vec2(b_aabb.center.x, b_aabb.center.y); // x: -100
  //     const auto vel_b = glm::vec2(b_vel.x, b_vel.y);                 // x: 10
  //     const auto relative_pos = pos_b - pos_a;                        // -200, 0

  //     // assume circles for the minkowski sum
  //     const auto combined_radius = ((aabb.size.x / 2.0f) + (b_aabb.size.x / 2.0f)) * 2.0f; // 32

  //     // if this dot product is positive, they're heading towards eachother
  //     // if its negative, they're heading away from each other (no possible collision)
  //     //
  //     const auto relative_vel = vel_a - vel_b; // -10, 0
  //     const auto dot_product = glm::dot(relative_pos, relative_vel);
  //     ImGui::Text("DotProduct: %f", dot_product);
  //     if (dot_product < 0)
  //       continue; // not headed for a collision, continue along preferred velocity#

  //     // warning: time to collision doesnt take in to account radius
  //     // const auto speed = glm::length(relative_vel);
  //     // const auto time_to_collision = dot_product / (speed * speed);
  //     // ImGui::Text("ttc: %f", time_to_collision);

  //     // Calculate the Velocity Obstacle
  //     //
  //     const auto distance = glm::length(relative_pos);
  //     const auto angle = atan2(relative_pos.y, relative_pos.x); // angle to x-axis
  //     const auto tangent_angle = atan(combined_radius / distance);
  //     const auto tangent_angle_left = angle + tangent_angle;
  //     const auto tangent_angle_right = angle - tangent_angle;
  //     auto tangent_point_left = pos_a;
  //     tangent_point_left.x += cos(tangent_angle_left) * distance;
  //     tangent_point_left.y += sin(tangent_angle_left) * distance;
  //     auto tangent_point_right = pos_a;
  //     tangent_point_right.x += cos(tangent_angle_right) * distance;
  //     tangent_point_right.y += sin(tangent_angle_right) * distance;
  //     // TODO: calculate the far side of the velocity object

  //     // Translate the VO in to a RVO.
  //     //
  //     const glm::vec2 translation = (vel_a + vel_b) / 2.0f;
  //     const auto a1 = pos_a + translation;
  //     const auto a2 = tangent_point_left + translation;
  //     const auto b1 = pos_a + translation;
  //     const auto b2 = tangent_point_right + translation;
  //     const auto c1 = tangent_point_left + translation;  // should be far side
  //     const auto c2 = tangent_point_right + translation; // should be far side
  //     // set_transform_with_line(r.get<TransformComponent>(enemy.cone_l), generate_line(a1, a2, 2));
  //     // set_transform_with_line(r.get<TransformComponent>(enemy.cone_r), generate_line(b1, b2, 2));
  //     // set_transform_with_line(r.get<TransformComponent>(enemy.cone_far), generate_line(c1, c2, 2));

  //     // Pick a velocity outside the RVO
  //     //
  //     const auto dir_left = glm::normalize(a2 - a1);
  //     const auto dir_right = glm::normalize(b2 - b1);
  //     const auto nrm_vel = glm::normalize(vel_a);

  //     // Is the current velocity already outside the RVO?
  //     //
  //     const auto valid_vel_check_a = glm::dot(nrm_vel, dir_left) < 0;
  //     const auto valid_vel_check_b = glm::dot(nrm_vel, dir_right) < 0;
  //     if (valid_vel_check_a && valid_vel_check_b)
  //       continue; // no need to adjust velocity, velocity is fine

  //     // Otherwise, pick a new velocity outside of the RVO.
  //     //
  //     // When choosing a new velocity,
  //     // the average is taken of its current velocity
  //     // and a velocity outside the veloicty object.
  //     // Robots are typically required to select the velocity closest to their own preferred velocity.

  //     // TODO...
  //     vel.x = dir_left.x * vel.base_speed;
  //     vel.y = dir_left.y * vel.base_speed;
  //   }
}

} // namespace game2d