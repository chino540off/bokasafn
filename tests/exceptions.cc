#include <gtest/gtest.h>

#include <bokasafn/exceptions.hh>

TEST(TestExceptions, Perror)
{
  auto f = []() {
    errno = EACCES;
    throw bokasafn::exceptions::perror("test");
  };
  EXPECT_THROW(f(), bokasafn::exceptions::perror);
}
