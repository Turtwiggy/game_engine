#include "pch.hpp"

#include "engine/maths/aabb.hpp"
#include "engine/maths/grid.hpp"
#include "engine/maths/maths.hpp"

#include <gtest/gtest.h>

TEST(TestSuite, AABB_Contains)
{
  // arrange
  const auto aabb = game2d::AABB{ { -100, -100 }, { 200, 200 } };
  const auto a_point = glm::vec2{ -100, -100 };
  const auto b_point = glm::vec2{ -100, 100 };
  const auto c_point = glm::vec2{ 100, -100 };
  const auto d_point = glm::vec2{ 100, 100 };

  // act
  const bool contains_a = game2d::contains(aabb, a_point);
  const bool contains_b = game2d::contains(aabb, b_point);
  const bool contains_c = game2d::contains(aabb, c_point);
  const bool contains_d = game2d::contains(aabb, d_point);

  // assert
  ASSERT_TRUE(contains_a);
  ASSERT_TRUE(contains_b);
  ASSERT_TRUE(contains_c);
  ASSERT_TRUE(contains_d);
};

TEST(TestSuite, AABB_NotContains)
{
  // arrange
  const auto aabb = game2d::AABB{ { -100, -100 }, { 200, 200 } };
  const auto point = glm::vec2{ -200, 100 };

  // act
  const bool contains = game2d::contains(aabb, point);

  // assert
  ASSERT_FALSE(contains);
};