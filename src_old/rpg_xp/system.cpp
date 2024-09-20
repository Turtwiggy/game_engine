#include "system.hpp"

#include "components.hpp"
#include "engine/entt/helpers.hpp"
#include "helpers.hpp"
#include "modules/ui_rpg_character/components.hpp"

#include <entt/entt.hpp>

namespace game2d {

void
update_rpg_xp_system(entt::registry& r) {
  //
  // what do I want this system to do?

  // When the player gains a level,
  // give the player an option to level a stat.
  //

  // Gives the player XP from a killed enemy
  // const auto& xp_view = r.view<IsDead, XpComponent>();
  // for (auto [entity, dead, xp] : xp_view.each()) {
  //   const auto& player_entity = get_first<PlayerComponent>(r);
  //   auto& player_xp = r.get<XpComponent>(player_entity);
  //   player_xp.amount += xp.amount;
  //   std::string msg = "You gained xp! " + std::to_string(xp.amount);
  //   game.ui_events.events.push_back(msg);
  // }

  // // This view uses (and validates) the WantsToLevelStat provided by the UI
  // const auto& stats_view = r.view<WantsToLevelStat, StatsComponent, XpComponent>();
  // for (auto [entity, lv_stats_by, stats, xp] : stats_view.each()) {

  //   const int xp_level = convert_xp_to_level(xp.amount);
  //   const int cur_level = stats.overall_level;
  //   int allowed_to_level = glm::max(xp_level - cur_level, 0);

  //   for (const auto& con : lv_stats_by.con) {
  //     if (allowed_to_level > 0) {
  //       stats.con_level += 1;
  //       stats.overall_level += 1;
  //       allowed_to_level--;
  //     }
  //   }
  //   for (const auto& str : lv_stats_by.str) {
  //     if (allowed_to_level > 0) {
  //       stats.str_level += 1;
  //       stats.overall_level += 1;
  //       allowed_to_level--;
  //     }
  //   }
  //   for (const auto& agi : lv_stats_by.agi) {
  //     if (allowed_to_level > 0) {
  //       stats.agi_level += 1;
  //       stats.overall_level += 1;
  //       allowed_to_level--;
  //     }
  //   }

  //   r.remove<WantsToLevelStat>(entity);
  //   r.remove<WantsToLevelUp>(entity);
  // }

  // // This view informs the UI that a level up is required
  // const auto& view = r.view<XpComponent, StatsComponent>();
  // for (auto [entity, xp, stats] : view.each()) {
  //   int xp_level = convert_xp_to_level(xp.amount);
  //   if (xp_level != stats.overall_level)
  //     r.emplace_or_replace<WantsToLevelUp>(entity);
  // }
};

} // namespace game2d