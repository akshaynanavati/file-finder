#pragma once
#include <mutex>

namespace ff {
namespace ts {
template <class T> class Locked {
  std::lock_guard<std::mutex> lk_;
  T *obj_;

public:
  Locked(T *obj, std::mutex &mut) : lk_(mut), obj_(obj) {}
  T &operator*() { return *obj_; }
  T *operator->() { return obj_; }
};

template <class T> class Synchronized {
  std::mutex mut_;
  T obj_;

public:
  template <class T_> Synchronized(T_ &&obj) : obj_(std::forward<T_>(obj)) {}

  Locked<T> lock() { return Locked(&obj_, mut_); }
};
} // namespace ts
} // namespace ff
