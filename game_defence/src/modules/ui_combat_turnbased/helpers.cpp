#include "helpers.hpp"

#include "actors/helpers.hpp"
#include "modules/gen_dungeons/components.hpp"
#include "modules/gen_dungeons/helpers.hpp"

#include "entt/helpers.hpp"
#include "modules/grid/components.hpp"

namespace game2d {

bool
inside_ship(entt::registry& r, const entt::entity e)
{
  const auto& map_e = get_first<MapComponent>(r);
  const auto& map = get_first_component<MapComponent>(r);
  const auto& results_e = get_first<DungeonGenerationResults>(r);
  const auto& results = r.get<DungeonGenerationResults>(results_e);

  const auto gp = get_grid_position(r, e);

  const auto [in_room, room] = inside_room(map, results.rooms, gp);
  const auto in_tunnel = inside_tunnels(results.tunnels, gp).size() > 0;
  const bool inside_spaceship = in_room || in_tunnel;

  return inside_spaceship;
};

} // namespace game2d