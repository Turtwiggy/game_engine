#include "engine/maths/maths.hpp"
#include "engine/maths/grid.hpp"

#include <gtest/gtest.h>

TEST(TestSuite, HelloMaths)
{
  // arrange
  const int index = 0;
  const glm::ivec2 gp{ 0, 0 };

  // act
  const glm::ivec2 result = engine::grid::index_to_grid_position(index, 5, 5);

  // assert
  ASSERT_EQ(gp, result);
};

TEST(TestSuite, DirToAngle)
{
  // arrange & act
  const auto angle0 = engine::dir_to_angle_radians({ 1.0f, 0.0f });
  const auto angle1 = engine::dir_to_angle_radians({ 0.0f, 1.0f });
  const auto angle2 = engine::dir_to_angle_radians({ -1.0f, 0.0f });
  const auto angle3 = engine::dir_to_angle_radians({ 0.0f, -1.0f });

  // assert
  const float epsilon = 0.001f;
  ASSERT_NEAR(angle0, 0.0f, epsilon);
  ASSERT_NEAR(angle1, 0.5f * engine::PI, epsilon);
  ASSERT_NEAR(angle2, 1.0f * engine::PI, epsilon);
  ASSERT_NEAR(angle3, 1.5f * engine::PI, epsilon);
};
