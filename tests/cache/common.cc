#include <gtest/gtest.h>

#include <bokasafn/cache/cache.hh>

namespace bokasafn {
namespace cache {

template <typename T>
class CommonTest: public testing::Test { };

TYPED_TEST_CASE_P(CommonTest);

TYPED_TEST_P(CommonTest, BasicPut)
{
  TypeParam c(10);

  c.put(0, 42);

  EXPECT_EQ(c.get(0), 42);
}

TYPED_TEST_P(CommonTest, MissingValue)
{
  TypeParam c(10);

  EXPECT_THROW(c.get(0), std::range_error);
}

TYPED_TEST_P(CommonTest, ColdData)
{
  TypeParam c(8);

  c.put(0, 42);

  for (auto r = 1; r < 2; ++r)
    for (auto i = 1; i < 2000; ++i)
      c.put(i, 42);

  EXPECT_THROW(c.get(0), std::range_error);
}

REGISTER_TYPED_TEST_CASE_P(CommonTest,
  BasicPut,
  MissingValue,
  ColdData
);

typedef testing::Types
<
  bokasafn::cache::sno<int, int>,
  bokasafn::cache::nno<int, int>,
  bokasafn::cache::slru<int, int>,
  bokasafn::cache::nlru<int, int>,
  bokasafn::cache::slfu<int, int>,
  bokasafn::cache::nlfu<int, int>
> CacheTypes;
INSTANTIATE_TYPED_TEST_CASE_P(CacheTypesInstances, CommonTest, CacheTypes);

} /** !cache */
} /** !bokasafn */
