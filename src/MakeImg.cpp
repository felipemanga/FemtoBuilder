#include "BuildStep.hpp"

static class MakeIMG : public BuildStep {
public:
    MakeIMG() : BuildStep{"make-img"} {}

    bool run() override {
        App::get().verbose("to-do");
        return true;
    }
} step;


