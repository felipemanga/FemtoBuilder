#include "BuildStep.hpp"

static class ImgToC : public BuildStep {
public:
    ImgToC() : BuildStep{"img-to-c"} {}

    bool run() override {
        App::get().verbose("to-do");
        return true;
    }
} step;


