#include <iostream>
#include <string>

#include "ff/filesystem.h"
#include "ff/filter.h"
#include "ff/queue.h"

#include "gflags/gflags.h"

using namespace ff;

#define ff_MAJOR_VERSION "0";
#define ff_MINOR_VERSION "0";
#define ff_PATCH_VERSION "0";

DEFINE_bool(all, false, "Show all results");

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
  } while (cur && i < 2);

  return !cur || dots > 0;
}

int main(int argc, char *argv[]) {
  gflags::SetUsageMessage(buildUsageString(argv[0]));
  gflags::SetVersionString(buildVersionString());
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  std::string dirs(argc > 1 ? argv[1] : ".");
  if (!fs::Dir::is_dir(dirs)) {
    std::cerr << "Invalid directory: " << dirs << std::endl;
    gflags::ShowUsageWithFlagsRestrict(argv[0], "ff");
    return 1;
  }

  ts::Queue<std::string> toVisit;
  toVisit.push(std::move(dirs));

  Filter filter([&toVisit](std::string &&pathname) {
    if (fs::Dir::is_dir(pathname)) {
      toVisit.push(std::move(pathname));
    } else {
      std::cout << pathname << '\n';
    }
  });

  size_t processed = 0;
  std::string parent;
  while (1) {
    while (!toVisit.try_pop(parent)) {
      if (processed == filter.nFiltered()) {
        return 0;
      }
    }

    fs::Dir dir(parent);
    for (const auto &f : dir) {
      if (!isDotDir(f.d_name)) {
        filter(parent + "/" + f.d_name);
        ++processed;
      }
    }
  }

  return 0;
}
