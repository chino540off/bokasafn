#include <gtest/gtest.h>

#include <bokasafn/cache/cache.hh>
#include <bokasafn/literals/size.hh>

using namespace bokasafn::literals;

TEST(TestCacheNumberedLRU, BasicUpdate)
{
  bokasafn::cache::nlru<std::string, int> c(2);

  c.put("k1", 42);
  c.put("k2", 51);

  EXPECT_EQ(c.get("k1"), 42);
  EXPECT_EQ(c.get("k2"), 51);

  c.put("k1", 42);
  c.put("k3", 101);

  EXPECT_THROW(c.get("k2"), std::range_error);
}
