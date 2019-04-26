#include <gtest/gtest.h>

#include <thread>

#include <bokasafn/net/saddr.hh>
#include <bokasafn/net/socket.hh>

using namespace std::chrono_literals;

TEST(TestNet, Saddr)
{
  std::stringstream ss;

  ss << bokasafn::net::saddr{"10.10.10.10", 12345};
  EXPECT_EQ(ss.str(), "10.10.10.10:12345");
  ss.str("");
  ss.clear();

  ss << bokasafn::net::saddr{"::1", 12345};
  EXPECT_EQ(ss.str(), "::1:12345");
  ss.str("");
  ss.clear();
}

TEST(TestNet, SaddrStatic)
{
  constexpr auto sa1 = bokasafn::net::saddr{};
  static_assert(sa1.family() == PF_UNSPEC);

  // C++20: constexpr std::reverse
  /*constexpr auto sa2 = */ bokasafn::net::saddr{INADDR_ANY, 12345};
  // static_assert(sa2.family() == AF_INET);
  // static_assert(sa2.in() == 0);
  // static_assert(sa2.port() == 12345);
  /*constexpr auto sa3 = */ bokasafn::net::saddr{{0}, 12345};
  /*constexpr auto sa4 = */ bokasafn::net::saddr{in6addr_any, 12345};
}

TEST(TestNet, SocketUDP)
{
  int data = 42;
  auto sa = bokasafn::net::saddr{"127.0.0.1", 12345};

  auto s = bokasafn::net::socket<AF_INET, SOCK_DGRAM, IPPROTO_UDP>();
  s.bind(sa);

  auto c = bokasafn::net::socket<AF_INET, SOCK_DGRAM, IPPROTO_UDP>();
  c.sendto(sa, &data, sizeof(data));

  int input;
  s.recvfrom(sa, &input, sizeof(input));

  EXPECT_EQ(input, data);
}

TEST(TestNet, SocketTCP)
{
  auto s = bokasafn::net::socket<AF_INET, SOCK_STREAM, IPPROTO_TCP>();
  s.set_option(bokasafn::net::option<SOL_SOCKET, SO_REUSEADDR, int>(true));
  s.bind({"0.0.0.0", 12345});
  s.listen(3);

  std::this_thread::sleep_for(1s);

  std::thread t([&s]() {
    bokasafn::net::saddr paddr{};

    int data = 42;
    auto p = s.accept(paddr);
    int input;

    p.send(&data, sizeof(data));
    p.recv(&input, sizeof(input));

    p.close();

    EXPECT_EQ(input, data);
  });

  auto c = bokasafn::net::socket<AF_INET, SOCK_STREAM, IPPROTO_TCP>();
  c.connect({"127.0.0.1", 12345});

  int data = 42;
  int input;
  c.recv(&input, sizeof(input));
  c.send(&input, sizeof(input));
  c.close();

  t.join();

  s.close();
  EXPECT_EQ(input, data);
}
