#include "pch.hpp"

#include "ui_scene_upgrades_components.hpp"

namespace game2d {

void
SINGLE_PersistentUpgradesMenuUI::do_init(entt::registry& r)
{
  for (int i = 0; i < (int)UpgradeableStat::count; i++) {

    const auto stat_enum = magic_enum::enum_value<UpgradeableStat>(i);
    const auto stat_str = std::string(magic_enum::enum_name<UpgradeableStat>(stat_enum));

    // stats to skip...
    if (stat_str.find("ACTOR_STAMINA") != std::string::npos)
      continue;

    Cell c;
    c.name = stat_str.c_str();
    c.action = [&]() { selected_stat = stat_enum; };
    state.cells.push_back(std::make_shared<Cell>(c));
  }

  init = true;
}

} // namespace game2d