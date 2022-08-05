#include "BuildStep.hpp"

static class CompileLD : public BuildStep {
public:
    CompileLD() : BuildStep{"compile-ld"} {}

    bool run() override {
        App::get().verbose("to-do");
        return true;
    }
} step;


