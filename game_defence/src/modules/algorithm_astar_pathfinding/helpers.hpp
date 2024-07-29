#pragma once

#include "modules/algorithm_astar_pathfinding/components.hpp"
#include "modules/grid/components.hpp"

#include <functional>
#include <glm/glm.hpp>

#include <optional>
#include <vector>

namespace game2d {

template<typename T>
int
distance(const T& a, const T& b)
{
  int distance_x = a.x < b.x ? b.x - a.x : a.x - b.x;
  int distance_y = a.y < b.y ? b.y - a.y : a.y - b.y;
  return (distance_x + distance_y);
};

template<typename T>
bool
equal(const T& a, const T& b)
{
  return (a.x == b.x) && (a.y == b.y);
};

template<typename T>
int
heuristic(const T& a, const T& b)
{
  return distance<T>(a, b);
};

[[nodiscard]] std::vector<glm::ivec2>
generate_direct(entt::registry& r,
                const MapComponent& grid,
                const int from_idx,
                const int to_idx,
                const std::optional<std::vector<Edge>> edges = std::nullopt);

[[nodiscard]] std::vector<glm::ivec2>
generate_direct_with_diagonals(entt::registry& r, const MapComponent& grid, const int from_idx, const int to_idx);

[[nodiscard]] std::vector<glm::ivec2>
generate_accessible_areas(entt::registry& r, const MapComponent& grid, const int from_idx, const int range);

[[nodiscard]] std::vector<astar_cell>
generate_flow_field(entt::registry& r, const MapComponent& grid, const int from_idx);

void
display_flow_field_with_visuals(entt::registry& r, MapComponent& grid);

[[nodiscard]] int
convert_position_to_index(const MapComponent& map, const glm::ivec2& pos);

bool
has_destination(entt::registry& r, const entt::entity& src_e);

bool
at_destination(entt::registry& r, const entt::entity& e);

bool
destination_is_blocked(entt::registry& r, const glm::ivec2 worldspace_pos);

} // namespace game2d