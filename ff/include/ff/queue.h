#pragma once

#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>

namespace ff {
namespace ts {
/**
 * Single producer single consumer thread safe queue with a max size of N.
 * Implemented as a circular buffer with a head_ and tail_ pointer. Items are
 * popped off the head_ and pushed to the tail_.
 */
template <class T, size_t N = 4096> class Queue {
  std::atomic_size_t head_ = 0;
  std::atomic_size_t tail_ = 0;
  std::unique_ptr<T[]> buf_ = std::make_unique<T[]>(N);

public:
  bool empty() const { return head_.load() == tail_.load(); }
  bool full() const {
    auto tail = tail_.load();
    auto head = head_.load();
    return (tail == (N - 1) && head == 0) || (head - tail == 1);
  }

  T pop() {
    while (empty()) {
    }

    auto head = head_.load();
    T item = std::move(buf_[head]);
    head_.store((head + 1) % N);
    return item;
  }

  bool try_pop(T &item) {
    if (empty()) {
      return false;
    }

    auto head = head_.load();
    item = std::move(buf_[head]);
    head_.store((head + 1) % N);
    return true;
  }

  template <class T_> void push(T_ &&item) {
    while (full()) {
    }

    auto tail = tail_.load();
    buf_[tail] = std::forward<T_>(item);
    tail_.store((tail + 1) % N);
  }

  /**
   * Non-blocking push. If the push succeeds and true is returned, item is
   * moved and should not be used. If this function returns false, the item
   * is unchanged.
   */
  bool try_push(T &item) {
    if (full()) {
      return false;
    }

    auto tail = tail_.load();
    buf_[tail] = std::move(item);
    tail_.store((tail + 1) % N);
    return true;
  }
};
} // namespace ts
} // namespace ff
