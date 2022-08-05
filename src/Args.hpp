#pragma once

#include <vector>
#include <string>

class Args {
public:
  using ArgSet = std::vector<std::string>;
  using iterator = ArgSet::iterator;

  static inline ArgSet args;
  static inline std::string arg1;

  static void init(int argc, const char* argv[]) {
    for (int i = 0; i < argc; ++i) {
      args.push_back(argv[i]);
    }
  }

  static iterator find(const std::string& key) {
    for (auto it = args.begin(); it != args.end(); ++it) {
      if (*it == key)
	return it;
    }
    return args.end();
  }

  static const std::string& get(std::size_t index) {
    static std::string empty;
    return index >= args.size() ? empty : args[index];
  }

  static bool has(const std::string& key) {
    return find(key) != args.end();
  }

  struct Range {
    iterator _begin, _end;
    iterator begin() {return _begin;}
    iterator end() {return _end;}
  };

  static Range in(const std::string& key) {
    Range range;
    range._begin = find(key);
    range._end = args.end();
    if (range._begin == range._end)
      return range;
    range._begin++;
    for (auto end = range._begin; end != range._end; ++end) {
      if (!end->empty() && end->at(0) == '-') {
	range._end = end;
	break;
      }
    }
    return range;
  }
};
