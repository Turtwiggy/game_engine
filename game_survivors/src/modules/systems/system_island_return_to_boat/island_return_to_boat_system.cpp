#include "pch.hpp"

#include "island_return_to_boat_system.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/maths/grid.hpp"
#include "engine/maths/maths.hpp"
#include "engine/renderer/transform.hpp"
#include "engine/std/vector/helpers.hpp"
#include "modules/actors/actor_islanddweller/islanddweller_components.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/actors/actor_rock/rock_components.hpp"
#include "modules/systems/system_island_movement/island_movement_components.hpp"

namespace game2d {

void
update_island_return_to_boat_system(entt::registry& r)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif

  auto& dead = get_first_component<SINGLE_EntityBinComponent>(r);

  const auto view = r.view<const MovementIslandComponent, const TransformComponent, const InputComponent>();
  for (const auto& [e, movement_c, t_c, input_c] : view.each()) {

    const bool return_to_boat = has(input_c.button_e, ActionStateEnum::DOWN);
    if (!return_to_boat)
      continue;

    // remove the player.
    dead.dead.push_back(e);

    // set the tile as "unoccupied"
    const int tilesize = SINGLE_Islands::instance.tilesize;
    const auto pos = glm::vec2{ t_c.position.x, t_c.position.y };
    const auto pos_adj = pos - glm::vec2{ tilesize * 0.5f, tilesize * 0.5f };
    const auto gp = engine::grid::worldspace_to_gridspace(pos_adj, tilesize);
    const auto id = engine::encode_cantor_pairing_function(gp.x, gp.y);
    const auto& all_islands_c = SINGLE_Islands::instance;
    const auto island_e = all_islands_c.id_to_island_eid.at(id);
    auto& island_c = r.get<DebugContoursComponent>(island_e);
    const auto it = std::find_if(island_c.occupied_island_xy.begin(),
                                 island_c.occupied_island_xy.end(),
                                 [&](const auto& other) { return other.second == e; });
    island_c.occupied_island_xy.erase(it);

    // add back control to your boat.
    auto boat_e = movement_c.boat_e;
    r.remove<DroppedAnchorComponent>(boat_e);
    r.emplace<MovementDirectComponent>(boat_e);

    //
  }
}

} // namespace game2d