#include "pch.hpp"

#include "engine/maths/grid.hpp"
#include "engine/maths/maths.hpp"

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

TEST(TestSuite, WrapMin)
{
  // arrange & act
  const int test4 = engine::wrap(-4, 3); // 2
  const int test3 = engine::wrap(-3, 3); // 0
  const int test2 = engine::wrap(-2, 3); // 1
  const int test1 = engine::wrap(-1, 3); // 2
  const int test0 = engine::wrap(0, 3);  // 0

  // assert
  ASSERT_EQ(test0, 0);
  ASSERT_EQ(test1, 2);
  ASSERT_EQ(test2, 1);
};

TEST(TestSuite, WrapMax)
{
  // arrange & act
  const int test = engine::wrap(4, 3);

  ASSERT_EQ(test, 1);
};

/*
TEST(TestSuit, Grid)
{
  // arrange
  const auto in_a = glm::vec2{ 0, 31.999 };
  const auto in_b = glm::vec2{ 0, -31.999 };
  const auto in_c = glm::vec2{ 0, 63.999 };
  const auto in_d = glm::vec2{ 0, -63.999 };

  const auto in_e = glm::vec2{ 0, 32 };
  const auto in_f = glm::vec2{ 0, -32 };
  const auto in_g = glm::vec2{ 0, 64 };
  const auto in_h = glm::vec2{ 0, -64 };

  const auto in_i = glm::vec2{ 0, 32.001 };
  const auto in_j = glm::vec2{ 0, -32.001 };
  const auto in_k = glm::vec2{ 0, 64.001 };
  const auto in_l = glm::vec2{ 0, -64.001 };

  // act
  const glm::ivec2 out_a = engine::grid::worldspace_to_gridspace(in_a, 32);
  const glm::ivec2 out_b = engine::grid::worldspace_to_gridspace(in_b, 32);
  const glm::ivec2 out_c = engine::grid::worldspace_to_gridspace(in_c, 32);
  const glm::ivec2 out_d = engine::grid::worldspace_to_gridspace(in_d, 32);

  const glm::ivec2 out_e = engine::grid::worldspace_to_gridspace(in_e, 32);
  const glm::ivec2 out_f = engine::grid::worldspace_to_gridspace(in_f, 32);
  const glm::ivec2 out_g = engine::grid::worldspace_to_gridspace(in_g, 32);
  const glm::ivec2 out_h = engine::grid::worldspace_to_gridspace(in_h, 32);

  // assert
  const bool eq_a = out_a == glm::ivec2{ 0, 0 };
  ASSERT_TRUE(eq_a);
  const bool eq_b = out_b == glm::ivec2{ 0, -1 };
  ASSERT_TRUE(eq_b);
  const bool eq_c = out_c == glm::ivec2{ 0, 1 };
  ASSERT_TRUE(eq_c);
  const bool eq_d = out_d == glm::ivec2{ 0, -2 };
  ASSERT_TRUE(eq_d);

  const bool eq_e = out_e == glm::ivec2{ 0, 1 };
  ASSERT_TRUE(eq_e);
  const bool eq_f = out_f == glm::ivec2{ 0, -1 };
  ASSERT_TRUE(eq_f);
  const bool eq_g = out_g == glm::ivec2{ 0, 2 };
  ASSERT_TRUE(eq_g);
  const bool eq_h = out_h == glm::ivec2{ 0, -2 };
  ASSERT_TRUE(eq_h);
};
*/
