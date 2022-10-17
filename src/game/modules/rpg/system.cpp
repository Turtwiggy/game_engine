#include "system.hpp"

#include "components.hpp"
#include "helpers.hpp"

#include <entt/entt.hpp>

namespace game2d {

void
update_rpg_system(GameEditor& editor, Game& game)
{
  auto& r = game.state;
  //
  // what do I want this system to do?

  // When the player gains a level,
  // give the player an option to level a stat.
  //

  const auto& view = r.view<XpComponent, StatsComponent>();
  for (auto [entity, xp, stats] : view.each()) {
    int xp_level = convert_xp_to_level(xp.amount);
    if (xp_level != stats.overall_level)
      r.emplace_or_replace<WantsToLevelUp>(entity);
  }
};

} // namespace game2d