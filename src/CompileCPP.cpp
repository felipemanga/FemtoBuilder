#include "App.hpp"
#include "BuildStep.hpp"
#include "Task.hpp"
#include "Shell.hpp"

#include <filesystem>
#include <regex>

static class CompileCPP : public BuildStep {
public:
    CompileCPP() : BuildStep{"compile-cpp"} {}


    struct Job {
        Path path;
        std::vector<std::string> flags;
        int errorCode;
        std::string ext;
        std::string compiler;
        std::string error;
    };
    std::vector<std::shared_ptr<Job>> jobs;
    std::vector<std::string> includeDirectories;
    std::string target;

    void addDirectory(const Path& path, const std::regex& ignore) {
        // App::get().verbose("Adding directory " + path.string());
        for (auto it : std::filesystem::directory_iterator{path}) {
            auto childPath = it.path();
            if (std::regex_search(childPath.c_str(), ignore) || childPath.filename().c_str()[0] == '.') {
                // App::get().verbose("Ignoring " + childPath.string());
                continue;
            }
            if (it.is_directory()) {
                addDirectory(childPath, ignore);
                continue;
            }
            auto extension = toupper(childPath.extension());
            if (extension.size() > 1 && extension != ".LD") {
                std::string ext = extension.c_str() + 1;
                if (auto compiler = App::get().var(ext + "-" + target)) {
                    auto job = std::make_shared<Job>();
                    job->compiler = App::get().normalize(*compiler);
                    job->path = childPath;
                    job->ext = ext;
                    jobs.push_back(job);
                // } else {
                //     App::get().verbose("No compiler for " + ext + "-" + target);
                }
            }
        }
        includeDirectories.push_back(path);
    }

    bool run() override {
        auto& app = App::get();
        auto& project = app.getProject();
        auto& libs = project["libs"];
        target = app.var("target", "");

        try {
            if (libs.is_object()) {
                auto libTargetEntry = app.var("target", "");
                for (JSON libList : std::vector<std::string>{libTargetEntry, "ALL"}) {
                    while (libList.is_string()) {
                        libList = libs[std::string(libList)];
                    }
                    if (libList.is_array()) {
                        for (auto& lib : libList) {
                            std::string ignore = "^\\\\..*";
                            std::string recurse;
                            if (lib.is_object()) {
                                if (lib["recurse"].is_string())
                                    recurse = lib["recurse"];
                                if (lib["ignore"].is_string())
                                    ignore = lib["ignore"];
                            } else if (lib.is_string()) {
                                recurse = lib;
                            }
                            if (recurse.empty()) {
                                std::cout << "Ignoring bad lib entry: " << lib << std::endl;
                                continue;
                            }
                            recurse = app.normalize(recurse);
                            addDirectory(recurse, std::regex(ignore));
                        }
                    }
                }
            }
            addDirectory(std::filesystem::current_path(), std::regex("^\\\\..*"));
        } catch (std::filesystem::filesystem_error ex) {
            error("Invalid path: " + ex.path1().string());
            return false;
        }

        TaskManager<std::shared_ptr<Job>, 4> taskMan;

        for (auto& job : jobs) {
            taskMan.add([job, &project, this]{
                std::vector<std::string> flags;
                for (auto& dir : includeDirectories)
                    flags.push_back(quote("-I" + dir));
                std::string ext = job->ext == "C" ? "C" : "CPP";
                auto it = project.find(ext + "Flags");
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

                job->compiler = join({
                            job->compiler,
                            quote(job->path.string()),
                            join(flags, " "),
                            "-o", "build_" + job->path.filename().string() + ".o"
                    }, " ");
                // std::cout << job->compiler << std::endl;
                job->errorCode = shell(job->compiler, [&](const char* str) {job->error += str;});
                std::cout << job->path.filename().string() << " " << job->errorCode << std::endl;
                return job;
            });
        }

        taskMan.wait([&](std::shared_ptr<Job> job){
            if (job->errorCode != 0) {
                std::cout << "\n\n" << job->compiler << "\nERROR: " << job->error << std::endl;
                error(std::to_string(job->errorCode));
                exit(1);
            }
        });

        return true;
    }
} step;


