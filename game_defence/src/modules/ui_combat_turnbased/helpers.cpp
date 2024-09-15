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
  const auto rooms = inside_room(r, gp);
  const bool inside_spaceship = rooms.size() > 0;
  return inside_spaceship;
};

} // namespace game2d