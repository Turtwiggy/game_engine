#include "spaceship_designer_helpers.hpp"

#include "actors/actor_helpers.hpp"
#include "engine/algorithm_astar_pathfinding/astar_components.hpp"
#include "engine/maths/grid.hpp"
#include "engine/physics/helpers.hpp"
#include "engine/renderer/transform.hpp"
#include "engine/sprites/components.hpp"
#include "engine/sprites/helpers.hpp"
#include "generation/components.hpp"
#include "generation/rooms_random.hpp"
#include "modules/actor_airlock/components.hpp"
#include "modules/actor_door/components.hpp"
#include "modules/colour/components.hpp"
#include "modules/map/components.hpp"
#include "modules/map/helpers.hpp"
#include "modules/raws/raws_components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/helpers.hpp"
#include "modules/renderer/lights/components.hpp"

namespace game2d {

const auto remove_duplicates = [](std::vector<Edge>& edges) {
  // std::sort for std::unique
  const auto p = [](const Edge& a, const Edge& b) {
    if (a.a != b.a)
      return ivec2_less(a.a, b.a);
    return ivec2_less(a.b, b.b);
  };
  std::sort(edges.begin(), edges.end(), p);

  const auto pred = [](const Edge& a, const Edge& b) { return a == b; };
  edges.erase(std::unique(edges.begin(), edges.end(), pred), edges.end());
};

const auto check_for_duplicates = [](entt::registry& r) {
  const auto& view = r.view<Edge>();

  std::vector<Edge> processed;

  // Once they're sorted and unqiue... create entt representation
  for (const auto& [e, edge] : view.each()) {

    auto dupl_it = std::find(processed.begin(), processed.end(), edge);
    if (dupl_it != processed.end())
      fmt::println("gen error: duplicate edge...");

    processed.push_back(edge);
  }
};

const auto give_edges_to_entt = [](entt::registry& r, const std::vector<Edge>& edges, const std::vector<Edge>& doors) {
  for (const auto& edge : edges) {
    // create the edge in entt
    auto e = create_empty<Edge>(r, edge);

    // some edges are doors...
    auto it = std::find_if(doors.begin(), doors.end(), [&edge](const Edge& other) { return other == edge; });
    if (it != doors.end())
      r.emplace<DoorComponent>(e, DoorComponent{ edge });
  }
};

void
generate_edges(entt::registry& r, MapComponent& map, const DungeonIntermediate& result)
{
  std::vector<Edge> edges;
  std::vector<Edge> doors;

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

      const Edge edge = Edge(grid_a, grid_b);
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

      const bool idx_is_wall = result.floor_types[idx] == FloorType::WALL;
      const bool nidx_is_floor = result.floor_types[neighbour_idx] == FloorType::FLOOR;
      const bool from_wall_to_floor = idx_is_wall && nidx_is_floor;

      if (from_wall_to_floor) {
        edges.push_back(edge);
        continue;
      }

      if (from_room_to_room) {
        edges.push_back(edge);
        doors.push_back(edge);
      }
    }
  }

  remove_duplicates(edges);
  give_edges_to_entt(r, edges, doors);
  check_for_duplicates(r);
};

