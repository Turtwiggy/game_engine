#include "pch.hpp"

#include "engine/maths/grid.hpp"
#include "modules/systems/system_upgrade/upgrade_components.hpp"

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
  modifiers_c.add(std::make_shared<StatFlatIncrease>(3.0f, stat));
  modifiers_c.add(std::make_shared<StatPercentIncrease>(50.0f, stat));

  // assert
  float result = modifiers_c.apply_modifiers(base_bullet_damage, stat);
  float expected = (((base_bullet_damage + 5) * 1.2) + 3) * 1.5;
  SDL_Log("Result: %f", result);
  ASSERT_EQ(result, expected);
};

TEST(TestSuite, StackPercentIncrease)
{
  // arrange
  std::string stat = "bullet_speed";
  float base_value = 10;

  // act
  StatModifierComponent modifiers_c;
  modifiers_c.add(std::make_shared<StatPercentIncrease>(50.0f, stat));
  modifiers_c.add(std::make_shared<StatPercentIncrease>(50.0f, stat));

  // assert
  float result = modifiers_c.apply_modifiers(base_value, stat);
  float expected = 22.5f; // 10*50%*50% I want to be 22.5, not 20
  ASSERT_EQ(result, expected);
}

TEST(TestSuite, StackPercentDecrease)
{
  // arrange
  std::string stat = "bullet_speed";
  float base_value = 10;

  // act
  StatModifierComponent modifiers_c;
  modifiers_c.add(std::make_shared<StatPercentIncrease>(-50.0f, stat));
  modifiers_c.add(std::make_shared<StatPercentIncrease>(-50.0f, stat));

  // assert
  float result = modifiers_c.apply_modifiers(base_value, stat);
  float expected = 2.5f; //  10-50%-50% I want to be 2.5, not 0
  ASSERT_EQ(result, expected);
}

} // namespace tests

} // namespace game2d