#include "pch.hpp"

#include "engine/maths/grid.hpp"
#include "engine/maths/maths.hpp"

#include <gtest/gtest.h>

TEST(TestSuite, HelloMaths)
{
  // arrange
  const int index = 0;
  const glm::ivec2 gp{ 0, 0 };

  // act
  const auto [x, y] = engine::grid::index_to_grid_position(index, 5);

  // assert
  ASSERT_EQ(gp, glm::ivec2(x, y));
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

TEST(TestSuite, AngleFlipYAxis)
{
  // note: all inputs on the positive y axis scale,
  // so that 90 degrees is the y-axis up,
  // and that the 270 degrees is the y-axis down.

  // convert angles to the engine angles,
  // where y-axis down is 90, and y-axis up is 270

  // arrange & act
  const auto angle_45 = engine::angle_degrees_flip_y_axis(-45);
  const auto angle0 = engine::angle_degrees_flip_y_axis(0);
  const auto angle45 = engine::angle_degrees_flip_y_axis(45);
  const auto angle90 = engine::angle_degrees_flip_y_axis(90);
  const auto angle135 = engine::angle_degrees_flip_y_axis(135);
  const auto angle180 = engine::angle_degrees_flip_y_axis(180);
  const auto angle225 = engine::angle_degrees_flip_y_axis(225);
  const auto angle270 = engine::angle_degrees_flip_y_axis(270);
  const auto angle315 = engine::angle_degrees_flip_y_axis(315);
  const auto angle360 = engine::angle_degrees_flip_y_axis(360);

  // assert
  const float epsilon = 0.001f;
  ASSERT_NEAR(angle_45, 45, epsilon);
  ASSERT_NEAR(angle0, 0, epsilon);
  ASSERT_NEAR(angle45, 315, epsilon);
  ASSERT_NEAR(angle90, 270, epsilon);
  ASSERT_NEAR(angle135, 225, epsilon);
  ASSERT_NEAR(angle180, 180, epsilon);
  ASSERT_NEAR(angle225, 135, epsilon);
  ASSERT_NEAR(angle270, 90, epsilon);
  ASSERT_NEAR(angle315, 45, epsilon);
  ASSERT_NEAR(angle360, 0, epsilon);
};
