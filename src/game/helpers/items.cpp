#include "items.hpp"

#include "game/components/breakable.hpp"

#include <iostream> // temp

namespace game2d {

Potion::Potion()
{
  name = "Potion";
  infinite_quantity = true;
};

bool
Potion::use(entt::registry& r, std::vector<entt::entity>& entities)
{
  std::cout << "healin..!" << std::endl;

  if (entities.size() > 0) {
    auto entity = entities[0]; // potion, only apply to first atm

    if (r.any_of<BreakableComponent>(entity)) {
      auto& hp = r.get<BreakableComponent>(entity);
      hp.hp += heal_amount;
      return true;
    }
  }

  return false;
};

} // namespace game2d