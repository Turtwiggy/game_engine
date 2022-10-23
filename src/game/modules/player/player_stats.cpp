#include "player_stats.hpp"

#include "game/modules/combat/components.hpp"
#include "game/modules/player/components.hpp"
#include "game/modules/rpg_xp/components.hpp"

namespace game2d {

void
update_player_stats_system(GameEditor& editor, Game& game)
{
  auto& r = game.state;

  const auto& stats_view = r.view<PlayerComponent, StatsComponent, HealthComponent>();
  stats_view.each([](auto entity, auto& player, auto& stats, auto& hp) {
    //
    hp.max_hp = 9 + stats.con_level;
  });
}

} // namespace game2d

// An "Attack" is basically a limiter that prevents collisions
// applying damage on every frame.
