/**
 *  @file multicast.hh
 *  @author Olivier Détour (detour.olivier@gmail.com)
 */
#ifndef BOKASAFN_NET_MULTICAST_HH_
#define BOKASAFN_NET_MULTICAST_HH_

#include <bokasafn/net/saddr.hh>

namespace bokasafn
{
namespace net
{

/**
 * @brief
 */
class mreq
{
public:
  mreq(saddr const & maddr) : family_(maddr.family())
  {
    switch (family_)
    {
      case AF_INET:
        mreq_.imr_multiaddr = maddr.in();
        mreq_.imr_interface.s_addr = htonT(INADDR_ANY);
        break;

      case AF_INET6:
        // FIXME
        break;
      default:
        break;
    }
  }

public:
  constexpr void const *
  raw() const
  {
    switch (family_)
    {
      case AF_INET:
        return &mreq_;
      case AF_INET6:
        return &mreq6_;
      default:
        return nullptr;
    }
  }

  constexpr size_t
  size() const
  {
    switch (family_)
    {
      case AF_INET:
        return sizeof(mreq_);
      case AF_INET6:
        return sizeof(mreq6_);
      default:
        return 0;
    }
  }

private:
  sa_family_t family_;
  union {
    struct ip_mreq mreq_;
    struct ipv6_mreq mreq6_;
  };
};

} /** !net */
} /** !bokasafn */

#endif /** !BOKASAFN_NET_MULTICAST_HH_ */
