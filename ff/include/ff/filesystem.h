#pragma once

#include <array>
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
  DirIt(const char *path) : dir_(opendir(path)) {
    if (dir_) {
      dirent_ = readdir(dir_);
    }
  }

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

enum class FileType { Dir = 0, File = 1, PermissionDenied = 2, Error = 3 };
constexpr std::array<const char *, 4> FileTypeStrings = {
    "Dir", "File", "PermissionDenied", "Error"};

struct File {
  std::string path;
  ssize_t lastSlashIdx = -1;
  FileType ft;

  typedef detail::DirIt iterator;

  File() noexcept : path("") {}

  template <class String>
  File(String &&path, ssize_t lastSlashIdxP) noexcept
      : path(std::forward<String>(path)), lastSlashIdx(lastSlashIdxP),
        ft(file_type()) {}

  File(const File &) = default;
  File(File &&) = default;
  File &operator=(const File &) = default;
  File &operator=(File &&) = default;

  FileType file_type() {
    struct stat statbuf;

    if (stat(path.c_str(), &statbuf)) {
      return FileType::Error;
    }

    mode_t mode = statbuf.st_mode;
    if ((mode & S_IRUSR) == 0) {
      return FileType::PermissionDenied;
    }

    if (S_ISDIR(mode)) {
      return FileType::Dir;
    }

    if (S_ISREG(mode)) {
      return FileType::File;
    }

    return FileType::Error;
  }

  File operator/(const char *suffix) const {
    if (path.back() == '/') {
      return File(path + suffix, path.length() - 1);
    }
    return File(path + "/" + suffix, path.length());
  }

  iterator begin() { return detail::DirIt(path.c_str()); }

  iterator end() { return detail::DirIt(); }
};
} // namespace fs
} // namespace ff
