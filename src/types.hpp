#pragma once

#include <algorithm>
#include <filesystem>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>

#include "Args.hpp"

using Path = std::filesystem::path;

using u32 = std::uint32_t;
using s32 = std::int32_t;
using u16 = std::uint16_t;
using s16 = std::int16_t;

inline std::string tolower(const std::string& str) {
    std::string ret = str;
    std::transform(ret.begin(), ret.end(), ret.begin(), [](char c){return std::tolower(c);});
    return ret;
}

inline std::string toupper(const std::string& str) {
    std::string ret = str;
    std::transform(ret.begin(), ret.end(), ret.begin(), [](char c){return std::toupper(c);});
    return ret;
}

inline std::string join(const std::vector<std::string>& vec, const std::string& sep) {
    std::string ret;
    if (!vec.empty()) {
        auto size = vec.size();
        auto totalSize = (size - 1) * sep.size();
        for (auto& str : vec) {
            totalSize += str.size();
        }
        ret.reserve(totalSize);
        ret = vec[0];
        for (auto i = 1; i < size; ++i) {
            ret += sep;
            ret += vec[i];
        }
    }
    return ret;
}

inline std::string quote(const std::string& str, const std::string& list = "\"") {
    std::string out;
    out.reserve(2 + str.size());
    out += '"';
    for (auto ch : str) {
        if (list.find(ch) != std::string::npos) {
            out += '\\';
        }
        out += ch;
    }
    out += '"';
    return out;
}