void
generate_airlocks(entt::registry& r, MapComponent& map, const DungeonIntermediate& result)
{
  for (int idx = 0; idx < map.xmax * map.ymax; idx++) {
    const auto xy = engine::grid::index_to_grid_position(idx, map.xmax, map.ymax);
    const auto neighbours = engine::grid::get_neighbour_gridpos(xy, map.xmax, map.ymax);

    const bool idx_is_airlock = result.floor_types[idx] == FloorType::AIRLOCK;
    if (!idx_is_airlock)
      continue;

    AirlockComponent airlock_c;

    for (const auto& [dir, neighbour_gp] : neighbours) {
      const auto edge = Edge(xy, neighbour_gp);
      const auto nidx = engine::grid::grid_position_to_index(neighbour_gp, map.xmax);

      // create edge in all 4 directions
      const auto e = create_empty<Edge>(r, edge);

      // set the airlock to have 2 doors: north and south
      if (dir == engine::grid::GridDirection::north) {
        r.emplace<DoorComponent>(e, DoorComponent{ edge });
        airlock_c.north_edge_copy = edge;
        airlock_c.door_north = e;
      } else if (dir == engine::grid::GridDirection::south) {
        r.emplace<DoorComponent>(e, DoorComponent{ edge });
        airlock_c.south_edge_copy = edge;
        airlock_c.door_south = e;
      }
    }

    //
    const auto pos = engine::grid::index_to_world_position_center((int)idx, map.xmax, map.ymax, map.tilesize);
    const auto gp = engine::grid::index_to_grid_position((int)idx, map.xmax, map.ymax);
    const auto e = create_transform(r, "airlock");
    r.emplace<DefaultColour>(e, engine::SRGBColour{ 1.0f, 1.0f, 1.0f, 1.0f });
    r.emplace<SpriteComponent>(e);
    set_sprite(r, e, "EMPTY");
    set_position(r, e, pos);
    set_size(r, e, { map.tilesize, map.tilesize });
    set_colour(r, e, r.get<DefaultColour>(e).colour);
    set_z_index(r, e, ZLayer::BACKGROUND);
    r.emplace<AirlockComponent>(e, airlock_c);
    map.map[engine::grid::grid_position_to_index(gp, map.xmax)].push_back(e);

    // Make it it's own room?
    Room room;
    room.tiles_idx = { engine::grid::grid_position_to_index(gp, map.xmax) };
    r.emplace<Room>(e, room);
    r.emplace<RoomName>(e, RoomName{ "Airlock" });
  }

  instantiate_edges(r, map);
  check_for_duplicates(r);
};

void
instantiate_edge(entt::registry& r, entt::entity e, const MapComponent& map_c)
{
  const auto& edge_c = r.get<Edge>(e);
  const glm::vec2 ga = engine::grid::grid_space_to_world_space_center(edge_c.a, map_c.tilesize);
  const glm::vec2 gb = engine::grid::grid_space_to_world_space_center(edge_c.b, map_c.tilesize);

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
    new_size.x = 4;
  if (new_size.y == 0)
    new_size.y = 4;

  // spawn_wall()
  // convert Edge to Wall (i.e. add physics and stuff)
  r.emplace<TransformComponent>(e);
  r.emplace<SpriteComponent>(e);
  set_sprite(r, e, "EMPTY");
  set_size(r, e, new_size);
  create_physics_actor_static(r, e, center, new_size);

  if (auto* door_c = r.try_get<DoorComponent>(e))
    set_colour(r, e, { 1.0f, 0.0f, 0.0f, 1.0f });
  else
    set_colour(r, e, { 1.0f, 1.0f, 1.0f, 1.0f });

  r.emplace_or_replace<LightOccluderComponent>(e);
  // r.emplace<DestroyBulletOnCollison>(e);
  // r.emplace<RequestParticleOnCollision>(e);
};

void
instantiate_edges(entt::registry& r, const MapComponent& map)
{
  for (const auto& [e, edge_c] : r.view<const Edge>(entt::exclude<TransformComponent>).each())
    instantiate_edge(r, e, map);
};

void
update_map_with_pathfinding(entt::registry& r, MapComponent& map, DungeonIntermediate& result)
{
  for (int idx = 0; idx < map.xmax * map.ymax; idx++) {
    if (result.floor_types[idx] == FloorType::WALL) {

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
instantiate_floors(entt::registry& r, MapComponent& map, DungeonIntermediate& result)
{
  for (size_t xy = 0; xy < result.floor_types.size(); xy++) {
    if (result.floor_types[xy] == FloorType::FLOOR) {
      const auto pos = engine::grid::index_to_world_position_center((int)xy, map.xmax, map.ymax, map.tilesize);
      const auto gp = engine::grid::index_to_grid_position((int)xy, map.xmax, map.ymax);
      const auto floor_e = spawn_floor(r, "default", pos, { map.tilesize, map.tilesize });

      map.map[engine::grid::grid_position_to_index(gp, map.xmax)].push_back(floor_e);
    }
  }
};

} // namespace game2d