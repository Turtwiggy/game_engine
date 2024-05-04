#include "system.hpp"

#include "components.hpp"
#include "entt/helpers.hpp"
#include "helpers/entity_pool.hpp"
#include "helpers/line.hpp"
#include "lifecycle/components.hpp"
#include "maths/grid.hpp"
#include "maths/maths.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/actors/helpers.hpp"
#include "modules/ai_pathfinding/components.hpp"
#include "modules/ai_pathfinding/helpers.hpp"
#include "modules/grid/components.hpp"
#include "modules/selected_interactions/components.hpp"
#include "physics/components.hpp"
#include "renderer/transform.hpp"

#include "imgui.h"

namespace game2d {

// choose a new random position
// move towards position
// this behaviour is interrupted if a player gets within range

void
update_actor_enemy_patrol_system(entt::registry& r, const glm::ivec2 mouse_pos, const float dt)
{
  const auto& map_e = get_first<MapComponent>(r);
  if (map_e == entt::null)
    return;
  const auto& map = get_first_component<MapComponent>(r);

  const auto& player_e = get_first<PlayerComponent>(r);
  if (player_e == entt::null)
    return;
  const auto& target_aabb = r.get<AABB>(player_e);

  ImGui::Begin("Debug__ActorEnemyPatrolSystem");

  // TODO: BAD. FIX.
  static engine::RandomState rnd;

  // Convert Map to Grid (?)
  GridComponent grid;
  grid.size = map.tilesize;
  grid.width = map.xmax;
  grid.height = map.ymax;
  grid.grid = map.map;

  const static auto convert_position_to_gridspace = [&map](const glm::ivec2 pos) -> glm::ivec2 {
    auto gridpos = engine::grid::world_space_to_grid_space(pos, map.tilesize);
    gridpos.x = glm::clamp(gridpos.x, 0, map.xmax - 1);
    gridpos.y = glm::clamp(gridpos.y, 0, map.ymax - 1);
    return gridpos;
  };
  const static auto convert_position_to_index = [&map](const glm::ivec2& pos) -> int {
    const auto gridpos = convert_position_to_gridspace(pos);
    return engine::grid::grid_position_to_index(gridpos, map.xmax);
  };
  const static auto clamp_angle_between_0_and_2PI = [](const float angle) -> float {
    if (angle < 0.0f)
      return angle + 2.0f * engine::PI;
    if (angle > 2.0f * engine::PI)
      return angle - 2.0f * engine::PI;
    return angle;
  };

  const auto& view = r.view<PatrolComponent, const AABB, const VelocityComponent>(entt::exclude<WaitForInitComponent>);
  for (const auto& [e, patrol_c, patrol_aabb, patrol_vel] : view.each()) {
    //
    const auto src = patrol_aabb.center;
    const auto dst = target_aabb.center;
    const auto src_idx = convert_position_to_index(src);
    const int dst_idx = convert_position_to_index(dst);

    const auto update_path_to_player = [&r, &e, &grid, &src_idx, &dst_idx, &src, &dst, &player_e]() {
      const auto path = generate_direct_with_diagonals(r, grid, src_idx, dst_idx);
      GeneratedPathComponent path_c;
      path_c.path = path;
      path_c.src_pos = src;
      path_c.dst_pos = dst;
      path_c.dst_ent = player_e;
      path_c.path_cleared.resize(path.size());
      r.emplace_or_replace<GeneratedPathComponent>(e, path_c);
    };

    // Attach a GeneratedPath to the enemy unit. HasTargetPosition gets overwritten.
    const auto update_path_to_rnd_idx = [&r, &e, &grid, &src_idx, &src, &map]() {
      // should be any valid tiles, but for the moment, just choose a random one
      const int grid_max = (grid.width * grid.height) - 1;
      const int dst_idx = int(engine::rand_det_s(rnd.rng, 0, grid_max));
      const auto dst = engine::grid::index_to_world_position(dst_idx, map.xmax, map.ymax, map.tilesize);

      const auto path = generate_direct(r, grid, src_idx, dst_idx);
      GeneratedPathComponent path_c;
      path_c.path = path;
      path_c.src_pos = src;
      path_c.dst_pos = dst;
      path_c.dst_ent = entt::null;
      path_c.path_cleared.resize(path.size());
      r.emplace_or_replace<GeneratedPathComponent>(e, path_c);
    };

    const auto update_path_to_neighbour_idx = [&r, &e, &grid, &src_idx, &src, &map]() {
      const auto src_gridpos = convert_position_to_gridspace(src);

      // get a random neighbour
      std::vector<std::pair<engine::grid::GridDirection, int>> results;
      get_neighbour_indicies(src_gridpos.x, src_gridpos.y, map.xmax, map.ymax, results);
      const int rnd_neighbour_idx = int(engine::rand_det_s(rnd.rng, 0, results.size()));
      const int dst_idx = results[rnd_neighbour_idx].second;
      const auto dst = engine::grid::index_to_world_position(dst_idx, map.xmax, map.ymax, map.tilesize);

      const auto path = generate_direct(r, grid, src_idx, dst_idx);
      GeneratedPathComponent path_c;
      path_c.path = path;
      path_c.src_pos = src;
      path_c.dst_pos = dst;
      path_c.dst_ent = entt::null;
      path_c.path_cleared.resize(path.size());
      r.emplace_or_replace<GeneratedPathComponent>(e, path_c);
    };

    // Calculate distance from player & chase the player if you're near to the player.
    const auto dir = glm::vec2(target_aabb.center) - glm::vec2(patrol_aabb.center);
    const int d2 = dir.x * dir.x + dir.y * dir.y;
    constexpr int distance_threshold_to_chase = 200 * 200;

    if (d2 < distance_threshold_to_chase) {

      // where is our target currently on the grid?
      const auto dst = target_aabb.center;
      const auto dst_idx = convert_position_to_index(dst);

      // check the vision cone for the p atrol
      const glm::vec2 dir = target_aabb.center - patrol_aabb.center;

      // clamp angle between 0 and 2*PI
      float angle_of_velocity = engine::dir_to_angle_radians({ patrol_vel.x, patrol_vel.y }) + engine::PI;
      angle_of_velocity = clamp_angle_between_0_and_2PI(angle_of_velocity);

      // clamp angle between 0 and 2*PI
      float angle_of_player = engine::dir_to_angle_radians(dir) + engine::PI;
      angle_of_player = clamp_angle_between_0_and_2PI(angle_of_player);
      ImGui::Text("Dir %f %f", dir.x, dir.y);
      ImGui::Text("Angle of velocity: %f", angle_of_velocity * engine::Rad2Deg);
      ImGui::Text("Angle of player: %f", angle_of_player * engine::Rad2Deg);

      // check if we're already chasing the player...
      const auto& existing_path = r.try_get<GeneratedPathComponent>(e);
      if (existing_path && existing_path->dst_ent == player_e) {
        // already chasing the player...

        // has the player moved grid index from the path we've generated?
        // problems:
        // - the below generates better patcauses "jitter" when the playhs, but er moves gridspace.
        // - if we don't do below, the target will continue heading to their destination but the player might have moved
        const auto path_dst_idx = convert_position_to_index(existing_path->dst_pos);
        if (dst_idx != path_dst_idx)
          update_path_to_player();
      } else {
        // we're in range and not chasing the player...
        set_colour(r, e, { 255, 165, 0, 1.0f }); // orange; a player is near

        //
        // hack:
        // force velocity right so that we can debug backstabbing
        //
        // auto& b_vel = r.get<VelocityComponent>(e);
        // b_vel.x = 5;
        // b_vel.y = 0;

        // assume the patrol is looking towards their velocity.
        // if the player comes across the vision cone, chase them
        const float half_cone_angle = 20;
        const float angle_of_vel_min = angle_of_velocity - (half_cone_angle * engine::Deg2Rad);
        const float angle_of_vel_max = angle_of_velocity + (half_cone_angle * engine::Deg2Rad);
        const bool player_in_vision_cone = angle_of_player >= angle_of_vel_min && angle_of_player <= angle_of_vel_max;
        ImGui::Text("Player in vision cone: %i", player_in_vision_cone);
        ImGui::Text("angle_of_vel_min: %f", angle_of_vel_min * engine::Rad2Deg);
        ImGui::Text("angle_of_vel_max: %f", angle_of_vel_max * engine::Rad2Deg);

        // Here, we're in range of the player, but we do not have a path.
        if (player_in_vision_cone) {
          set_colour(r, e, { 255, 0, 0, 1.0f }); // red; attack!!

          update_path_to_player(); // change course, scotty! we've spotted a whale!
        }

        // Work out if we're on target for a backstab
        // if the player is approaching from 180 degrees opposite to velocity,
        // and add some buffer angles.
        float ideal_backstab_angle = angle_of_velocity + (180.0f * engine::Deg2Rad);
        ideal_backstab_angle = clamp_angle_between_0_and_2PI(ideal_backstab_angle);
        const float half_backstab_angle = 20;
        const float backstab_angle_min = ideal_backstab_angle - (half_backstab_angle * engine::Deg2Rad);
        const float backstab_angle_max = ideal_backstab_angle + (half_backstab_angle * engine::Deg2Rad);
        const bool on_target_for_backstab = angle_of_player >= backstab_angle_min && angle_of_player <= backstab_angle_max;

        // you're onboard for a collision, and you're approaching from the behind.
        if (on_target_for_backstab) {
          set_colour(r, e, { 0, 165, 0, 1.0f }); // green; you're gonna backstab it
          r.emplace_or_replace<BackstabbableComponent>(e);
        } else
        // you're not going to collide, or approaching from the side or front.
        {
          if (const auto* b = r.try_get<BackstabbableComponent>(e))
            r.remove<BackstabbableComponent>(e);
        }
      }
    } // end being in range of the player.
    else {
      // out of range of the player.
      set_colour(r, e, { 0, 200, 200, 1.0f }); // blueish; search mode

      const auto& existing_path = r.try_get<GeneratedPathComponent>(e);

      // was chasing a player but that player is now out of range
      if (existing_path && existing_path->dst_ent != entt::null)
        update_path_to_rnd_idx();

      // was not chasing a player, generate a path
      if (!existing_path)
        update_path_to_rnd_idx();
    }

    const auto& existing_path = r.try_get<GeneratedPathComponent>(e);

    // we've reached the end of the path, but we wern't chasing a player.
    if (existing_path && existing_path->dst_ent == entt::null) {
      const auto current_dst_idx = convert_position_to_index(existing_path->dst_pos);
      if (src_idx == current_dst_idx)
        update_path_to_rnd_idx();
    }

    // you have no existing path?
    if (!existing_path)
      update_path_to_rnd_idx();
  }

  // HACK: debug velocity
  {
    static EntityPool velocity_transforms;
    const auto& vel_view = r.view<const VelocityComponent, const TransformComponent>(entt::exclude<PlayerComponent>);

    int count = 0;
    for (const auto& [e, vel, t] : vel_view.each())
      count++;
    velocity_transforms.update(r, count); // size_hint() does not seem to be good

    for (int i = 0; const auto& [e, vel, t] : vel_view.each()) {
      const auto& generated_e = velocity_transforms.instances[i];
      const int vel_extra = 5; // elongate
      const glm::ivec2 pos = t.position;
      const glm::ivec2 pos_with_vel = { t.position.x + vel.x * vel_extra, t.position.y + vel.y * vel_extra };
      set_transform_with_line(r, generated_e, generate_line(pos, pos_with_vel, 2));
      i++;
    }
  }

  ImGui::End();
}

} // namespace game2d