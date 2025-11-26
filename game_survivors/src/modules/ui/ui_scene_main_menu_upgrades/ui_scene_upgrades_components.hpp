#pragma once

#include "modules/core/ui/ui_common_components.hpp"
#include "modules/systems/system_upgrade/upgrade_components.hpp"
#include "modules/ui/ui_element_cursor/element_cursor_components.hpp"

namespace game2d {

struct RequestToShowUpgradesMenu
{
  bool placeholder = true;
};

struct GridCell;
struct HeaderCell : public Cell
{
  int value;

  std::vector<std::shared_ptr<Cell>> gridcells;
};

enum class GridCellType
{
  HULL = 0,
  WEAPON = 1,
  STAT = 2,
};

struct GridCell : public Cell
{
  HeaderCell* header = nullptr;

  GridCellType type; // 0 = hull, 1 = weapon, 2 = stat
  int index;         // index in to either hulls or weapons or stats

  GridCell(GridCellType type, int index)
    : type(type)
    , index(index) {};
};

struct SINGLE_PersistentUpgradesMenuUI : public DefaultUI
{
  // hold a button to purchase upgrade
  float purchase_time = 0.0f;
  const float purchase_time_max = 0.70f;

  std::shared_ptr<Cell> active_header;
  UiCursorComponent cursor_c;

  void do_init(entt::registry& r) override;
};

} // namespace game2d