#include "helpers.hpp"

#include "engine/algorithm_astar_pathfinding/astar_components.hpp"
#include "engine/maths/grid.hpp"
#include "generation/components.hpp"
#include "generation/rooms_random.hpp"
#include "modules/components/raws.hpp"
#include "modules/map/components.hpp"
#include "modules/map/helpers.hpp"

namespace game2d {

void
generate_edges(entt::registry& r, MapComponent& map, const DungeonGenerationResults& result)
{
  const auto& rooms = result.rooms;

  std::vector<Edge> edges;

  for (int idx = 0; idx < map.xmax * map.ymax; idx++) {
    const auto xy = engine::grid::index_to_grid_position(idx, map.xmax, map.ymax);
    const auto neighbours = engine::grid::get_neighbour_gridpos(xy, map.xmax, map.ymax);
    for (const auto& [dir, neighbour_gp] : neighbours) {
      const auto grid_a = xy;
      const auto grid_b = neighbour_gp;

      // 3 cases to generate edges for walls:
      // transition from wall to floor
      // transition from room to room
      // you're in a room and on the edge of the grid

      Edge edge;
      edge.gp_a = grid_a;
      edge.gp_b = grid_b;

      const auto rooms_a = inside_room(r, grid_a);
      const auto rooms_b = inside_room(r, grid_b);
      const bool in_room_a = rooms_a.size() > 0;
      const bool in_room_b = rooms_b.size() > 0;
      const bool from_room_to_room = (in_room_a && in_room_b) && (rooms_a != rooms_b);

      // check if you're in a room and on the edge of the grid
      // grid_a has to be in bounds, as that's generated off the index
      // grid_b could be out of bounds, as that's the neighbour grid position
      bool in_bounds = engine::grid::grid_position_in_bounds(grid_b, map.xmax, map.ymax);
      if (!in_bounds && in_room_a) {
        edges.push_back(edge);
        continue;
      }

      if (!in_bounds)
        continue;
      const auto neighbour_idx = engine::grid::grid_position_to_index(neighbour_gp, map.xmax);

      const bool idx_is_wall = result.wall_or_floors[idx] == 1;
      const bool nidx_is_floor = result.wall_or_floors[neighbour_idx] == 0;
      const bool from_wall_to_floor = idx_is_wall && nidx_is_floor;

      if (from_wall_to_floor)
        edges.push_back(edge);

      // doors...
      // if (from_room_to_room)
      //   edges.push_back(edge);
    }
  }

  // std::sort for std::unique
  const auto p = [&map](const Edge& a, const Edge& b) {
    int a_idx_a = engine::grid::grid_position_to_index(a.gp_a, map.xmax);
    int a_idx_b = engine::grid::grid_position_to_index(a.gp_b, map.xmax);
    int b_idx_a = engine::grid::grid_position_to_index(b.gp_a, map.xmax);
    int b_idx_b = engine::grid::grid_position_to_index(b.gp_b, map.xmax);

    // either:
    // sort by the first gridpoint in the edge,
    // or if they're equal sort by the second grid point
    return (a_idx_a < b_idx_a) || (a_idx_a == b_idx_a && a_idx_b < b_idx_b);
  };
  std::sort(edges.begin(), edges.end(), p);

  const auto pred = [](const Edge& a, const Edge& b) { return a == b; };
  edges.erase(std::unique(edges.begin(), edges.end(), pred), edges.end());

  // Once they're sorted and unqiue... create entt representation
  for (const auto& edge : edges) {
    auto e = create_transform(r, "edge");
    r.emplace<Edge>(e, edge);
  }
};

void
instantiate_edges(entt::registry& r, MapComponent& map)
{
  for (const auto& [e, edge_c] : r.view<Edge>().each()) {
    const glm::vec2 ga = engine::grid::grid_space_to_world_space_center(edge_c.gp_a, map.tilesize);
    const glm::vec2 gb = engine::grid::grid_space_to_world_space_center(edge_c.gp_b, map.tilesize);

    // debug edge
    // const auto l0 = generate_line({ ga.x, ga.y }, { gb.x, gb.y }, 4);
    // const entt::entity e0 = create_gameplay(r, EntityType::empty_with_transform, { 0, 0 });
    // set_position_and_size_with_line(r, e0, l0);
    // set_colour(r, e0, { 1.0f, 0.0, 0.0, 0.5f });

    const auto center = (gb + ga) / 2.0f;
    const auto size = glm::abs(gb - ga);
    const bool was_horizontal = size.x > size.y;

    auto new_size = glm::vec2{ size.y, size.x };
    if (new_size.x == 0)
      new_size.x = 2;
    if (new_size.y == 0)
      new_size.y = 2;

    spawn_wall(r, "default", center, new_size);
  }
};

void
update_map_with_pathfinding(entt::registry& r, MapComponent& map, DungeonGenerationResults& result)
{
  for (int idx = 0; idx < map.xmax * map.ymax; idx++) {
    if (result.wall_or_floors[idx] == 1) {

      const auto e = create_empty<PathfindComponent>(r, { PathfindComponent{ -1 } });

      if (map.map[idx].size() != 0) {
        fmt::println("ERROR: wall index isnt empty");
        exit(1); // crash
      }

      add_entity_to_map(r, e, idx);
    }
  }
};

void
generate_floors(entt::registry& r, MapComponent& map, DungeonGenerationResults& result)
{
  result.floor_tiles.resize(result.wall_or_floors.size(), entt::null);
  for (size_t xy = 0; xy < result.wall_or_floors.size(); xy++) {
    if (result.wall_or_floors[xy] == 0) {
      const auto pos = engine::grid::index_to_world_position_center((int)xy, map.xmax, map.ymax, map.tilesize);
      const auto gp = engine::grid::index_to_grid_position((int)xy, map.xmax, map.ymax);
      const auto floor_e = spawn_floor(r, "default", pos, { map.tilesize, map.tilesize });
      const auto floor_idx = engine::grid::grid_position_to_index(gp, map.xmax);
      result.floor_tiles[floor_idx] = floor_e;
    }
  }
};

} // namespace game2d