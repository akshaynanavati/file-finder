#pragma once
#include <mutex>

namespace ff {
namespace ts {
/**
 * An RAII wrapper around a locked object. The constructor blocks
 * until the lock can be acquired at which point the object can be
 * modified. The lock is released on destruction of Locked.
 *
 * Provides a pointer API to the underlying object.
 */
template <class T> class Locked {
  std::lock_guard<std::mutex> lk_;
  T *obj_;

public:
  Locked(T *obj, std::mutex &mut) : lk_(mut), obj_(obj) {}
  T &operator*() { return *obj_; }
  T *operator->() { return obj_; }
};

/**
 * Synchronizes any object. The internals of the object cannot be accessed
 * except via a call to lock which returns a pointer type to the object.
 */
template <class T> class Synchronized {
  std::mutex mut_;
  T obj_;

public:
  template <class T_> Synchronized(T_ &&obj) : obj_(std::forward<T_>(obj)) {}

  Locked<T> lock() { return Locked(&obj_, mut_); }
};
} // namespace ts
} // namespace ff
