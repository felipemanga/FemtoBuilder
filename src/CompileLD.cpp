#include "App.hpp"
#include "BuildStep.hpp"
#include "Task.hpp"
#include "Shell.hpp"

#include <filesystem>
#include <regex>


static class CompileLD : public BuildStep {
public:
    CompileLD() : BuildStep{"compile-ld"} {}

    std::vector<std::string> objectFiles;
    std::string target;

    void addDirectory(const Path& path) {
        for (auto it : std::filesystem::directory_iterator{path}) {
            auto childPath = it.path();
            if (it.is_directory()) {
                continue;
            }
            auto extension = toupper(childPath.extension());
            if (extension != ".O") {
                continue;
            }
            objectFiles.push_back(quote(childPath));
        }
    }

    bool run() override {
        auto& app = App::get();
        auto& project = app.getProject();
        target = app.var("target", "");
        try {
            addDirectory(std::filesystem::current_path());
        } catch (std::filesystem::filesystem_error ex) {
            error("Invalid path: " + ex.path1().string());
            return false;
        }

        std::vector<std::string> flags;
        auto it = project.find("LDFlags");
        if (it != project.end() && it->is_object()) {
            auto& flagBlock = *it;
            it = flagBlock.find(target);
            if (it != flagBlock.end() && it->is_array()) {
                for (auto& entry : *it) {
                    if (entry.is_string())
                        flags.push_back(entry);
                }
            }
            it = flagBlock.find("RELEASE");
            if (it != flagBlock.end() && it->is_array()) {
                for (auto& entry : *it) {
                    if (entry.is_string())
                        flags.push_back(entry);
                }
            }
            it = flagBlock.find("ALL");
            if (it != flagBlock.end() && it->is_array()) {
                for (auto& entry : *it) {
                    if (entry.is_string())
                        flags.push_back(entry);
                }
            }
        }
        for (auto& flag : flags) {
            if (flag == "$objectFiles") {
                flag = join(objectFiles, " ");
            } else {
                flag = quote(App::get().normalize(flag));
            }
        }
        auto linker = App::get().var("LD-" + target, "ld");
        auto compiler = linker + " " + join(flags, " ");
        auto errorCode = shell(compiler);
        if (errorCode != 0) {
            std::cout << "\n\n" << compiler << std::endl;
            error(std::to_string(errorCode));
            exit(1);
        }

        return true;
    }
} step;
