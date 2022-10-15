#include "intent_select_units_for_item.hpp"

#include "components.hpp"
#include "game/components/actors.hpp"
#include "game/components/events.hpp"
#include "modules/events/helpers/mouse.hpp"
#include "modules/renderer/components.hpp"

namespace game2d {

void
update_select_units_for_item_system(GameEditor& editor, Game& game)
{

  auto& r = game.state;

  const auto& colliding = r.view<CollidingWithCursorComponent>();

  // Use the selected entities as the item's use target.

  const auto& view = r.view<WantsToSelectUnitsForItem, const TagComponent>();
  for (auto [entity, items, tag] : view.each()) {

    for (auto& item : items.items) {
      item.targets.clear();
      for (auto [entity, coll] : colliding.each())
        item.targets.push_back(entity);
    }
  }
};

} // namespace game2d