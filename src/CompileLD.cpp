#include "App.hpp"
#include "BuildStep.hpp"
#include <filesystem>
#include "Shell.hpp"

static class CompileLD : public BuildStep {
public:
    CompileLD() : BuildStep{"compile-ld"} {}

    bool run() override {
        auto& app = App::get();
        auto& project = app.getProject();
        auto target = app.var("target", "");
        auto linker = App::get().var("LD-" + target, "g++");

        std::vector<std::string> flags;

        if (auto it = project.find("LDFlags"); it != project.end() && it->is_object()) {
            auto& flagBlock = *it;
            it = flagBlock.find(target);
            if (it != flagBlock.end() && it->is_array()) {
                for (auto& entry : *it) {
                    if (entry.is_string())
                        flags.push_back(quote(app.normalize(entry)));
                }
            }
            it = flagBlock.find("RELEASE");
            if (it != flagBlock.end() && it->is_array()) {
                for (auto& entry : *it) {
                    if (entry.is_string())
                        flags.push_back(quote(app.normalize(entry)));
                }
            }
            it = flagBlock.find("ALL");
            if (it != flagBlock.end() && it->is_array()) {
                for (auto& entry : *it) {
                    if (entry.is_string())
                        flags.push_back(quote(app.normalize(entry)));
                }
            }
        }

        if (auto it = std::find(flags.begin(), flags.end(), "\"$objectFiles\""); it != flags.end()) {
            *it = join(app.list("objects"), " ");
        }

        std::string stdout;
        auto cmdline = quote(app.normalize(linker)) + " " + app.normalize(join(flags, " "));
        info(cmdline);
        auto errorCode = shell(cmdline, [&](const char* str) {stdout += str;});
        if (errorCode != 0) {
            error(stdout);
            exit(1);
        }

        return true;
    }
} step;
