#include "system.hpp"

#include "modules/actor_cover/components.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/system_turnbased_enemy/components.hpp"
#include "modules/ui_inventory/components.hpp"

namespace game2d {

// take in to account: inventory
// take in to account: if you've got the "incover" component
// set the result in the defencecomponent

void
update_combat_defence_system(entt::registry& r)
{
  for (const auto& [e, unit_c, defence_c] : r.view<const ActionState, DefenceComponent>().each()) {
    //
    int defence_value = 0;

    // check if you're in cover
    if (auto* req = r.try_get<InCoverComponent>(e))
      defence_value += 5;

    // check your equipped items
    if (auto* inv = r.try_get<DefaultBody>(e)) {
      for (const entt::entity slot_e : inv->body) {

        // inventory slot...
        if (slot_e == entt::null)
          continue;

        // inventory item...
        const auto& inv_c = r.get<InventorySlotComponent>(slot_e);
        if (inv_c.item_e == entt::null)
          continue;

        // inventory item type...
        const auto& item_c = r.get<ItemTypeComponent>(inv_c.item_e);
        if (item_c.type != ItemType::armour)
          continue;

        // inventory item data...
        defence_value += r.get<DefenceComponent>(inv_c.item_e).armour;
      }
    }

    //
    defence_c.armour = defence_value;
  }

  //
}

} // namespace game2d