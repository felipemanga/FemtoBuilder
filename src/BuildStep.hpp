#pragma once

#include <unordered_map>
#include <unordered_set>
#include <string>

#include "App.hpp"


class BuildStep {
    static std::unordered_map<std::string, BuildStep*>& getSteps() {
        static std::unordered_map<std::string, BuildStep*> steps;
        return steps;
    }

public:
    static inline std::unordered_set<std::string> projectFiles;

    static BuildStep* get(const std::string& name) {
        auto it = getSteps().find(name);
        if (it != getSteps().end())
            return it->second;
        it = getSteps().find("*");
        if (it != getSteps().end())
            return it->second;
        return nullptr;
    }

    BuildStep(const std::string& name) {
        getSteps()[name] = this;
    }

    virtual bool run() {
        App::get().verbose("to-do");
        return true;
    }
};
