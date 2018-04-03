#pragma once

#include <atomic>
#include <queue>
#include <string>
#include <thread>

#include "ff/matcher.h"
#include "ff/queue.h"

namespace ff {
namespace detail {
const std::vector<Matcher> blacklist = {{MatchType::Contains, "buck-out/"}};

} // namespace detail
/**
 * Asynchronously filters out paths that match a glob in the input vector. It
 * accepts any string except the emtpy string which will stop the worker and
 * therefore should not be passed. Filter::finish should be called to stop the
 * filter worker. The Filter is safe to destruct even if finish has not been
 * called.
 */
template <class F> class Filter {
  static constexpr auto kStopFilter = "";

  ts::Queue<std::string> queue_;
  F cb_;
  std::thread t_;
  std::atomic_size_t nFiltered_{0};

  // TODO
  bool matches(const std::string &fname) {
    auto cb = [fname](const Matcher &matcher) {
      return matcher.matches(fname);
    };

    return std::none_of(detail::blacklist.begin(), detail::blacklist.end(), cb);
  }

  void worker() {
    while (true) {
      std::string pathname = queue_.pop();
      if (pathname == kStopFilter) {
        return;
      }

      if (matches(pathname)) {
        cb_(std::move(pathname));
      }
      ++nFiltered_;
    }
  }

public:
  Filter(F &&callback) : cb_(std::move(callback)), t_(&Filter::worker, this) {}
  ~Filter() { finish(); }

  template <class String> void operator()(String &&pathname) {
    queue_.push(std::forward<String>(pathname));
  }

  void finish() {
    if (t_.joinable()) {
      queue_.push(kStopFilter);
      t_.join();
    }
  }

  size_t nFiltered() { return nFiltered_.load(); }
};
} // namespace ff
