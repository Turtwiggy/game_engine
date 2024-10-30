#include "move_player_on_map_helpers.hpp"

#include "actors/actor_helpers.hpp"
#include "engine/algorithm_astar_pathfinding/astar_components.hpp"
#include "engine/algorithm_astar_pathfinding/astar_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/maths/grid.hpp"
#include "modules/map/components.hpp"
#include "modules/map/helpers.hpp"
#include "modules/system_move_to_target_via_lerp/components.hpp"

namespace game2d {

void
move_action_lerp_to_neighbour(entt::registry& r, const entt::entity e, const glm::vec2& dst_wp)
{
  const auto map_e = get_first<MapComponent>(r);
  if (map_e == entt::null) {
    SDL_Log("Cant move... no map");
    return; // no map
  }
  const auto& map = r.get<MapComponent>(map_e);

  const auto info = get_entity_mapinfo(r, e);
  const auto src_idx = info->idx_in_map;
  const auto dst_idx = engine::grid::worldspace_to_index(dst_wp, map.tilesize, map.xmax, map.ymax);
  const auto src_gp = engine::grid::index_to_grid_position(src_idx, map.xmax, map.ymax);
  const auto dst_gp = engine::grid::index_to_grid_position(dst_idx, map.xmax, map.ymax);

  const auto area = generate_accessible_areas(r, map, { src_gp.x, src_gp.y }, 1);
  const auto it = std::find(area.begin(), area.end(), dst_gp);
  if (it == area.end())
    return; // not able to reach destination

  const auto next_dst = (*it);
  const auto next_idx = engine::grid::grid_position_to_index(next_dst, map.xmax);

  // move action...
  const bool moved = move_entity_on_map(r, e, next_idx);
  if (!moved) {
    SDL_Log("%s", std::format("move_action_common(): entity didnt move...").c_str());
    return;
  }

  const int a = engine::grid::grid_position_to_index(get_grid_position(r, e), map.xmax);
  const int b = next_idx;

  // Lerp the player model, independent of the grid representation
  const auto offset = glm::vec2{ map.tilesize / 2.0f, map.tilesize / 2.0f };
  remove_if_exists<LerpToFixedTarget>(r, e);
  LerpToFixedTarget lerp;
  lerp.a = engine::grid::index_to_world_position(a, map.xmax, map.ymax, map.tilesize) + offset;
  lerp.b = engine::grid::index_to_world_position(b, map.xmax, map.ymax, map.tilesize) + offset;
  lerp.t = 0.0f;
  r.emplace<LerpToFixedTarget>(e, lerp);
};

void
move_action_follow_path(entt::registry& r, const entt::entity e, const glm::vec2& dst_wp)
{
  const auto map_e = get_first<MapComponent>(r);
  if (map_e == entt::null) {
    SDL_Log("Cant move... no map");
    return; // no map
  }
  const auto& map = r.get<MapComponent>(map_e);

  const auto src_wp = get_position(r, e);
  const auto src_gp = engine::grid::worldspace_to_grid_space(src_wp, 50);
  const auto dst_gp = engine::grid::worldspace_to_grid_space(dst_wp, 50);

  const auto path = generate_direct(r, { src_gp.x, src_gp.y }, { dst_gp.x, dst_gp.y });
  const auto it = std::find(path.begin(), path.end(), dst_gp);
  if (it == path.end())
    return; // not able to reach destination

  GeneratedPathComponent path_c;
  path_c.path = path;
  path_c.src_pos = get_position(r, e);
  path_c.dst_pos = dst_wp;
  path_c.dst_ent = entt::null;
  r.emplace_or_replace<GeneratedPathComponent>(e, path_c);
};

} // namespace game2d
