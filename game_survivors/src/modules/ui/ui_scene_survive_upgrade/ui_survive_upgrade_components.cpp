#include "pch.hpp"

#include "ui_survive_upgrade_components.hpp"

#include "engine/entt/helpers.hpp"

namespace game2d {

std::pair<float, std::string>
get_stat_from_stat_table(entt::registry& r, Rarity rarity, UpgradeableStat upgrade)
{

  const auto rarity_str = std::string(magic_enum::enum_name(rarity));
  const auto upgrade_str = std::string(magic_enum::enum_name(upgrade));

  const auto& stat_table = get_first_component<SINGLE_UpgradeToValue>(r);
  for (const auto& stat : stat_table.upgrades) {
    if (stat.stat == upgrade_str) {
      for (const auto& [rarity, value] : stat.values) {
        if (rarity == rarity_str)
          return { value, stat.type };
      }
    }
  }

  // return { 0, "stat_flat_increase" };
  const auto err_str = std::format("Not impl: {}, {}", rarity_str, upgrade_str);
  throw std::runtime_error(err_str.c_str());

  //
}

} // namespace game2d