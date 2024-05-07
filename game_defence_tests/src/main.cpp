
#include <cstdio>
#include <gtest/gtest.h>

#include "test_actor_patrol.hpp"

#undef main

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
  printf("Running main() from %s\n", __FILE__);
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

// TEST(TestSuite, HelloTest)
// {
//   ASSERT_TRUE(true);
// }

// TEST(TestSuite, HelloTest2)
// {
//   ASSERT_EQ(1.0f, 0.0f);
// }
