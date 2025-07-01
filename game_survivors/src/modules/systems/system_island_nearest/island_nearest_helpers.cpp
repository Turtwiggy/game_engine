#include "pch.hpp"

#include "island_nearest_helpers.hpp"

#include "engine/maths/grid.hpp"
#include "modules/actors/actor_islanddweller/islanddweller_components.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/actors/actor_rock/rock_components.hpp"
#include "modules/combat/combat_core/components.hpp"
#include "modules/core/raws/raws_components.hpp"
#include "modules/steam_input/steam_input_components.hpp"
#include "modules/systems/system_island_movement/island_movement_components.hpp"

namespace game2d {

void
land_player_on_island(entt::registry& r,
                      DebugContoursComponent& island_c,
                      const glm::ivec2 gp,
                      entt::entity boat_e,
                      entt::entity island_e)
{
  const int tilesize = SINGLE_Islands::instance.tilesize;

  auto pos = engine::grid::gridspace_to_worldspace(gp, tilesize);
  pos += glm::vec2{ tilesize, tilesize };

  // Spawn the player at the open space.
  auto island_player_e = spawn(r, "actor_islanddweller_player");
  give_life(r, island_player_e, pos, { tilesize, tilesize });
  r.emplace<PlayerComponent>(island_player_e);
  r.emplace<TeamComponent>(island_player_e, TeamComponent{ AvailableTeams::player });
  r.emplace<HealthComponent>(island_player_e, HealthComponent{ .max_hp = 3, .hp = 3 });

  // Add inputs to the island dweller.
  r.emplace<MovementIslandComponent>(island_player_e,
                                     MovementIslandComponent{
                                       .island_e = island_e,
                                       .boat_e = boat_e,
                                     });
  if (r.all_of<SteamControllerComponent>(boat_e))
    r.emplace<SteamControllerComponent>(island_player_e, r.get<SteamControllerComponent>(boat_e));
  if (r.all_of<KeyboardComponent>(boat_e))
    r.emplace<KeyboardComponent>(island_player_e, r.get<KeyboardComponent>(boat_e));

  // Remove inputs from the boat.
  r.remove<MovementDirectComponent>(boat_e);
  r.emplace_or_replace<DroppedAnchorComponent>(boat_e);
  // r.remove<InputComponent>(e); // dont remove input component

  // set the tile as occupied.
  island_c.occupied_island_xy.push_back({ gp, island_player_e });
};

std::vector<glm::ivec2>
get_unoccupied_tiles(const DebugContoursComponent& island_c)
{
  const auto& all = island_c.all_island_xy;
  const auto& occupied = island_c.occupied_island_xy;

  std::vector<glm::ivec2> unoccupied;
  for (const auto& xy : all) {
    auto it = std::find_if(occupied.begin(), occupied.end(), [&xy](const auto& p) { return p.first == xy; });
    if (it != occupied.end())
      continue;
    unoccupied.push_back(xy);
  }

  return unoccupied;
};

bool
occupied(entt::registry& r, const DebugContoursComponent& island_c, glm::ivec2 gp)
{
  const auto at_xy = [&gp](const std::pair<glm::ivec2, entt::entity>& data) { return data.first == gp; };
  const auto it = std::find_if(island_c.occupied_island_xy.begin(), island_c.occupied_island_xy.end(), at_xy);
  return it != island_c.occupied_island_xy.end(); // tile is occupied
};

entt::entity
e_at_xy(entt::registry& r, const DebugContoursComponent& island_c, glm::ivec2 gp)
{
  const auto it = std::find_if(island_c.occupied_island_xy.begin(),
                               island_c.occupied_island_xy.end(),
                               [&](const auto& other) { return other.first == gp; });
  if (it == island_c.occupied_island_xy.end())
    return entt::null;
  return (*it).second;
};

} // namespace game2d