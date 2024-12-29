#include "move_player_on_map_system.hpp"

#include "actors/actor_helpers.hpp"
#include "engine/algorithm_astar_pathfinding/astar_components.hpp"
#include "engine/entt/entity_pool.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/map/components.hpp"
#include "engine/maths/grid.hpp"
#include "engine/physics/components.hpp"
#include "engine/renderer/transform.hpp"
#include "engine/sprites/helpers.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/spaceship_designer/generation/rooms_random.hpp"
#include "modules/system_move_player_on_map/move_player_on_map_helpers.hpp"
#include "modules/system_move_to_target_via_lerp/components.hpp"


namespace game2d {

bool
inside_ship(entt::registry& r, const entt::entity e)
{
  const auto& map_e = get_first<MapComponent>(r);
  const auto& map = get_first_component<MapComponent>(r);
  const auto gp = get_grid_position(r, e);
  const auto rooms = inside_room(r, gp);
  const bool inside_spaceship = rooms.size() > 0;
  return inside_spaceship;
};

void
update_move_player_on_map_system(entt::registry& r, uint64_t ms_dt)
{
  const float tilesize = 50.0f;
  const auto& ri = get_first_component<SINGLE_RendererInfo>(r);

  //
  // avoid movement if hovering ui
  // move direct i.e. WASD movement around grid
  //
  /*
  if (ri.viewport_hovered) {
    const auto& view =
      r.view<const PlayerComponent, const TransformComponent, InputComponent>(entt::exclude<GeneratedPathComponent>);
    for (const auto& [e, player_c, transform_c, inp_c] : view.each()) {

      const auto& map_e = get_first<MapComponent>(r);
      if (map_e == entt::null)
        continue;
      const auto& map_c = r.get<MapComponent>(map_e);

      if (!inside_ship(r, e)) {
        inp_c.unprocessed_move_down = false;
        continue; // only move if onboard
      }

      if (!inp_c.unprocessed_move_down)
        continue; // no input pressed
      inp_c.unprocessed_move_down = false;

      const auto round_away_from_zero = [](const float value) -> float {
        if (value > 0.0f)
          return std::ceil(value);
        else if (value < 0.0f)
          return std::floor(value);
        else
          return 0.0f;
      };

      const auto wp = glm::vec2{ transform_c.position.x, transform_c.position.y };

      // do the move
      const auto move_position = glm::vec2{
        wp.x + round_away_from_zero(inp_c.lx) * tilesize,
        wp.y + round_away_from_zero(inp_c.ly) * tilesize,
      };

      move_action_lerp_to_neighbour(r, e, move_position);
    }
  }
  */

  //
  // If the player has a generated path component attached,
  // lerp from your current position to the destination position
  //

  static EntityPool pool;

  // ImGui::Begin("Debug Paths");
  {
    const auto& path_view = r.view<GeneratedPathComponent>();

    int i = 0;
    for (const auto& [e, path_c] : path_view.each()) {
      for (const auto& path : path_c.path)
        i++;
    };
    pool.update(r, i);

    for (int i = 0; const auto& [e, path_c] : path_view.each()) {
      // ImGui::Separator();
      for (const auto& p : path_c.path) {
        // ImGui::Text("%i,%i", p.x, p.y);

        const auto debug_e = pool.instances[i];
        auto pos = engine::grid::grid_space_to_world_space_center(p, 50);
        set_position(r, debug_e, pos);
        set_size(r, debug_e, { 50, 50 });
        set_sprite(r, debug_e, "CURSOR_3");

        i++;
      }
    }
  }
  // ImGui::End();

  //
  // Do the actual moving along the path...
  //
  const auto& map_e = get_first<MapComponent>(r);
  if (map_e == entt::null)
    return;
  const auto& map_c = r.get<MapComponent>(map_e);
  const float lerp_speed = 10.0f;
  const auto& view = r.view<const PhysicsBodyComponent, GeneratedPathComponent>();
  for (const auto& [e, body_c, path_c] : view.each()) {

    const auto cur_wsp = glm::vec2{ body_c.body->GetPosition().x, body_c.body->GetPosition().y };
    const auto cur_gp = engine::grid::worldspace_to_grid_space({ cur_wsp.x, cur_wsp.y }, map_c.tilesize);
    const auto cur_tile_wsp = engine::grid::grid_space_to_world_space_center(cur_gp, map_c.tilesize);

    const auto it = std::find(path_c.path.begin(), path_c.path.end(), cur_gp);

    if (it == path_c.path.end())
      continue;

    if (std::next(it) == path_c.path.end())
      continue;

    const auto nxt_gp = *(std::next(it));
    const auto nxt_wsp = engine::grid::grid_space_to_world_space(nxt_gp, map_c.tilesize);

    for (int i = 0; const auto& p : path_c.path) {

      // not on our section in the path
      if (p != cur_gp) {
        i++;
        continue;
      }

      // Move towards the center of the current tile.
      if (!path_c.path_cleared[i]) {
        auto* lerp_maybe = r.try_get<LerpToFixedTarget>(e);
        if (!lerp_maybe) {
          auto& lerp = r.emplace<LerpToFixedTarget>(e);
          lerp.a = cur_wsp;
          lerp.b = cur_tile_wsp;
          lerp.speed = lerp_speed;
        }
      }

      // Check if the path is cleared
      if (!path_c.path_cleared[i]) {
        const auto d = cur_wsp - cur_tile_wsp;
        const float d2 = d.x * d.x + d.y * d.y;
        const float threshold = 6;
        if (d2 <= threshold) {
          path_c.path_cleared[i] = true;

          // stop lerp
          if (auto* lerp = r.try_get<LerpToFixedTarget>(e))
            r.remove<LerpToFixedTarget>(e);
        }
      }

      // need to have cleared the path for the next conditions
      if (!path_c.path_cleared[i])
        break;

      // If we've cleared the current gridtile path,
      // wait for a little bit to simulate "walking" steps
      if (path_c.wait_at_destination && path_c.wait_time_ms_left > 0) {
        path_c.wait_time_ms_left -= ms_dt;
        break;
      }

      // reset the wait time for the next path
      path_c.wait_time_ms_left = path_c.wait_time_ms;

      // aim for the next gridtile path

      auto* lerp_maybe = r.try_get<LerpToFixedTarget>(e);
      if (!lerp_maybe) {
        const auto dst_tile_wsp = engine::grid::grid_space_to_world_space_center(nxt_gp, map_c.tilesize);
        auto& lerp = r.emplace<LerpToFixedTarget>(e);
        lerp.a = cur_wsp;
        lerp.b = dst_tile_wsp;
        lerp.speed = lerp_speed;
      }

      i++;
    }

    //
  }
}

} // namespace game2d