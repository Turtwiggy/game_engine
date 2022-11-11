#include "stats.hpp"

#include "game/components/actors.hpp"
#include "game/modules/combat/components.hpp"
#include "game/modules/rpg_xp/components.hpp"

namespace game2d {

void
update_stats_system(GameEditor& editor, Game& game)
{
  auto& r = game.state;

  const auto& stats_view = r.view<StatsComponent, HealthComponent, EntityTypeComponent>();
  stats_view.each([](auto entity, auto& stats, auto& hp, auto& et) {
    //
    if (et.type == EntityType::actor_player)
      hp.max_hp = (stats.con_level * 4);
    else
      hp.max_hp = stats.con_level;

    if (hp.hp > hp.max_hp)
      hp.hp = hp.max_hp;
  });
}

} // namespace game2d

// An "Attack" is basically a limiter that prevents collisions
// applying damage on every frame.
