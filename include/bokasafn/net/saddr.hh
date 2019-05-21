/**
 *  @file addr.hh
 *  @author Olivier Détour (detour.olivier@gmail.com)
 */
#ifndef BOKASAFN_NET_SADDR_HH_
#define BOKASAFN_NET_SADDR_HH_

#include <arpa/inet.h>
#include <endian.h> // __BYTE_ORDER __LITTLE_ENDIAN
#include <netdb.h>
#include <sys/socket.h>

#include <algorithm> // std::reverse()
#include <string>

#include <bokasafn/exceptions.hh>

namespace bokasafn
{
namespace net
{

template <typename T>
constexpr T
htonT(T value) noexcept
{
#if __BYTE_ORDER == __LITTLE_ENDIAN
  char * ptr = reinterpret_cast<char *>(&value);
  std::reverse(ptr, ptr + sizeof(T));
#endif
  return value;
}

template <typename T>
constexpr T
ntohT(T value) noexcept
{
  return htonT(value);
}

class saddr
{
public:
  constexpr saddr() : sa_raw_({PF_UNSPEC, {}}) {}

  constexpr saddr(in_addr_t const & in, std::uint16_t port)
    : sa_in_({AF_INET, htonT(port), {htonT(in)}, {}})
  {
  }
  constexpr saddr(struct in_addr const & in, std::uint16_t port)
    : sa_in_({AF_INET, htonT(port), in, {}})
  {
  }
  constexpr saddr(struct in6_addr const & in, std::uint16_t port)
    : sa_in6_({AF_INET6, 0, htonT(port), in, 0})
  {
  }

  saddr(std::string const & str, std::uint16_t port)
  {
    struct addrinfo hint = {};
    struct addrinfo * info = nullptr;

    hint.ai_family = PF_UNSPEC;
    hint.ai_flags = AI_NUMERICHOST;

    if (getaddrinfo(str.c_str(), nullptr, &hint, &info))
      throw bokasafn::exceptions::perror("getaddrinfo");

    sa_raw_.sa_family = info->ai_family;

    freeaddrinfo(info);

    switch (sa_raw_.sa_family)
    {
      case AF_INET:
        sa_in_.sin_family = AF_INET;
        sa_in_.sin_port = htonT(port);
        inet_pton(sa_raw_.sa_family, str.c_str(), &sa_in_.sin_addr);
        break;

      case AF_INET6:
        sa_in6_.sin6_family = AF_INET6;
        sa_in6_.sin6_port = htonT(port);
        inet_pton(sa_raw_.sa_family, str.c_str(), &sa_in6_.sin6_addr);
        break;

      default:
        sa_raw_.sa_family = PF_UNSPEC;
        break;
    }
  }

public:
  constexpr struct sockaddr const *
  get() const
  {
    switch (sa_raw_.sa_family)
    {
      case AF_INET:
        return reinterpret_cast<struct sockaddr const *>(&sa_in_);
      case AF_INET6:
        return reinterpret_cast<struct sockaddr const *>(&sa_in6_);
      default:
        return nullptr;
    }
  }

  constexpr size_t
  size() const
  {
    switch (sa_raw_.sa_family)
    {
      case AF_INET:
        return sizeof(sa_in_);
      case AF_INET6:
        return sizeof(sa_in6_);
      default:
        return 0;
    }
  }

public:
  constexpr std::uint16_t
  port() const
  {
    switch (sa_raw_.sa_family)
    {
      case AF_INET:
        return htonT(sa_in_.sin_port);
      case AF_INET6:
        return htonT(sa_in6_.sin6_port);
      default:
        return 0;
    }
  }

  constexpr sa_family_t
  family() const
  {
    return sa_raw_.sa_family;
  }

  constexpr void const *
  in_raw() const
  {
    switch (sa_raw_.sa_family)
    {
      case AF_INET:
        return &sa_in_.sin_addr;
      case AF_INET6:
        return &sa_in6_.sin6_addr;
      default:
        return nullptr;
    }
  }

  constexpr in_addr
  in() const
  {
    return sa_in_.sin_addr;
  }

  constexpr struct sockaddr *
  raw()
  {
    return &sa_raw_;
  }

  constexpr struct sockaddr const *
  raw() const
  {
    return &sa_raw_;
  }

private:
  union {
    struct sockaddr sa_raw_;
    struct sockaddr_in sa_in_;
    struct sockaddr_in6 sa_in6_;
  };
};

template <typename ostream>
ostream &
operator<<(ostream & os, saddr const & a)
{
  char buffer[ INET6_ADDRSTRLEN ];

  inet_ntop(a.family(), a.in_raw(), buffer, INET6_ADDRSTRLEN);

  return os << buffer << ":" << a.port(), os;
}

bool
operator<(saddr const & lhs, saddr const & rhs)
{
  return memcmp(lhs.raw(), rhs.raw(), sizeof(struct sockaddr)) < 0;
}

} /** !net */
} /** !bokasafn */

#endif /** !BOKASAFN_NET_SADDR_HH_ */
