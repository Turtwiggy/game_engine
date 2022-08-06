#include "potion.hpp"

#include <iostream> // temp

namespace game2d {

Potion::Potion()
{
  name = "Potion";
  quantity = 1;
};

bool
Potion::use(entt::registry& r)
{
  std::cout << " do a potion thing" << std::endl;
  return false;
};

} // namespace game2d