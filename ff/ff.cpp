#include <iostream>
#include <string>

#include "ff/filesystem.h"
#include "ff/filter.h"
#include "ff/queue.h"
#include "ff/util.h"

#ifdef ff_GFLAGS
#include "gflags/gflags.h"
#endif

using namespace ff;

#define ff_MAJOR_VERSION "0";
#define ff_MINOR_VERSION "0";
#define ff_PATCH_VERSION "0";

#ifdef ff_GFLAGS
DEFINE_bool(all, false, "Show all results");
#endif

constexpr auto kDot = '.';

std::string buildUsageString(const char *argv0) {
  std::string usage = "USAGE: ";
  usage += argv0;
  usage += " <dir>";
  return usage;
}

std::string buildVersionString() {
  std::string version;
  version += ff_MAJOR_VERSION;
  version += kDot;
  version += ff_MINOR_VERSION;
  version += kDot;
  version += ff_PATCH_VERSION;
  return version;
}

bool isDotDir(const char *dir) {
  char cur;
  size_t i = 0;
  size_t dots = 0;
  do {
    cur = dir[i++];
    if (cur == '.') {
      ++dots;
    }
  } while (cur && i < 3);

  return !cur && dots > 0;
}

int allResults(fs::File &&dir) {
  std::vector<fs::File> toVisit;
  toVisit.push_back(std::move(dir));

  while (!toVisit.empty()) {
    auto file = std::move(toVisit.back());
    toVisit.pop_back();

    switch (file.ft) {
    case fs::FileType::Dir: {
      for (const auto &child : file) {
        if (isDotDir(child.d_name)) {
          continue;
        }

        toVisit.push_back(file / child.d_name);
      }
      break;
    }
    case fs::FileType::File: {
      std::cout << file.path << '\n';
      break;
    }
    default: { continue; }
    }
  }
  return 0;
}

int main(int argc, char *argv[]) {
#ifdef ff_GFLAGS
  gflags::SetUsageMessage(buildUsageString(argv[0]));
  gflags::SetVersionString(buildVersionString());
  gflags::ParseCommandLineFlags(&argc, &argv, true);
#endif

  std::string dirs(argc > 1 ? argv[1] : ".");
  auto dir = fs::File(std::move(dirs), -1);
  if (dir.ft != fs::FileType::Dir) {
    std::cerr << "Invalid directory: " << dirs << std::endl;
#ifdef ff_GFLAGS
    gflags::ShowUsageWithFlagsRestrict(argv[0], "ff");
#else
    std::cerr << "USAGE: " << argv[0] << " <dir>" << std::endl;
#endif
    return 1;
  }

#ifdef ff_GFLAGS
  if (FLAGS_all) {
    return allResults(std::move(dir));
  }
#endif

  ts::Queue<fs::File> filtered;

  Filter filter(
      [&filtered](fs::File &&file) {
        switch (file.ft) {
        case fs::FileType::Dir: {
          filtered.push(std::move(file));
          break;
        }
        case fs::FileType::File: {
          std::cout << file.path << '\n';
          break;
        }
        default: { ff_UNREACHABLE(); }
        }
      },
      dir);

  filtered.push(std::move(dir));
  size_t processed = 0;
  std::vector<fs::File::iterator> toVisit;
  while (1) {
    if (!toVisit.empty() && !filter.full()) {
      auto &next = toVisit.back();
      if (!next) {
        toVisit.pop_back();
        continue;
      }

      if (!isDotDir(next->d_name)) {
        // Must succeed as filter was checked to not be full
        filter(fs::File::concat(next.parentPath, next->d_name));
        ++processed;
      }
      ++next;
    }

    if (filtered.empty()) {
      // Need to check empty again. If filter.nFiltered was incremented, this
      // could be non-empty and a memory fence between nFiltered being
      // incremented and the queue internals guarantees it will be udpated
      // here.
      if (toVisit.empty() && processed == filter.nFiltered() &&
          filtered.empty()) {
        return 0;
      }
    } else {
      toVisit.push_back(filtered.pop().begin());
    }
  }

  return 0;
}
