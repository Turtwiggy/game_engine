#include "pch.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/io/settings.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/system_upgrade/upgrade_components.hpp"
#include "persistent_upgrade_helpers.hpp"

namespace game2d {

struct StatInfo
{
  std::string key;
  std::string type;
  float val;
};

void
load_persistent_upgrades_and_apply_to_player(entt::registry& r)
{
  GET_FIRST_OR_RETURN(SINGLE_Upgrades, r, upgrade_e, upgrade_c);

  std::vector<StatInfo> stats;

  for (int i = 0; i < (int)UpgradeableStat::count; i++) {
    const auto stat_enum = magic_enum::enum_value<UpgradeableStat>(i);
    const auto stat_str = std::string(magic_enum::enum_name<UpgradeableStat>(stat_enum));

    auto val_opt = get_string(stat_str);
    if (!val_opt.has_value())
      continue;
    SDL_Log("You've purchased an upgrade... %s", stat_str.c_str());

    // Load how much the stat gives from the upgrades.
    const auto find_by_key = [&stat_str](Upgrade& u) { return u.key == stat_str; };
    const auto it = std::find_if(upgrade_c.upgrades.begin(), upgrade_c.upgrades.end(), find_by_key);
    if (it == upgrade_c.upgrades.end()) {
      // You've purchased an upgrade that isnt in the SINGLE_Upgrades.upgrades. CURIOUS
      continue;
    }

    const Upgrade u = (*it);
    const int your_level = std::stoi(val_opt.value());
    for (int lv_idx = 0; lv_idx < your_level; lv_idx++) {
      auto lv_info = u.levels[lv_idx];

      StatInfo stat_info;
      stat_info.key = u.key;
      stat_info.type = lv_info.type; // stat or flat percent
      stat_info.val = lv_info.value;
      stats.push_back(stat_info);
    }
  }

  for (const auto& [e, player_c, stat_c] : r.view<const PlayerComponent, StatModifierComponent>().each()) {
    // add to stats

    for (const auto& stat_info : stats) {

      const bool add_flat = stat_info.type == "stat_flat_increase";
      const bool add_percent = stat_info.type == "stat_percent_increase";
      const auto mod_key = stat_info.key;
      const auto mod_val = stat_info.val;

      if (add_flat)
        stat_c.add(std::make_shared<StatFlatIncrease>(mod_val, mod_key));
      else if (add_percent)
        stat_c.add(std::make_shared<StatPercentIncrease>(mod_val, mod_key));

      //
    }
  }
}

} // namespace game2d