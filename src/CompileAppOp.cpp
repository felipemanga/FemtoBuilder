#include "App.hpp"
#include "AppOp.hpp"
#include "BuildStep.hpp"

class CompileAppOp : public AppOp {
public:
    CompileAppOp() : AppOp{"*"} {}

    void run() override {
        auto& app = App::get();
        auto& project = app.getProject();
        if (project.is_null()) {
            error("Could not load FemtoIDE project");
            return;
        }

        if (!app.var("target")) {
            auto it = project.find("target");
            if (it != project.end() && it->is_string()) {
                app.setVar("target", *it);
            }
        }

        app.verbose("Compiling for target: " + app.var("target", "?"));

        auto pipeline = loadPipeline(project);
        for (auto& stepName : pipeline) {
            auto step = BuildStep::get(stepName);
            if (!step) {
                error("Invalid pipeline step " + stepName);
                return;
            }
            app.verbose("Compile step " + stepName);
            if (!step->run()) {
                return;
            }
        }
    }

    std::vector<std::string> loadPipeline(JSON& project) {
        std::vector<std::string> out;
        JSON::iterator pipeline;
        std::string target = App::get().var("target", "");
        auto pipelines = project.find("pipelines");
        if (pipelines == project.end() || !pipelines->is_object()) {
            error("Could not read project pipelines");
            return out;
        }
        while (true) {
            pipeline = pipelines->find(target);
            if (pipeline == pipelines->end()) {
                error("Could not load pipeline " + target);
                return out;
            }
            if (pipeline->is_array())
                break;
            if (pipeline->is_string()) {
                target = *pipeline;
                continue;
            }
            error("Invalid pipeline " + target);
            return out;
        }
        for (auto& entry : *pipeline) {
            if (!entry.is_string()) {
                warn("Invalid pipeline step in " + target);
                continue;
            }
            out.push_back(entry);
        }
        return out;
    }
} compile;
