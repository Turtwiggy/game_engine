#include "system_ai_components.hpp"

#include "actors/actor_helpers.hpp"
#include "engine/algorithm_astar_pathfinding/astar_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/maths/grid.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/combat/components.hpp"
#include "modules/combat_show_tiles_in_range/show_tiles_in_range_components.hpp"
#include "modules/combat_show_tiles_in_range/show_tiles_in_range_helpers.hpp"
#include "modules/map/components.hpp"
#include "modules/system_names/components.hpp"

namespace game2d {

float
MoveConsideration::Evaluate(entt::registry& r, entt::entity e) const
{
  const auto map_e = get_first<MapComponent>(r);
  const auto& map_c = r.get<MapComponent>(map_e);
  const auto src_wp = get_position(r, e);
  const auto src_gp = engine::grid::worldspace_to_grid_space(src_wp, map_c.tilesize);
  const auto src_team = r.get<TeamComponent>(e);

  std::map<int, std::vector<glm::ivec2>> distance_and_path;

  const auto targets_view = r.view<const TeamComponent, const TransformComponent>();
  for (const auto& [other_e, other_team, other_t] : targets_view.each()) {
    if (e == other_e)
      continue;

    if (src_team.team == other_team.team)
      continue; // not interested in same team

    const auto dst_wp = glm::vec2{ other_t.position.x, other_t.position.y };
    const auto d = dst_wp - src_wp;
    const auto d2 = d.x * d.x + d.y * d.y;
    const auto dst_gp = engine::grid::worldspace_to_grid_space(dst_wp, map_c.tilesize);
    const auto path = generate_direct_with_diagonals(r, src_gp, dst_gp);

    if (path.size() != 0) {
      // note: remove the end tile so that the ai doesnt path on top of the destination entity
      std::vector<glm::ivec2> path_without_end_tile{ path.begin(), path.end() - 1 };
      distance_and_path.emplace(d2, path_without_end_tile);
    }
  }

  MoveConsiderationData data_c;

  // move to the closest enemy entity
  // take the path with the shortest distance (value at beginning of map)
  if (distance_and_path.size() > 0)
    data_c.final_path = distance_and_path.begin()->second;

  r.emplace_or_replace<MoveConsiderationData>(e, data_c);
  if (data_c.final_path.size() > 0)
    return 0.75f;

  return 0.0f;
};

float
AttackConsideration::Evaluate(entt::registry& r, entt::entity e) const
{
  // make sure we've got the latest tiles around the entity...
  auto& tiles_c = r.get_or_emplace<TilesComponent>(e);
  const auto& map_c = get_first_component<MapComponent>(r);
  const auto& input_c = r.get<InputComponent>(e);
  tiles_c.tiles = get_tiles_for_knife(r, map_c, get_grid_position(r, e));

  AttackConsiderationData data_c;

  // A lot of utility in attacking if something is in range
  const auto& your_team = r.get<TeamComponent>(e);
  int enemies_around = 0;
  for (const auto& tile : tiles_c.tiles) {
    const auto idx = engine::grid::grid_position_to_index(tile, map_c.xmax);
    for (const auto map_e : map_c.map[idx]) {
      const auto& other_team = r.get<TeamComponent>(map_e);
      SDL_Log("Considering attacking... %s", r.get<NameComponent>(map_e).first_name.c_str());

      if (your_team.team == other_team.team)
        continue; // dont attack same team?

      if (other_team.team == AvailableTeams::neutral)
        continue; // dont attack neutrals?

      // attack enemies only
      data_c.targets.push_back(map_e);
    }
  }

  r.emplace_or_replace<AttackConsiderationData>(e, data_c);
  if (data_c.targets.size() > 0)
    return 1.0f;

  return 0.0f;
};

} // namespace game2d