#include "App.hpp"
#include "BuildStep.hpp"
#include "Shell.hpp"

static class CompileBin : public BuildStep {
public:
    CompileBin() : BuildStep{"compile-bin"} {}

    bool run() override {
        auto& app = App::get();
        auto& project = app.getProject();
        auto target = app.var("target", "");
        auto objdump = App::get().var("ELF2BIN-" + target, "g++");
        std::vector<std::string> flags;

        if (auto it = project.find("ELF2BINFlags"); it != project.end() && it->is_object()) {
            auto& flagBlock = *it;
            it = flagBlock.find(target);
            if (it != flagBlock.end() && it->is_array()) {
                for (auto& entry : *it) {
                    if (!entry.is_string())
                        continue;
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

        std::string stdout;
        auto cmdline = quote(app.normalize(objdump)) + " " + join(flags, " ");
        info(cmdline);
        auto errorCode = shell(cmdline, [&](const char* str) {stdout += str;});
        if (errorCode != 0) {
            error(stdout);
        }

        return true;
    }
} step;
