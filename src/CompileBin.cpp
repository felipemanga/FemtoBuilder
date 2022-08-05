#include "BuildStep.hpp"

static class CompileBin : public BuildStep {
public:
    CompileBin() : BuildStep{"compile-bin"} {}

    bool run() override {
        App::get().verbose("to-do");
        return true;
    }
} step;


