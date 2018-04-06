#pragma once

#include <iostream>

#include <algorithm>
#include <fnmatch.h>
#include <fstream>
#include <memory>
#include <regex>
#include <string>
#include <vector>

#include "ff/filesystem.h"
#include "ff/util.h"

namespace ff {
/**
 * Base class for all matchers. Can be used as a valid matcher which
 * matches nothing.
 */
class Matcher {
public:
  virtual ~Matcher() = default;
  virtual void addPattern(std::string &&pattern) {}
  virtual bool matches(const fs::File &file) const { return false; }
};

class GitMatcher : public Matcher {
  enum class GitMatchType { Dir, Glob, RelGlob };
  struct GitPattern {
    /* const */ GitMatchType type;
    /* const */ std::string patterns;

    GitPattern(const std::string &gitignoreRoot, std::string &&line) {
      if (line.back() == '/') {
        type = GitMatchType::Dir;
        line.pop_back();
      } else if (line.front() == '/') {
        type = GitMatchType::Glob;
        line = gitignoreRoot + line;
      } else if (line.find('/') == std::string::npos) {
        type = GitMatchType::RelGlob;
      } else {
        type = GitMatchType::Glob;
        line = gitignoreRoot + "/" + line;
      }
      patterns = std::move(line);
    }
  };

  std::string gitignoreRoot_;
  std::vector<GitPattern> patterns_;

public:
  GitMatcher(const std::string &gitignoreRoot) : gitignoreRoot_(gitignoreRoot) {
    patterns_.emplace_back(gitignoreRoot_, ".git/");
  }

  void addPattern(std::string &&line) override {
    patterns_.push_back(GitPattern(gitignoreRoot_, std::move(line)));
  }

  bool matches(const fs::File &file) const override {
    return std::any_of(
        patterns_.begin(), patterns_.end(), [&file](const GitPattern &gp) {
          const auto &p = file.path;
          switch (gp.type) {
          case GitMatchType::Dir: {
            auto begin = p.begin() + file.lastSlashIdx + 1;
            auto end = p.end();
            if (static_cast<size_t>(end - begin) != gp.patterns.size()) {
              return false;
            }
            return std::equal(begin, end, gp.patterns.begin());
          }
          case GitMatchType::Glob: {
            return fnmatch(gp.patterns.c_str(), p.c_str(), FNM_PATHNAME) == 0;
          }
          case GitMatchType::RelGlob: {
            return fnmatch(gp.patterns.c_str(),
                           p.c_str() + file.lastSlashIdx + 1,
                           FNM_PATHNAME) == 0;
          }
          default: { ff_UNREACHABLE(); }
          }
        });
  }
}; // namespace ff

class RegexMatcher : public Matcher {
  std::vector<std::regex> patterns_;

public:
  void addPattern(std::string &&pattern) override {
    patterns_.push_back(std::regex(pattern));
  }

  bool matches(const fs::File &file) const override {
    return std::any_of(patterns_.begin(), patterns_.end(),
                       [&file](const std::regex &pattern) {
                         auto s = file.path;
                         return std::regex_search(s.begin(), s.end(), pattern);
                       });
  }
};

std::unique_ptr<Matcher> createMatcher(const fs::File &dir) {
  {
    auto gitignore = dir / ".gitignore";
    if (gitignore.ft == fs::FileType::File) {
      auto matcher = std::make_unique<GitMatcher>(dir.path);
      std::ifstream f(gitignore.path);
      std::string line;
      while (std::getline(f, line)) {
        if (line.front() == '#' || line.empty()) {
          continue;
        }
        matcher->addPattern(std::move(line));
      }
      return matcher;
    }
  }
  // TODO parse hgignore
  return std::make_unique<Matcher>();
}
} // namespace ff
