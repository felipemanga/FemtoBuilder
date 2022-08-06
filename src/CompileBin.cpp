#include "App.hpp"
#include "BuildStep.hpp"
#include "Task.hpp"
#include "Shell.hpp"

#include <filesystem>
#include <regex>

static class CompileBin : public BuildStep {
public:
    CompileBin() : BuildStep{"compile-bin"} {}

    bool run() override {
        auto& app = App::get();
        auto& project = app.getProject();
        auto target = app.var("target", "");

        std::vector<std::string> flags;
        auto it = project.find("ELF2BINFlags");
        if (it != project.end() && it->is_object()) {
            auto& flagBlock = *it;
            it = flagBlock.find(target);
            if (it != flagBlock.end() && it->is_array()) {
                for (auto& entry : *it) {
                    if (entry.is_string())
                        flags.push_back(quote(entry));
                }
            }
            it = flagBlock.find("RELEASE");
            if (it != flagBlock.end() && it->is_array()) {
                for (auto& entry : *it) {
                    if (entry.is_string())
                        flags.push_back(quote(entry));
                }
            }
            it = flagBlock.find("ALL");
            if (it != flagBlock.end() && it->is_array()) {
                for (auto& entry : *it) {
                    if (entry.is_string())
                        flags.push_back(quote(entry));
                }
            }
        }
        auto compiler = App::get().normalize("${ELF2BIN-" + target + "} " + join(flags, " "));
        auto errorCode = shell(compiler);
        if (errorCode != 0) {
            std::cout << "\n\n" << compiler << std::endl;
            error(std::to_string(errorCode));
            exit(1);
        }

        return true;
    }
} step;
