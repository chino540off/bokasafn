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
#include <iostream>
#include <list>
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
  enum class fd_type_t
  {
    fd,
    timer,
  };

  struct fd_handler_t
  {
    fd_type_t type;
    epoll_event e;
    std::list<std::pair<int, func_t>> funcs;

    template <typename... Args>
    bool
    call(int events, Args &&... args)
    {
      bool again = false;

      for (auto const & it : funcs)
      {
        if (events & it.first)
        {
          again |= it.second(std::forward<Args>(args)...);

          if (!again)
            return again;
        }
      }

      return again;
    }

    void
    add(int events, func_t f)
    {
      e.events |= events;

      funcs.push_back({events, f});
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
      std::array<epoll_event, MAX_EVENTS> epoll_events{};

      auto n = epoll_wait(fd_, epoll_events.begin(), MAX_EVENTS, timeout.count());

      for (auto i = 0; i < n; ++i)
      {
        int fd = epoll_events[ i ].data.fd;
        int events = epoll_events[ i ].events;
        auto it = handlers_.find(fd);

        if (it == handlers_.end())
        {
          // there is no handle for this file descriptor
          continue;
        }

        // Call handle on the correct events
        bool again = it->second.call(events, fd);

        if (again)
        {
          if (epoll_ctl(fd_, EPOLL_CTL_MOD, fd, &it->second.e))
            throw bokasafn::exceptions::perror("epoll_ctl(MOD)");
        }
        else
        {
          remove(fd);
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
  add(int fd, func_t f, int flags = EPOLLIN, fd_type_t type = fd_type_t::fd)
  {
    auto it = handlers_.find(fd);

    if (it == handlers_.end())
    {
      epoll_event evt;

      memset(&evt, 0, sizeof(evt));
      evt.events = EPOLLONESHOT | flags; // Want to control after each call if we want to continue
      evt.data.fd = fd;

      fd_handler_t handle{type, evt, {}};
      handle.add(flags, f);

      handlers_.emplace(fd, handle);

      if (epoll_ctl(fd_, EPOLL_CTL_ADD, fd, &evt))
        throw bokasafn::exceptions::perror("epoll_ctl(ADD)");
    }
    else
    {
      it->second.add(flags, f);

      if (epoll_ctl(fd_, EPOLL_CTL_MOD, fd, &it->second.e))
        throw bokasafn::exceptions::perror("epoll_ctl(MOD)");
    }

    return fd;
  }

  void
  remove(int fd)
  {
    auto it = handlers_.find(fd);
    if (it == handlers_.end())
      return;

    if (it->second.type == fd_type_t::timer)
      close(fd);

    handlers_.erase(it);

    epoll_ctl(fd_, EPOLL_CTL_DEL, fd, nullptr);
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
      throw bokasafn::exceptions::perror("timer_create");

    struct itimerspec ts
    {
      {0, 0}, to_timespec(dur)
    };

    if (timerfd_settime(fd, 0, &ts, NULL) < 0)
      throw bokasafn::exceptions::perror("timer_settime");

    return add(fd, [dur, f, this](int fd) {
      size_t data = 0;

      // Read on timer fd to stop epoll
      if (read(fd, &data, sizeof(data)) < 0)
        throw bokasafn::exceptions::perror("timer_read");

      struct itimerspec ts
      {
        {0, 0}, to_timespec(dur)
      };

      auto ret = f(fd);

      if (ret)
      {
        if (timerfd_settime(fd, 0, &ts, NULL) < 0)
          throw bokasafn::exceptions::perror("timer_settime");
      }

      return ret;
    }, EPOLLIN, fd_type_t::timer);
  }

private:
  bool running_;
  int fd_;

  std::unordered_map<int, fd_handler_t> handlers_;
};

} /** !bokasafn */

#endif /** !BOKASAFN_EPOOL_HH_ */
