#pragma once

#include <iostream>
#include <string>
#include "json.hpp"
#include "types.hpp"

using JSON = nlohmann::json;

class App {
protected:
    static inline App* instance = nullptr;

public:
    static App& get() {return *instance;}

    // vars
    virtual bool verbose() = 0;
    virtual bool verbose(const std::string& text) = 0;
    virtual void setVar(const std::string& key, const std::string& value) = 0;
    virtual std::string var(const std::string& key, const std::string& defaultValue) = 0;
    virtual std::optional<std::string> var(const std::string& key) = 0;
    virtual void dumpVars() = 0;
    virtual std::string normalize(const std::string& src) = 0;

    // misc
    virtual JSON& getProject() = 0;
    virtual std::optional<std::string> readFile(const Path& path) = 0;
    virtual void setReturnCode(int) = 0;
};

inline void error(const std::string& str) {
    App::get().setReturnCode(1);
    std::cerr << str << std::endl;
}

inline void warn(const std::string& str) {
    std::cerr << str << std::endl;
}
