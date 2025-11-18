#include "pch.hpp"

#include "islander_components.hpp"
#include "islander_helpers.hpp"

#include "engine/maths/grid.hpp"
#include "engine/maths/maths.hpp"
#include "modules/actors/actor_rock/rock_components.hpp"
#include "modules/combat/combat_core/components.hpp"
#include "modules/core/raws/raws_components.hpp"
#include "modules/systems/system_island_ai/island_ai_components.hpp"
#include "modules/systems/system_island_movement/island_movement_components.hpp"
#include "modules/systems/system_island_nearest/island_nearest_helpers.hpp"

namespace game2d {

entt::entity
spawn_islander_unoccupied(entt::registry& r,
                          engine::RandomState& rnd,
                          const entt::entity island_e,
                          const std::string tag,
                          const AvailableTeams team,
                          const bool has_brain)
{
  const auto tilesize_unit = default_map_unit_tilesize;
  const auto tilesize_map = SINGLE_Islands::instance.tilesize;

  auto& island_c = r.get<DebugContoursComponent>(island_e);

  const auto unoccupied = get_unoccupied_tiles(island_c);
  const auto xy = unoccupied[(int)engine::rand_det_s(rnd.rng, 0, (int)unoccupied.size())];
  const auto thing_e = spawn(r, tag);
  auto pos = engine::grid::gridspace_to_worldspace(xy, tilesize_map);
  pos += glm::vec2{ tilesize_map, tilesize_map }; // off grid
  give_life(r, thing_e, pos, { tilesize_unit, tilesize_unit });
  r.emplace<IslandDwellerComponent>(thing_e);
  r.emplace<HealthComponent>(thing_e, HealthComponent{ 2, 2 });
  r.emplace<TeamComponent>(thing_e, TeamComponent{ .team = team });

  // let the thing move
  // add brains to enemies
  r.emplace<MovementIslandComponent>(thing_e, MovementIslandComponent{ .island_e = island_e });
  if (has_brain)
    r.emplace<IslanderAiComponent>(thing_e);

  island_c.occupied_island_xy.push_back({ xy, thing_e });
  return thing_e;
};

entt::entity
spawn_islander_unoccupied_edge(entt::registry& r,
                               engine::RandomState& rnd,
                               const entt::entity island_e,
                               const std::string tag,
                               const AvailableTeams team,
                               const bool has_brain)
{
  const auto tilesize_unit = default_map_unit_tilesize;
  const auto tilesize_map = SINGLE_Islands::instance.tilesize;
  auto& island_c = r.get<DebugContoursComponent>(island_e);

  const auto unoccupied = get_unoccupied_edge_tiles(island_c);
  const auto xy = unoccupied[(int)engine::rand_det_s(rnd.rng, 0, (int)unoccupied.size())];
  const auto thing_e = spawn(r, tag);
  auto pos = engine::grid::gridspace_to_worldspace(xy, tilesize_map);
  pos += glm::vec2{ tilesize_map, tilesize_map }; // off grid
  give_life(r, thing_e, pos, { tilesize_unit, tilesize_unit });
  r.emplace<IslandDwellerComponent>(thing_e);
  r.emplace<HealthComponent>(thing_e, HealthComponent{ 2, 2 });
  r.emplace<TeamComponent>(thing_e, TeamComponent{ .team = team });

  // let the thing move
  // add brains to enemies
  r.emplace<MovementIslandComponent>(thing_e, MovementIslandComponent{ .island_e = island_e });
  if (has_brain)
    r.emplace<IslanderAiComponent>(thing_e);

  island_c.occupied_island_xy.push_back({ xy, thing_e });
  return thing_e;
};

} // namespace game2d