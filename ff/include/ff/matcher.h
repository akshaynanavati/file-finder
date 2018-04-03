#pragma once

#include <algorithm>
#include <string>

namespace ff {
enum class MatchType { Contains, EndsWith };

class Matcher {
  MatchType t_;
  std::string s_;

public:
  template <class String>
  Matcher(MatchType t, String &&s) : t_(t), s_(std::forward<String>(s)) {}

  bool matches(const std::string &s) const {
    const size_t n = s_.size();
    if (s.size() < n) {
      return false;
    }

    switch (t_) {
    case MatchType::Contains: {
      return std::search(s.begin(), s.end(), s_.begin(), s_.end()) != s.end();
    }
    case MatchType::EndsWith: {
      std::cout << "Matching " << s << " to " << s_ << std::endl;
      return std::equal(s.end() - n, s.end() + n, s_.begin());
    }
    }
    throw std::runtime_error("Impossible to reach here");
  }
};
} // namespace ff
