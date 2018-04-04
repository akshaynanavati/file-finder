#include <iostream>
#include <string>

#include "ff/filesystem.h"
#include "ff/filter.h"
#include "ff/queue.h"

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

int main(int argc, char *argv[]) {
#ifdef ff_GFLAGS
  gflags::SetUsageMessage(buildUsageString(argv[0]));
  gflags::SetVersionString(buildVersionString());
  gflags::ParseCommandLineFlags(&argc, &argv, true);
#endif

  std::string dirs(argc > 1 ? argv[1] : ".");
  auto dir = fs::File(std::move(dirs));
  if (dir.ft != fs::FileType::Dir) {
    std::cerr << "Invalid directory: " << dirs << std::endl;
#ifdef ff_GFLAGS
    gflags::ShowUsageWithFlagsRestrict(argv[0], "ff");
#else
    std::cerr << "USAGE: " << argv[0] << " <dir>" << std::endl;
#endif
    return 1;
  }

  ts::Queue<fs::File> toVisit;
  toVisit.push(std::move(dir));

  Filter filter([&toVisit](fs::File &&file) {
    switch (file.ft) {
    case fs::FileType::Dir: {
      toVisit.push(std::move(file));
      break;
    }
    case fs::FileType::File: {
      std::cout << file.path << '\n';
      break;
    }
    default: { throw std::runtime_error("Impossible to reach"); }
    }
  });

  size_t processed = 0;
  fs::File parent;
  while (1) {
    while (!toVisit.try_pop(parent)) {
      if (processed == filter.nFiltered()) {
        return 0;
      }
    }

    for (const auto &f : parent) {
      if (!isDotDir(f.d_name)) {
        filter(parent / f.d_name);
        ++processed;
      }
    }
  }

  return 0;
}
