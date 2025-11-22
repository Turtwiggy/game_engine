#include "pch.hpp"

#include "alive_players_components.hpp"
#include "alive_players_system.hpp"
#include "engine/physics/physics_helpers.hpp"
#include "modules/actors/actor_boat/boat_components.hpp"
#include "modules/combat/combat_core/components.hpp"

namespace game2d {

void
update_alive_players_system(entt::registry& r)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif

  auto& alive_players = SINGLE_AlivePlayers::instance;
  alive_players.players.clear();
  for (const auto& [e, boat_c] : r.view<PlayerBoatComponent>().each()) {
    auto fixture_e = get_fixture_by_tag(r, e, "fixture_player");
    auto& hp_c = r.get<HealthComponent>(fixture_e);
    if (hp_c.hp > 0)
      alive_players.players.push_back(e);
  }

  //
}

} // namespace game2d