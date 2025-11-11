#include "pch.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/maths/grid.hpp"
#include "engine/maths/maths.hpp"
#include "modules/core/io/io_helpers.hpp"
#include "modules/core/ui/ui_common_helpers.hpp"
#include "modules/systems/system_persistent_upgrades/persistent_upgrade_components.hpp"
#include "ui_scene_upgrades_components.hpp"
#include "ui_scene_upgrades_helpers.hpp"

namespace game2d {
using namespace engine::grid;

void
SINGLE_PersistentUpgradesMenuUI::do_init(entt::registry& r)
{
  const auto upgrade_e = get_first<SINGLE_PersistentUpgrades>(r);
  if (upgrade_e == entt::null)
    return;
  auto& upgrade_c = r.get<SINGLE_PersistentUpgrades>(upgrade_e);

  for (int i = 0; i < (int)UpgradeableStat::count; i++) {

    const auto stat_enum = magic_enum::enum_value<UpgradeableStat>(i);
    const auto stat_str = std::string(magic_enum::enum_name<UpgradeableStat>(stat_enum));

    // stats to skip...
    if (stat_enum == UpgradeableStat::ACTOR_STAMINA)
      continue;

    // if there are no persistentt upgrades, skip the stat.
    const auto [aquired, total] = get_upgrade_level(r, upgrade_c, stat_str);
    if (total == 0)
      continue;

    Cell c;
    c.name = stat_str.c_str();
    c.action = [&]() {};
    state.cells.push_back(std::make_shared<Cell>(c));
  }

  create_as_vertical_layout(state.cells);
  state.active = state.cells[0];
  init = true;
}

} // namespace game2d