#include "pch.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/maths/grid.hpp"
#include "engine/maths/maths.hpp"
#include "islander_remove_from_island_helpers.hpp"
#include "modules/actors/actor_rock/rock_components.hpp"

namespace game2d {

void
handle_death_event__islander_remove_from_island(entt::registry& r, const DeathEvent& evt)
{
  auto dead_e = evt.dead;

  const auto tilesize = SINGLE_Islands::instance.tilesize;
  const auto pos = get_position(r, dead_e);
  const auto pos_adj = pos - glm::vec2{ tilesize * 0.5f, tilesize * 0.5f };
  const auto gp = engine::grid::worldspace_to_gridspace(pos_adj, tilesize);
  const auto id = engine::encode_cantor_pairing_function(gp.x, gp.y);

  // figure out what island they were on.
  const auto island_e = SINGLE_Islands::instance.id_to_island_eid.at(id);
  auto& island_c = r.get<DebugContoursComponent>(island_e);

  // remove them from the island
  const auto it = std::find_if(island_c.occupied_island_xy.begin(),
                               island_c.occupied_island_xy.end(),
                               [&dead_e](const auto& other) { return other.second == dead_e; });

  if (it == island_c.occupied_island_xy.end()) {
    SDL_Log("Warning: something died, but cant find it on the island.");
    return;
  }

  SDL_Log("something died! removing %i %i from island %zu", gp.x, gp.y, (uint32_t)island_e);
  island_c.occupied_island_xy.erase(it);
}

} // namespace game2d