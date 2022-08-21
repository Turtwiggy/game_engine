#include "intent_drink_potion.hpp"

#include "game/components/components.hpp"
#include "glm/glm.hpp"

namespace game2d {

void
update_intent_drink_potion_system(entt::registry& r)
{
  const auto& view = r.view<HealthComponent, WantsToDrinkPotionComponent>();
  view.each([&r](auto entity, auto& hp, auto& intent) {
    if (auto* potion = r.try_get<PotionComponent>(intent.potion)) {
      hp.hp = glm::min(hp.max_hp, hp.hp + potion->heal_amount);

      r.remove<WantsToDrinkPotionComponent>(entity);
      r.destroy(intent.potion);
    };
  });
};

} // namespace game2d