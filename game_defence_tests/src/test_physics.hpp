#pragma once

#include "physics/components.hpp"
#include "physics/helpers.hpp"

#include <gtest/gtest.h>

namespace game2d {

namespace tests {

TEST(TestSuite, Test_CircleCollision_Collision)
{
  CircleCollider a;
  a.center = { 0, 0 };
  a.radius = 2.0;

  CircleCollider b;
  b.center = { 1, 0 };
  b.radius = 2.0;

  ASSERT_TRUE(collide(a, b));
};

TEST(TestSuite, Test_CircleCollision_NoCollision)
{
  CircleCollider a;
  a.center = { 0, 0 };
  a.radius = 1.0;

  CircleCollider b;
  b.center = { 10, 0 };
  b.radius = 1.0;

  ASSERT_FALSE(collide(a, b));
}

} // namespace tests

} // namespace game2d