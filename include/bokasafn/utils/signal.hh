/**
 *  @file signal.hh
 *  @author Olivier Détour (detour.olivier@gmail.com)
 */
#ifndef BOKASAFN_UTILS_SIGNAL_HH_
# define BOKASAFN_UTILS_SIGNAL_HH_

# include <functional>
# include <csignal>

namespace bokasafn {
namespace utils {

template <int S>
struct signal
{
  using sfx = void(int);
  typedef std::function<void(int)> fx_t;

  fx_t fx;

  static signal holder;
  static void handler(int sn) { holder.fx(sn); }
};
template <int S>
signal<S> signal<S>::holder;

// this is a scope
template <int S>
struct signal_handler
{
  using sfx = void(int);
  sfx * oldfx_;

  signal_handler(typename signal<S>::fx_t fx)
  {
    signal<S>::holder.fx = fx;
    oldfx_ = std::signal(S, &signal<S>::handler);
  }

  ~signal_handler()
  {
    std::signal(S, oldfx_);
  }
};

} /** !utils */
} /** !bokasafn */

#endif /** !BOKASAFN_UTILS_SIGNAL_HH_ */
