#pragma once

#include <atomic>
#include <memory>
#include <queue>
#include <string>
#include <thread>

#include "ff/filesystem.h"
#include "ff/matcher.h"
#include "ff/queue.h"

namespace ff {
/**
 * Asynchronously filters out paths that match a glob in the input vector. It
 * accepts any string except the emtpy string which will stop the worker and
 * therefore should not be passed. Filter::finish should be called to stop the
 * filter worker. The Filter is safe to destruct even if finish has not been
 * called.
 */
template <class F> class Filter {
  const fs::File kStopFilter = fs::File();

  ts::Queue<fs::File> queue_;
  F cb_;
  std::thread t_;
  std::atomic_size_t nFiltered_{0};
  std::unique_ptr<Matcher> toIgnore_;

  bool shouldProcess(const fs::File &file) {
    if (file.ft == fs::FileType::Error ||
        file.ft == fs::FileType::PermissionDenied) {
      return false;
    }

    return !toIgnore_->matches(file);
  }

  void worker() {
    while (true) {
      auto file = queue_.pop();
      if (file.path.empty()) {
        return;
      }

      if (shouldProcess(file)) {
        cb_(std::move(file));
      }
      ++nFiltered_;
    }
  }

public:
  Filter(F &&callback, const fs::File &dir)
      : cb_(std::move(callback)), t_(&Filter::worker, this),
        toIgnore_(createMatcher(dir)) {}
  ~Filter() { finish(); }

  void operator()(fs::File &&file) { queue_.push(std::move(file)); }

  void finish() {
    if (t_.joinable()) {
      queue_.push(kStopFilter);
      t_.join();
    }
  }

  size_t nFiltered() { return nFiltered_.load(); }
};
} // namespace ff
