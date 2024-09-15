#include "system.hpp"

#include "actors/actors.hpp"
#include "components.hpp"

#include "actors/helpers.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "events/helpers/keyboard.hpp"
#include "imgui/helpers.hpp"
#include "maths/grid.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/gen_dungeons/components.hpp"
#include "modules/gen_dungeons/helpers.hpp"
#include "modules/grid/components.hpp"
#include "modules/system_fov/symmetric_shadowcasting.hpp"
#include <modules/algorithm_astar_pathfinding/helpers.hpp>

#include "imgui.h"
#include <fmt/core.h>

namespace game2d {
using namespace std::literals;

void
update_fov_system(entt::registry& r, const glm::ivec2& mouse_pos)
{
  const auto& map = get_first_component<MapComponent>(r);
  const auto& dungeon = get_first_component<DungeonGenerationResults>(r);

  const auto player_e = get_first<PlayerComponent>(r);
  if (player_e == entt::null)
    return;
  const auto player_pos = get_position(r, player_e);
  const auto player_gridpos = engine::grid::worldspace_to_grid_space(player_pos, map.tilesize);

  const auto rooms = inside_room(r, player_gridpos);
  const bool in_room = rooms.size() > 0;
  const bool outside_ship = !in_room;

  std::vector<int> walls_or_floors_adjusted = dungeon.wall_or_floors; // copy

  int cur_idx = 0;
  static int break_idx = 0;
  const auto input = get_first_component<SINGLETON_InputComponent>(r);
  if (get_key_down(input, SDL_SCANCODE_KP_7))
    break_idx--;
  if (get_key_down(input, SDL_SCANCODE_KP_8))
    break_idx++;

  static bool debug_fov = false;

  if (debug_fov) {
    imgui_draw_bool("debug_fov", debug_fov);
    for (const entt::entity& floor_e : dungeon.floor_tiles)
      if (floor_e != entt::null) {
        set_colour(r, floor_e, { 0.05f, 0.05f, 0.05f, 1.0f });
        // set_z_index(r, floor_e, ZLayer::FOREGROUND);
      }

    ImGui::Text("break_idx %i", break_idx);
  }

  if (!debug_fov) {
    // prevent out of bounds issues

    if (outside_ship)
      return;

    // Generate request when player moves gridpos
    static entt::entity cached_player_e = entt::null;
    static glm::ivec2 cached_origin{ 0, 0 };
    if (player_e != cached_player_e) {
      create_empty<RequestUpdateFOV>(r);
      cached_player_e = player_e;
    }
    if (cached_player_e != entt::null) {
      const auto gp = get_grid_position(r, cached_player_e);
      if (cached_origin != gp) {
        create_empty<RequestUpdateFOV>(r);
        cached_origin = gp;
      }
    }

    // only update fov when needed
    const auto reqs = r.view<RequestUpdateFOV>();
    if (reqs.size() == 0)
      return;
    r.destroy(reqs.begin(), reqs.end());
  }

  const glm::ivec2 origin = player_gridpos;

  static std::vector<entt::entity> edge_debug;
  r.destroy(edge_debug.begin(), edge_debug.end());
  edge_debug.clear();

  // adjust this vector based on edges
  for (int dir = 0; dir < 4; dir++) {
    const auto has_edge = [&](const Tile& t0, const Tile& t1) -> entt::entity {
      const auto [x, y] = transform(t0, dir, origin);
      const auto [prev_x, prev_y] = transform(t1, dir, origin);
      return edge_between_gps(r, { x, y }, { prev_x, prev_y });
    };
    const auto is_type = [&](const Tile& t, const TileType& type) -> bool {
      const auto [x, y] = transform(t, dir, origin);
      const auto idx = engine::grid::grid_position_to_index({ x, y }, map.xmax);
      if (idx < 0)
        return false;
      return walls_or_floors_adjusted[idx] == static_cast<int>(type);
    };
    const auto is_wall = [&](const Tile& t) -> bool { return is_type(t, TileType::WALL); };
    const auto is_floor = [&](const Tile& t) -> bool { return is_type(t, TileType::FLOOR); };

    Row first_row;
    first_row.depth = 1;
    first_row.start_slope = -1.0f;
    first_row.end_slope = 1.0f;
    std::vector<Row> rows{ first_row };

    while (!rows.empty()) {
      Row row = rows.back();
      rows.pop_back();
      std::optional<Tile> prev_tile = std::nullopt;

      const int min_col = round_up(row.depth * row.start_slope);
      const int max_col = round_down(row.depth * row.end_slope);
      for (int i = min_col; i < (max_col + 1); i++) { // row.tiles()

        if (debug_fov) {
          cur_idx++;
          if (cur_idx > break_idx)
            break;
        }

        const Tile tile(row.depth, i);
        const auto [x, y] = transform(tile, dir, origin);
        const auto idx = engine::grid::grid_position_to_index({ x, y }, map.xmax);

        const Tile above_tile(tile.depth - 1, i);
        const auto [abv_x, abv_y] = transform(above_tile, dir, origin);
        const bool invalid_above = above_tile.depth == 0 && above_tile.col != 0;
        const auto edge = has_edge(tile, above_tile);
        const bool edge_exists = edge != entt::null;

        const Tile tile_l(tile.depth, i + 1);
        const Tile tile_r(tile.depth, i - 1);
        const auto [r_x, r_y] = transform(tile_r, dir, origin);
        const bool invalid_right = tile_r.col > max_col;
        const auto r_edge = has_edge(tile, tile_r);
        const bool r_edge_exists = r_edge != entt::null;

        // edge between red and white
        // edge between blue and white
        const Tile tile_ur(tile.depth - 1, i - 1);
        const auto [ur_x, ur_y] = transform(tile_ur, dir, origin);
        const auto edge_a = has_edge(tile_r, tile_ur);
        const auto edge_b = has_edge(above_tile, tile_ur);
        const bool ur_edge_exists = edge_a != entt::null && edge_b != entt::null;

        // edge between red and white
        // edge between blue and white
        const Tile tile_ul(tile.depth - 1, i + 1);
        const auto [ul_x, ul_y] = transform(tile_ul, dir, origin);
        const auto edge_ul_a = has_edge(tile_l, tile_ul);
        const auto edge_ul_b = has_edge(above_tile, tile_ul);
        const bool ul_edge_exists = edge_ul_a != entt::null && edge_ul_b != entt::null;

        if (debug_fov && cur_idx == break_idx) {
          ImGui::Text("cur_idx %i break_idx: %i", cur_idx, break_idx);

          const auto r_idx = engine::grid::grid_position_to_index({ r_x, r_y }, map.xmax);
          const auto abv_idx = engine::grid::grid_position_to_index({ abv_x, abv_y }, map.xmax);
          const auto ur_idx = engine::grid::grid_position_to_index({ ur_x, ur_y }, map.xmax);
          const auto ul_idx = engine::grid::grid_position_to_index({ ul_x, ul_y }, map.xmax);

          if (dungeon.floor_tiles.size() > 0 && dungeon.floor_tiles[idx] != entt::null)
            set_colour(r, dungeon.floor_tiles[idx], { 1.0f, 1.0f, 1.0f, 1.0f });

          if (!invalid_above && dungeon.floor_tiles.size() > 0 && dungeon.floor_tiles[abv_idx] != entt::null)
            set_colour(r, dungeon.floor_tiles[abv_idx], { 0.0f, 0.5f, 1.0f, 1.0f });

          if (!invalid_right && dungeon.floor_tiles.size() > 0 && dungeon.floor_tiles[r_idx] != entt::null)
            set_colour(r, dungeon.floor_tiles[r_idx], { 0.0f, 0.0f, 1.0f, 1.0f });

          // if (dungeon.floor_tiles.size() > 0 && dungeon.floor_tiles[ur_idx] != entt::null)
          //   set_colour(r, dungeon.floor_tiles[ur_idx], { 1.0f, 1.0f, 1.0f, 1.0f }); // white

          // if (dungeon.floor_tiles.size() > 0 && dungeon.floor_tiles[ul_idx] != entt::null)
          //   set_colour(r, dungeon.floor_tiles[ul_idx], { 0.0f, 1.0f, 1.0f, 1.0f }); // cyan

          const auto debug_edge = [&r, &map](Edge edge) {
            const auto offset = glm::vec2{ map.tilesize / 2.0f, map.tilesize / 2.0f };
            const auto ga = engine::grid::grid_space_to_world_space_center(edge.gp_a, map.tilesize) + offset;
            const auto gb = engine::grid::grid_space_to_world_space_center(edge.gp_b, map.tilesize) + offset;
            const auto l = generate_line({ ga.x, ga.y }, { gb.x, gb.y }, 1);
            const entt::entity e = create_transform(r);
            set_position_and_size_with_line(r, e, l);
            set_colour(r, e, { 1.0f, 1.0f, 1.0f, 1.0f });
            r.get<TagComponent>(e).tag = "debugline";
            edge_debug.push_back(e);
          };

          if (!invalid_above && edge_exists)
            debug_edge(r.get<Edge>(edge));
          if (!invalid_right && r_edge_exists)
            debug_edge(r.get<Edge>(r_edge));
          if (ur_edge_exists)
            debug_edge({ { ur_x, ur_y }, { x, y } });
          if (ul_edge_exists)
            debug_edge({ { ul_x, ul_y }, { x, y } });
        }

        // If tile isnt a wall,
        // but there is an edge between tile and above tile,
        if (!invalid_above && edge_exists && !is_wall(tile))
          walls_or_floors_adjusted[idx] = 1;

        const auto furthest_tile = [&dir, &origin](const Tile& a, const Tile& b) -> Tile {
          const auto [ax, ay] = transform(a, dir, origin);
          const auto [bx, by] = transform(b, dir, origin);
          const auto a_tile = origin - glm::ivec2{ ax, ay };
          const auto b_tile = origin - glm::ivec2{ bx, by };
          const auto d2_tile_a = a_tile.x * a_tile.x + a_tile.y * a_tile.y;
          const auto d2_tile_b = b_tile.x * b_tile.x + b_tile.y * b_tile.y;
          return d2_tile_a > d2_tile_b ? a : b;
        };

        // if tile isnt a wall,
        // and tile_r isnt a wall,
        // and there is an edge between tile and tile_r
        if (!invalid_right && r_edge_exists && !is_wall(tile) && !is_wall(tile_r)) {
          const auto further_tile = furthest_tile(tile, tile_r);
          const auto [fx, fy] = transform(further_tile, dir, origin);
          const auto fidx = engine::grid::grid_position_to_index({ fx, fy }, map.xmax);
          walls_or_floors_adjusted[fidx] = 1;
        }

        // corners
        if (ur_edge_exists && !is_wall(tile) && !is_wall(tile_ur)) {
          const auto further_tile = furthest_tile(tile, tile_ur);
          const auto [x, y] = transform(further_tile, dir, origin);
          const auto idx = engine::grid::grid_position_to_index({ x, y }, map.xmax);
          walls_or_floors_adjusted[idx] = 1;
        }
        if (ul_edge_exists && !is_wall(tile) && !is_wall(tile_ul)) {
          const auto further_tile = furthest_tile(tile, tile_ul);
          const auto [x, y] = transform(further_tile, dir, origin);
          const auto idx = engine::grid::grid_position_to_index({ x, y }, map.xmax);
          walls_or_floors_adjusted[idx] = 1;
        }

        if (prev_tile.has_value() && is_wall(prev_tile.value()) && is_floor(tile))
          row.start_slope = slope(tile);

        if (prev_tile.has_value() && is_floor(prev_tile.value()) && is_wall(tile)) {
          Row next_row = next(row);
          next_row.end_slope = slope(tile);
          rows.push_back(next_row);
        }

        prev_tile = tile;
      }

      if (prev_tile.has_value() && is_floor(prev_tile.value()))
        rows.push_back(next(row)); // scan(row.next())
    }
  }

  // stop everything being visible
  const auto vview = r.view<VisibleComponent>();
  r.remove<VisibleComponent>(vview.begin(), vview.end());

  //
  // symemtric shadowcasting
  //
  const auto visible_idxs = do_shadowcasting(r, origin, walls_or_floors_adjusted);

  // Update the floor colouring
  for (const auto idx : visible_idxs) {
    if (dungeon.floor_tiles[idx] != entt::null) {
      r.emplace_or_replace<VisibleComponent>(dungeon.floor_tiles[idx]);
    }
  }

  // remove visible from adjusted floors=>walls,
  // usually walls are visible, but because we've adjusted
  // it to be "behind" an edge, that square is no longer visible
  for (size_t i = 0; i < dungeon.wall_or_floors.size(); i++) {
    const auto floor_e = dungeon.floor_tiles[i];
    if (walls_or_floors_adjusted[i] == 1 && dungeon.wall_or_floors[i] == 0 && floor_e != entt::null) {
      if (debug_fov)
        set_colour(r, floor_e, { 1.0f, 0.0f, 0.0f, 1.0f });
      if (auto* visible_c = r.try_get<VisibleComponent>(floor_e))
        r.remove<VisibleComponent>(floor_e);
    }
  }

  // mark origin visible
  const int origin_idx = engine::grid::grid_position_to_index(origin, map.xmax);
  mark_visible(r, dungeon.floor_tiles[origin_idx]);
  // mark_visible(r, map.map[origin_idx]);

  // change colour of all floor tiles to visible state
  if (!debug_fov) {
    for (const entt::entity& floor_e : dungeon.floor_tiles) {
      if (floor_e == entt::null)
        continue;
      const bool is_visible = r.try_get<VisibleComponent>(floor_e) != nullptr;
      const auto floor_pos = get_position(r, floor_e);
      const auto floor_idx = engine::grid::worldspace_to_index(floor_pos, map.tilesize, map.xmax, map.ymax);

      // Mark the entity in the map as visible
      // if they're standing on a visible floor
      if (is_visible) {
        for (const auto e : map.map[floor_idx])
          mark_visible(r, e);
      }
      // if (is_visible) {
      //   set_colour(r, floor_e, r.get<DefaultColour>(floor_e).colour);
      // } else
      //   set_colour(r, floor_e, { 0.35f, 0.35f, 0.35f, 1.0f });
    }
  }

  if (debug_fov) {
    ImGui::Begin("DebugFoV");
    imgui_draw_int("break_idx"s, break_idx);
    imgui_draw_int("cur_idx"s, cur_idx);

    const auto grid_pos = engine::grid::worldspace_to_grid_space(mouse_pos, map.tilesize);
    auto mouse_idx = engine::grid::grid_position_to_index(grid_pos, map.xmax);
    mouse_idx = glm::clamp(mouse_idx, 0, map.xmax * map.ymax - 1);
    ImGui::Text("mouse: %i is_wall_or_floor: %i", mouse_idx, walls_or_floors_adjusted[mouse_idx]);

    const auto floor_e = dungeon.floor_tiles[mouse_idx];
    if (floor_e != entt::null && r.try_get<VisibleComponent>(floor_e) != nullptr)
      ImGui::Text("is_visible: 1");
    else
      ImGui::Text("is_visible: 0");
    ImGui::End();
  }
}

} // namespace game2d
