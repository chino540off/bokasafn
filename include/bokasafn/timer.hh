/**
 *  @file timer.hh
 *  @author Olivier Détour (detour.olivier@gmail.com)
 */
#ifndef BOKASAFN_TIMER_HH_
# define BOKASAFN_TIMER_HH_

# include <atomic>
# include <thread>

namespace bokasafn {

/**
 * @brief 
 */
class timer
{
  public:
    timer(): clear_(false) { }

  public:
    template <typename F>
    void timeout(int delay, F f)
    {
      clear_ = false;

      thread_ = std::thread([=]()
      {
        if(this->clear_)
          return;

        std::this_thread::sleep_for(std::chrono::milliseconds(delay));

        if(this->clear_)
          return;

        f();
      });
    }

    template <typename F>
    void interval(int interval, F f)
    {
      clear_ = false;

      thread_ = std::thread([=]()
      {
        while (true)
        {
          if(this->clear_)
            return;

          std::this_thread::sleep_for(std::chrono::milliseconds(interval));

          if(this->clear_)
            return;

          f();
        }
      });
    }

    void stop()
    {
      clear_ = true;
    }

    void wait()
    {
      thread_.join();
    }

  private:
    std::atomic<bool> clear_;
    std::thread thread_;
};

} /** !bokasafn */

#endif /** !BOKASAFN_TIMER_HH_ */
