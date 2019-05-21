#include <fcntl.h>
#include <unistd.h>

#include <thread>

#include <gtest/gtest.h>

#include <bokasafn/epoll.hh>

using namespace std::literals::chrono_literals;

TEST(TestEpoll, FileDescriptor)
{
  int p[ 2 ];
  EXPECT_EQ(pipe(p), 0);

  EXPECT_GE(write(p[ 1 ], "a", 2), 0);
  close(p[ 1 ]);

  bool test1 = false;
  bool test2 = false;

  bokasafn::epoll<20> e;

  e.add(p[ 0 ], [&test1](int fd) {
    char buffer[ 16 ];

    // Read from pipe
    EXPECT_GE(read(fd, buffer, sizeof(buffer)), 0);
    test1 = true;

    return true;
  });
  e.add(p[ 0 ], [&test2](int) {
    test2 = true;
    return false;
  });

  std::thread t([&e]() { e.start(500ms); });

  std::this_thread::sleep_for(2s);

  e.remove(p[ 0 ]);
  close(p[ 0 ]);

  e.stop();
  t.join();

  EXPECT_TRUE(test1);
  EXPECT_TRUE(test2);
}

TEST(TestEpoll, Timer)
{
  bokasafn::epoll<20> e;
  int test1 = 0;
  int test2 = 0;

  e.timer(2s, [&test1](int) {
    test1++;
    return false;
  });

  e.timer(1s, [&test2](int) {
    test2++;
    return true;
  });

  std::thread t([&e]() { e.start(500ms); });

  std::this_thread::sleep_for(5s);

  e.stop();
  t.join();

  EXPECT_EQ(test1, 1);
  EXPECT_GE(test2, 2);
}
