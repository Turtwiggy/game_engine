#include "intent_use_item.hpp"

#include "components.hpp"
#include "game/components/actors.hpp"
#include "game/modules/combat/components.hpp"
#include "game/modules/player/helpers.hpp"
#include "modules/renderer/components.hpp"

#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <imgui.h>

namespace game2d {

void
update_intent_use_item_system(GameEditor& editor, Game& game)
{
  auto& r = game.state;
  auto& eb = game.dead;

  //
  // WantsToUse
  const auto& use_view = r.view<WantsToUse>();
  use_view.each([&editor, &game, &r, &eb](auto user, WantsToUse& intent) {
    //
    for (const Use& item : intent.items) {

      // Does the item heal?
      auto* item_heals = r.try_get<GiveHealsComponent>(item.entity);
      for (const entt::entity& target : item.targets) {
        auto* target_hp = r.try_get<HealthComponent>(target);
        // Does the item heal?
        if (item_heals && target_hp)
          target_hp->hp = glm::min(target_hp->max_hp, target_hp->hp + item_heals->health);
      }

      // Does the item damage?
      auto* item_damage = r.try_get<AttackComponent>(item.entity);
      if (item_damage) {
        for (const entt::entity& target : item.targets) {
          // Can the target take damage?
          auto* target_damage_buffer = r.try_get<TakeDamageComponent>(target);
          if (target_damage_buffer) {
            // TEMP: just use min damage
            RecieveDamageRequest dmg;
            dmg.base_damage = item_damage->min_damage;
            target_damage_buffer->damage.push_back(dmg);
          } else {
            std::cerr << "Tried to attack an entity that doesn't have a TakeDamageComponent\n";
          }
        }
      }

      // Is the item throwable?
      auto* throwable = r.try_get<ThrowableComponent>(item.entity);
      if (throwable) {
        shoot(editor, game, user);
      }

      // Is the item consumable?
      auto* item_consumable = r.try_get<ConsumableComponent>(item.entity);
      if (item_consumable)
        eb.dead.emplace(item.entity);
    }

    // Done processing all the items
    r.remove<WantsToUse>(user);
  });
};

} // namespace game2d