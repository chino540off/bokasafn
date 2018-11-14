#include <gtest/gtest.h>

#include <bokasafn/literals/size.hh>

using namespace bokasafn::literals;

TEST(TestSizeLiterals, Basic)
{
  EXPECT_EQ(1_KB, (unsigned)1024);
  EXPECT_EQ(1_MB, 1024_KB);
  EXPECT_EQ(1_GB, 1024_MB);
  EXPECT_EQ(1_TB, 1024_GB);
}

TEST(TestSizeLiterals, Operators)
{
  EXPECT_TRUE(0_KB == 0);
  EXPECT_TRUE(0_KB == size());
  EXPECT_TRUE(1_KB == 1_KB);
  EXPECT_TRUE(1_KB == 1_KB);
  EXPECT_TRUE(1_KB != 2_KB);
  EXPECT_TRUE(1_KB <  2_KB);
  EXPECT_TRUE(1_KB <= 2_KB);
  EXPECT_TRUE(2_KB >  1_KB);
  EXPECT_TRUE(2_KB >= 1_KB);

  EXPECT_TRUE(1_MB == 1_KB * 1024);
  EXPECT_TRUE(1_MB == 1_KB * 1_KB);
}
