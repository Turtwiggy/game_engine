#include "intent_use_item.hpp"

#include "components.hpp"
#include "game/components/actors.hpp"
#include "game/modules/combat/components.hpp"
#include "modules/renderer/components.hpp"

#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <imgui.h>

namespace game2d {

void
update_intent_use_item_system(GameEditor& editor, Game& game)
{
  auto& r = game.state;

  //
  // WantsToUse
  const auto& use_view = r.view<WantsToUse>();
  use_view.each([&r](auto entity, WantsToUse& intent) {
    //
    for (const Use& item : intent.items) {
      auto* item_heals = r.try_get<GiveHealsComponent>(item.entity);
      auto* item_consumable = r.try_get<ConsumableComponent>(item.entity);

      // If not, use on self
      // auto& target = entity;
      // auto* target_hp = r.try_get<HealthComponent>(entity);

      // If targets are set, use those
      for (const entt::entity& target : item.targets) {
        auto* target_hp = r.try_get<HealthComponent>(target);
        // Does the item heal?
        if (item_heals && target_hp) {
          target_hp->hp = glm::min(target_hp->max_hp, target_hp->hp + item_heals->health);
          std::cout << "hp now at: " << target_hp->hp << "\n";
        }
      }

      // Is the item consumable?
      if (item_consumable)
        r.destroy(item.entity);
    }

    // Done processing all the items
    r.remove<WantsToUse>(entity);
  });
};

} // namespace game2d