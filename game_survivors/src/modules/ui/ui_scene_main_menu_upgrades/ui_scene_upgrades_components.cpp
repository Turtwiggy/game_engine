#include "pch.hpp"

#include "engine/maths/grid.hpp"
#include "engine/maths/maths.hpp"
#include "modules/core/ui/ui_common_helpers.hpp"
#include "ui_scene_upgrades_components.hpp"
#include "ui_scene_upgrades_helpers.hpp"

namespace game2d {
using namespace engine::grid;

void
SINGLE_PersistentUpgradesMenuUI::do_init(entt::registry& r)
{
  for (int i = 0; i < (int)UpgradeableStat::count; i++) {

    const auto stat_enum = magic_enum::enum_value<UpgradeableStat>(i);
    const auto stat_str = std::string(magic_enum::enum_name<UpgradeableStat>(stat_enum));

    // stats to skip...
    if (stat_enum == UpgradeableStat::ACTOR_STAMINA)
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