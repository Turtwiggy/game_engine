#include "helpers.hpp"

namespace game2d {

bool
inv_is_empty(entt::registry& r, const DefaultInventory& inv_c)
{
  bool all_items_empty = true;

  for (const auto& inv_e : inv_c.inv) {
    const auto& slot_c = r.get<InventorySlotComponent>(inv_e);
    const auto item_e = slot_c.item_e;
    if (item_e != entt::null)
      all_items_empty = false;
  }

  return all_items_empty;
}

} // namespace game2d