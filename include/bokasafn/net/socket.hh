/**
 *  @file socket.hh
 *  @author Olivier Détour (detour.olivier@gmail.com)
 */
#ifndef BOKASAFN_NET_SOCKET_HH_
#define BOKASAFN_NET_SOCKET_HH_

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#include <unistd.h>

#include <chrono>
#include <memory>

#include <bokasafn/exceptions.hh>
#include <bokasafn/net/multicast.hh>
#include <bokasafn/net/saddr.hh>

namespace bokasafn
{
namespace net
{

template <int L, int O, typename T>
class option
{
public:
  constexpr static int level = L;
  constexpr static int optname = O;
  constexpr static socklen_t optlen = sizeof(T);
  void const * optval;

public:
  option(T const & v) : optval(&value_), value_(v) {}

private:
  T value_;
};

template <int L, int O>
class option<L, O, std::chrono::seconds>
{
private:
  static timeval
  convert(std::chrono::seconds dur)
  {
    return timeval{dur.count(), 0};
  }

public:
  constexpr static int level = L;
  constexpr static int optname = O;
  constexpr static socklen_t optlen = sizeof(timeval);
  void const * optval;

  option(std::chrono::seconds const & v) : optval(&value_), value_(convert(v)) {}

private:
  timeval value_;
};

template <int L, int O>
class option<L, O, mreq>
{
private:
  mreq value_;

public:
  constexpr static int level = L;
  constexpr static int optname = O;
  socklen_t optlen;
  void const * optval;

  option(mreq const & v) : value_(v), optlen(value_.size()), optval(value_.raw()) {}
};

/**
 * @brief
 */
template <int AF, int SOCK, int PROTO>
class socket
{
public:
  socket() { fd_ = ::socket(AF, SOCK, PROTO); }
  ~socket() { close(); }

private:
  socket(int fd) : fd_(fd) {}

public:
  void
  bind(saddr const & a) const
  {
    // bind to receive address
    if (::bind(fd_, a.get(), a.size()) < 0)
      throw bokasafn::exceptions::perror("bind");
  }

  void
  connect(saddr const & a) const
  {
    if (::connect(fd_, a.get(), a.size()) < 0)
      throw bokasafn::exceptions::perror("connect");
  }

  void
  listen(int backlog) const
  {
    if (::listen(fd_, backlog) < 0)
      throw bokasafn::exceptions::perror("listen");
  }

  std::shared_ptr<socket<AF, SOCK, PROTO>>
  accept(saddr & a) const
  {
    socklen_t addrlen = sizeof(sockaddr);

    int fd = ::accept(fd_, a.raw(), &addrlen);
    if (fd < 0)
      throw bokasafn::exceptions::perror("accept");

    return std::shared_ptr<socket<AF, SOCK, PROTO>>(new socket<AF, SOCK, PROTO>(fd));
  }

public:
  void
  close()
  {
    ::close(fd_);
    fd_ = -1;
  }

public:
  template <typename O>
  void
  set_option(O const & opt) const
  {
    int err = setsockopt(fd_, opt.level, opt.optname, opt.optval, opt.optlen);
    if (err < 0)
      throw bokasafn::exceptions::perror("setsockopt");
  }

  int
  get_flags() const
  {
    int flags;

    flags = fcntl(fd_, F_GETFL, 0);
    if (flags == -1)
      throw bokasafn::exceptions::perror("fcntl(F_GETFL)");

    return flags;
  }

  void
  set_flags(int flags) const
  {
    if (fcntl(fd_, F_SETFL, flags) == -1)
      throw bokasafn::exceptions::perror("fcntl(F_SETFL)");
  }

  void
  add_flags(int flags) const
  {
    set_flags(get_flags() | flags);
  }

  void
  remove_flags(int flags) const
  {
    set_flags(get_flags() & ~flags);
  }

  void
  join_mgroup(saddr const & maddr) const
  {
    set_option(option<IPPROTO_IP, IP_ADD_MEMBERSHIP, mreq>(maddr));
  }

  void
  leave_mgroup(saddr const & maddr) const
  {
    set_option(option<IPPROTO_IP, IP_DROP_MEMBERSHIP, mreq>(maddr));
  }

public:
  ssize_t
  recvfrom(saddr & a, void * buffer, size_t size) const
  {
    socklen_t addrlen = sizeof(sockaddr);

    return ::recvfrom(fd_, buffer, size, 0, a.raw(), &addrlen);
  }

  ssize_t
  recv(void * buffer, size_t size, int flags = 0) const
  {
    return ::recv(fd_, buffer, size, flags);
  }

public:
  ssize_t
  sendto(saddr const & a, void const * buffer, size_t size) const
  {
    return ::sendto(fd_, buffer, size, 0, a.get(), a.size());
  }

  ssize_t
  send(void const * buffer, size_t size, int flags = 0) const
  {
    return ::send(fd_, buffer, size, flags);
  }

public:
  int
  fd() const
  {
    return fd_;
  }

private:
  int fd_;
};

namespace ipv4
{

using tcp = socket<AF_INET, SOCK_STREAM, IPPROTO_TCP>;
using udp = socket<AF_INET, SOCK_DGRAM, IPPROTO_UDP>;

} /** !ipv4 */

namespace ipv6
{

using tcp = socket<AF_INET6, SOCK_STREAM, IPPROTO_TCP>;
using udp = socket<AF_INET6, SOCK_DGRAM, IPPROTO_UDP>;

} /** !ipv6 */
} /** !net */
} /** !bokasafn */

#endif /** !BOKASAFN_NET_SOCKET_HH_ */
