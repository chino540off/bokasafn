/**
 *  @file exceptions.hh
 *  @author Olivier Détour (detour.olivier@gmail.com)
 */
#ifndef BOKASAFN_EXCEPTIONS_HH_
#define BOKASAFN_EXCEPTIONS_HH_

#include <cerrno>
#include <cstring>
#include <exception>

namespace bokasafn
{

namespace exceptions
{

/**
 * @brief
 */
class perror : std::exception
{
public:
  perror(std::string const & label) : what_(label + ": " + std::strerror(errno))
  {
    ::perror(label.c_str());
  }

  virtual char const *
  what() const noexcept
  {
    return what_.c_str();
  }

private:
  std::string what_;
};

} /** !exceptions */
} /** !bokasafn */

#endif /** !BOKASAFN_EXCEPTIONS_HH_ */
