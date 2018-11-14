/**
 *  @file size.hh
 *  @author Olivier Détour (detour.olivier@gmail.com)
 */
#ifndef BOKASAFN_LITERALS_SIZE_HH_
# define BOKASAFN_LITERALS_SIZE_HH_

namespace bokasafn {
namespace literals {

struct size
{
  constexpr size(std::size_t v_): v(v_) {}
  constexpr size(): v(0) {}


  constexpr size operator*=(size const & rhs)
  {
    v *= rhs.v;

    return *this;
  }

  std::size_t v;
};

constexpr size operator*(size lhs, size const & rhs) { return lhs *= rhs; }

constexpr bool operator==(size const & lhs, size const & rhs) { return lhs.v == rhs.v; }
constexpr bool operator!=(size const & lhs, size const & rhs) { return lhs.v != rhs.v; }
constexpr bool operator>(size const & lhs, size const & rhs)  { return lhs.v > rhs.v; }
constexpr bool operator>=(size const & lhs, size const & rhs) { return lhs.v >= rhs.v; }
constexpr bool operator<(size const & lhs, size const & rhs)  { return lhs.v < rhs.v; }
constexpr bool operator<=(size const & lhs, size const & rhs) { return lhs.v <= rhs.v; }

constexpr size operator"" _KB(unsigned long long int v)
{
  return size(v * 1024);
}

constexpr size operator"" _MB(unsigned long long int v)
{
  return size(v) * 1024_KB;
}

constexpr size operator"" _GB(unsigned long long int v)
{
  return size(v) * 1024_MB;
}

constexpr size operator"" _TB(unsigned long long int v)
{
  return size(v) * 1024_GB;
}

} /** !literals */
} /** !bokasafn */

#endif /** !BOKASAFN_LITERALS_SIZE_HH_ */
