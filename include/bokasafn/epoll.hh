/**
 *  @file epool.hh
 *  @author Olivier Détour (detour.olivier@gmail.com)
 */
#ifndef BOKASAFN_EPOOL_HH_
#define BOKASAFN_EPOOL_HH_

#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <unistd.h>

#include <array>
#include <chrono>
#include <functional>
#include <system_error>
#include <unordered_map>

#include <bokasafn/exceptions.hh>

namespace bokasafn
{

/**
 * @brief
 */
template <std::size_t MAX_EVENTS>
class epoll
{
public:
  using func_t = std::function<bool(int)>;

private:
  struct fd_handler_t
  {
    func_t f;
    epoll_event e;

    template <typename... Args>
    bool
    call(int event, Args &&... args)
    {
      bool ret = false;

      if (event & EPOLLIN)
        ret |= f(std::forward<Args>(args)...);

      return ret;
    }
  };

public:
  epoll() : running_(false)
  {
    fd_ = epoll_create1(0);
    if (fd_ < 0)
      throw bokasafn::exceptions::perror("epoll_create1");
  }

  ~epoll() { close(fd_); }

public:
  template <typename P, typename R>
  void
  start(std::chrono::duration<P, R> dur)
  {
    std::chrono::milliseconds timeout = dur;

    running_ = true;

    while (running_)
    {
      std::array<epoll_event, MAX_EVENTS> events{};

      auto n = epoll_wait(fd_, events.begin(), MAX_EVENTS, timeout.count());

      for (auto i = 0; i < n; ++i)
      {
        int fd = events[ i ].data.fd;
        int event = events[ i ].events;
        auto it = handlers_.find(fd);

        if (it == handlers_.end())
        {
          // there is no handle for this file descriptor
          continue;
        }

        // Call handle on the correct event
        bool again = it->second.call(event, fd);

        if (again)
        {
          if (epoll_ctl(fd_, EPOLL_CTL_MOD, fd, &it->second.e))
            throw bokasafn::exceptions::perror("epoll_ctl(MOD)");
        }
        else
        {
          handlers_.erase(it);
        }
      }
    }
  }

  void
  stop()
  {
    running_ = false;
  }

public:
  int
  add(int fd, func_t f, int flags = EPOLLIN)
  {
    epoll_event evt;
    evt.events = flags | EPOLLONESHOT;
    evt.data.fd = fd;

    fd_handler_t handle{f, evt};

    if (epoll_ctl(fd_, EPOLL_CTL_ADD, fd, &evt))
      throw bokasafn::exceptions::perror("epoll_ctl(ADD)");

    handlers_.emplace(fd, handle);

    return fd;
  }

  void
  remove(int fd)
  {
    handlers_.erase(fd);

    if (epoll_ctl(fd_, EPOLL_CTL_DEL, fd, nullptr))
      throw bokasafn::exceptions::perror("epoll_ctl(DEL)");
  }

private:
  constexpr timespec
  to_timespec(std::chrono::nanoseconds dur)
  {
    auto secs = std::chrono::duration_cast<std::chrono::seconds>(dur);
    dur -= secs;

    return timespec{secs.count(), dur.count()};
  }

public:
  template <typename P, typename R>
  int
  timer(std::chrono::duration<P, R> dur, func_t f)
  {
    int fd = timerfd_create(CLOCK_MONOTONIC, 0);
    if (fd < 0)
      throw bokasafn::exceptions::perror("timerfd_create");

    struct itimerspec ts
    {
      to_timespec(dur), to_timespec(dur)
    };

    if (timerfd_settime(fd, 0, &ts, NULL) < 0)
      throw bokasafn::exceptions::perror("timerfd_settime");

    return add(fd, [f](int fd) {
      size_t data = 0;

      // Read on timer fd to stop epoll
      if (read(fd, &data, sizeof(data)) < 0)
        throw bokasafn::exceptions::perror("timer read");

      return f(fd);
    });
  }

private:
  bool running_;
  int fd_;

  std::unordered_map<int, fd_handler_t> handlers_;
};

} /** !bokasafn */

#endif /** !BOKASAFN_EPOOL_HH_ */
