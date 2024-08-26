#include "helpers.hpp"

#include "actors/helpers.hpp"
#include "components.hpp"
#include "entt/helpers.hpp"
#include "maths/grid.hpp"
#include "modules/algorithm_astar_pathfinding/helpers.hpp"


#include <fmt/core.h>

namespace game2d {

void
add_entity_to_map(entt::registry& r, const entt::entity src_e, const int idx)
{
  auto& map = get_first_component<MapComponent>(r);

  if (map.map[idx] != entt::null) {
    fmt::println("add_entity_to_map(): src contains an entity");
    return;
  }

  map.map[idx] = src_e;
};

bool
move_entity_on_map(entt::registry& r, const int idx_a, const int idx_b)
{
  auto& map = get_first_component<MapComponent>(r);

  const entt::entity src_e = map.map[idx_a];
  const entt::entity dst_e = map.map[idx_b];

  if (src_e == entt::null) {
    fmt::println("move_entity_on_map(): src does not contain entity");
    return false;
  }

  if (dst_e != entt::null) {
    fmt::println("move_entity_on_map(): dst not clear for move");
    return false;
  }

  map.map[idx_a] = entt::null;
  map.map[idx_b] = src_e;
  return true;
};

std::vector<glm::ivec2>
generate_path(entt::registry& r, const entt::entity src_e, const glm::ivec2& worldspace_pos, const size_t limit)
{
  const auto& map = get_first_component<MapComponent>(r);

  const auto src_idx = engine::grid::worldspace_to_index(get_position(r, src_e), map.tilesize, map.xmax, map.ymax);
  const auto dst_idx = engine::grid::worldspace_to_index(worldspace_pos, map.tilesize, map.xmax, map.ymax);

  auto path = generate_direct(r, map, src_idx, dst_idx, map.edges);

  // make sure path.size() < limit
  // return +1, as usually the first path[0] is the element the entity is currently standing on
  if (path.size() > limit)
    return { path.begin(), path.begin() + limit + 1 };

  return path;
};

} // namespace game2d