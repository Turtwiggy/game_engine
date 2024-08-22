
#include "maths/grid.hpp"
#include <fmt/core.h>
#include <glm/glm.hpp>
#include <gtest/gtest.h>

// Logical	    ASSERT_TRUE(condition)
//              ASSERT_FALSE(condition)
// General comparison	ASSERT_EQ(expected, actual) / ASSERT_NE(val1, val2)
//                      ASSERT_LT(val1, val2) / ASSERT_LE(val1, val2)
//                      ASSERT_GT(val1, val2) / ASSERT_GE(val1, val2)
// Float point comparison	ASSERT_FLOAT_EQ(expected, actual)
//                          ASSERT_DOUBLE_EQ(expected, actual)
//                          ASSERT_NEAR(val1, val2, abs_error)
// String comparison	ASSERT_STREQ(expected_str, actual_str) / ASSERT_STRNE(str1, str2)
//                      ASSERT_STRCASEEQ(expected_str, actual_str) / ASSERT_STRCASENE(str1, str2)
// Exception checking	ASSERT_THROW(statement, exception_type)
//                      ASSERT_ANY_THROW(statement)
//                      ASSERT_NO_THROW(statement)

int
main(int argc, char** argv)
{
  fmt::println("Running main() from {}", __FILE__);
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

TEST(TestSuite, HelloAgain)
{
  ASSERT_TRUE(true);
};

TEST(TestSuite, HelloTest)
{
  ASSERT_TRUE(true);
};

TEST(TestSuite, HelloTest2)
{
  ASSERT_NE(1.0f, 0.0f);
};

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
