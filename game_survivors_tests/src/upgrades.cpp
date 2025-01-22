#include "engine/maths/grid.hpp"
#include "modules/system_upgrade/upgrade_components.hpp"

#include <SDL2/SDL_log.h>
#include <glm/glm.hpp>
#include <gtest/gtest.h>

namespace game2d {

namespace tests {

TEST(TestSuite, TestModifiers)
{
  // arrange
  std::string stat = "bullet_speed";
  float base_bullet_damage = 10;

  // act
  StatModifierComponent modifiers_c;
  modifiers_c.add(std::make_shared<StatFlatIncrease>(5.0f, stat));
  modifiers_c.add(std::make_shared<StatPercentIncrease>(20.0f, stat));

  // assert
  float result = modifiers_c.apply_modifiers(base_bullet_damage, stat);
  float expected = (base_bullet_damage + 5) + (base_bullet_damage * 0.2);
  SDL_Log("Result: %f", result);
  ASSERT_EQ(result, expected);
};

} // namespace tests

} // namespace game2d