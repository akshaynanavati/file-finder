#pragma once

#include <array>
#include <cstring>
#include <string>

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

namespace ff {
namespace fs {
namespace detail {
/**
 * An iterator over a directory. If path does not point to a valid
 * directory, this is equivalent to the end iterator i.e. incrementing and
 * dereferencing are undefined.
 *
 * This class should never actually be accessed except as File::iterator.
 */
class DirIt {
  DIR *dir_ = nullptr;
  struct dirent *dirent_ = nullptr;

public:
  std::string parentPath;

  typedef std::forward_iterator_tag iterator_category;
  typedef struct dirent *pointer;
  typedef struct dirent &reference;

  DirIt() = default;
  DirIt(const char *path) : dir_(opendir(path)), parentPath(path) {
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
  DirIt(DirIt &&other) {
    dir_ = other.dir_;
    dirent_ = other.dirent_;
    other.dir_ = nullptr;
    other.dirent_ = nullptr;
    parentPath = std::move(other.parentPath);
  }
  DirIt &operator=(const DirIt &) = delete;
  DirIt &operator=(DirIt &&) = delete;

  DirIt &operator++() {
    dirent_ = readdir(dir_);
    return *this;
  }

  reference &operator*() const { return *dirent_; }

  pointer operator->() const { return dirent_; }

  bool operator==(const DirIt &other) { return dirent_ == other.dirent_; }

  bool operator!=(const DirIt &other) { return !operator==(other); }

  bool operator!() { return dirent_ == nullptr; }
};
} // namespace detail

enum class FileType { Dir = 0, File = 1, PermissionDenied = 2, Error = 3 };
constexpr std::array<const char *, 4> FileTypeStrings = {
    "Dir", "File", "PermissionDenied", "Error"};

/**
 * Represents a file on the file system. If path is a dir, the files underneath
 * it can be iterated through.
 */
struct File {
  std::string path;
  ssize_t lastSlashIdx = -1; // Position of last slash in path
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

  // TODO save this call by checking d_type in struct dirent.
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

  /**
   * Concatenates two File paths with a separator (/).
   */
  File operator/(const char *suffix) const { return concat(path, suffix); }

  iterator begin() { return detail::DirIt(path.c_str()); }

  iterator end() { return detail::DirIt(); }

  static File concat(const std::string &path1, const char *path2) {
    if (path1.back() == '/') {
      return File(path1 + path2, path1.length() - 1);
    }
    std::string newPath;
    newPath += path1;
    newPath += '/';
    newPath += path2;
    return File(newPath, path1.length());
  }
};
} // namespace fs
} // namespace ff
