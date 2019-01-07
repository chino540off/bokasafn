#include <gtest/gtest.h>

#include <bokasafn/timer.hh>

TEST(TestTimer, Timeout)
{
  bokasafn::timer t;
  int value = 0;

  t.timeout(1000, [&value]()
  {
    value = 42;
  });

  t.wait();

  EXPECT_EQ(value, 42);
}

TEST(TestTimer, Interval)
{
  bokasafn::timer t;
  int value = 0;

  t.interval(1000, [&value]()
  {
    value++;
  });

  std::this_thread::sleep_for(std::chrono::seconds(2));
  t.stop();
  t.wait();

  EXPECT_EQ(value, 1);
}
