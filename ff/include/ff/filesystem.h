#pragma once

#include <string>

#include <dirent.h>
#include <sys/stat.h>

namespace ff {
namespace fs {
namespace detail {
class DirIt {
  DIR *dir_ = nullptr;
  struct dirent *dirent_ = nullptr;

public:
  typedef std::forward_iterator_tag iterator_category;
  typedef struct dirent *pointer;
  typedef struct dirent &reference;

  DirIt() = default;
  DirIt(const char *path) : dir_(opendir(path)), dirent_(readdir(dir_)) {}

  ~DirIt() noexcept {
    if (dir_) {
      closedir(dir_);
    }
  }

  DirIt(const DirIt &) = delete;
  DirIt(DirIt &&) = default;
  DirIt &operator=(const DirIt &) = delete;
  DirIt &operator=(DirIt &&) = default;

  DirIt &operator++() {
    dirent_ = readdir(dir_);
    return *this;
  }

  reference &operator*() const { return *dirent_; }

  pointer operator->() const { return dirent_; }

  bool operator==(const DirIt &other) { return dirent_ == other.dirent_; }

  bool operator!=(const DirIt &other) { return !operator==(other); }
};
} // namespace detail

struct Dir {
  const std::string path;

  typedef detail::DirIt iterator;

  template <class String>
  Dir(String &&path = ".") noexcept : path(std::forward<String>(path)) {}

  Dir(const Dir &) = default;
  Dir(Dir &&) = default;
  Dir &operator=(const Dir &) = default;
  Dir &operator=(Dir &&) = default;

  static bool is_dir(const std::string &path) {
    struct stat statbuf;

    if (lstat(path.c_str(), &statbuf)) {
      return false;
    }
    return S_ISDIR(statbuf.st_mode);
  }

  iterator begin() { return detail::DirIt(path.c_str()); }

  iterator end() { return detail::DirIt(); }
};

bool is_file(const std::string &path) {
  struct stat statbuf;

  if (lstat(path.c_str(), &statbuf)) {
    return false;
  }

  return S_ISREG(statbuf.st_mode);
}
} // namespace fs
} // namespace ff
