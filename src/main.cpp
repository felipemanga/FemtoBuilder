#include "App.hpp"
#include "AppOp.hpp"
#include "AppPath.hpp"

#include <exception>
#include <fstream>
#include <regex>
#include <sstream>

class AppImpl : public App {
    int returnCode = 0;
    Path root;
    JSON project;
    std::unordered_map<std::string, std::string> vars;

public:
    AppImpl() {
        instance = this;
    }

    int getReturnCode() {return returnCode;}

    void setReturnCode(int rc) override {
        if (returnCode == 0)
            returnCode = rc;
    }

    void run() {
        loadVars();
        auto opName = Args::get(1);
        verbose("Running Op: " + (AppOp::has(opName) ? opName : "compile"));
        AppOp::get(opName)->run();
    }

    void loadVars() {
        vars["appPath"] = getAppPath();
        #ifdef __WINDOWS__
        vars["os"] = "windows";
        vars["executableExt"] = ".exe";
        #endif
        #ifdef __linux__
        vars["os"] = "linux";
        vars["executableExt"] = "";
        #endif
        #ifdef __APPLE__
        vars["os"] = "darwin";
        vars["executableExt"] = "";
        #endif

        if (auto configStr = readFile(normalize("${appPath}/config.json"))) {
            try {
                auto config = JSON::parse(*configStr);
                if (config.is_object()) {
                    for (auto it = config.begin(); it != config.end(); ++it) {
                        if (it.value().is_string()) {
                            vars[it.key()] = it.value();
                        }
                    }
                }
            } catch(std::exception ex) {
                error("Error parsing config.json");
            }
        } else {
            error("Could not read " + normalize("${appPath}/config.json"));
        }

        std::string key;
        for (auto& arg : Args::args) {
            if (arg.empty())
                continue;
            if (arg[0] == '-') {
                key = arg.substr(1);
                continue;
            }
            if (key.empty()) {
                continue;
            }
            vars[key] = arg;
            key.clear();
        }
    }

    int isVerbose = -1;
    bool verbose() override {
        if (isVerbose == -1) {
            #ifdef _DEBUG
            auto str = var("verbose", "true");
            #else
            auto str = var("verbose", "false");
            #endif
            isVerbose = !str.empty() && std::tolower(str[0]) == 't';
        }
        return isVerbose != 0;
    }

    bool verbose(const std::string& text) override {
        if (!verbose())
            return false;
        std::cout << text << std::endl;
        return true;
    }

    void dumpVars() override {
        for (auto& entry : vars) {
            std::cout << entry.first << " = " << entry.second << std::endl;
        }
    }

    void setVar(const std::string& key, const std::string& value) override {
        vars[key] = value;
    }

    std::string var(const std::string& key, const std::string& defaultValue) override {
        auto it = vars.find(key);
        return it == vars.end() ? defaultValue : it->second;
    }

    std::optional<std::string> var(const std::string& key) override {
        auto it = vars.find(key);
        return it != vars.end() ? std::optional<std::string>{it->second} : std::nullopt;
    }

    std::string normalize(const std::string& src) override {
        std::string out = src;
        std::regex exp{"\\$\\{[^{}]+\\}"};
        int offset = 0;
        for (std::sregex_iterator it{src.begin(), src.end(), exp}; it != std::sregex_iterator{}; ++it) {
            std::string key{src.begin() + it->position() + 2, src.begin() + it->position() + it->length() - 1};
            auto value = var(key, "");
            out.replace(it->position() + offset, it->length(), value);
            offset += value.size() - it->length();
        }
        return out;
    }
    
    std::optional<std::string> readFile(const Path& path) override {
        std::ifstream stream{path};
        if (!stream.is_open()) {
            return {};
        }
        std::stringstream buffer;
        buffer << stream.rdbuf();
        return buffer.str();
    }

    JSON& getProject() override {
        if (!project.is_null())
            return project;

        auto projectPath = var("projectPath");
        if (projectPath) {
            std::error_code ec;
            std::filesystem::current_path(*projectPath, ec);
            if (ec) {
                error("Invalid project path: " + *projectPath);
                return project;
            }
        }

        root = std::filesystem::current_path();
        while (true) {
            if (auto str = readFile(root / "project.json")) {
                try {
                    project = JSON::parse(*str);
                    break;
                } catch (const std::exception& exception) {
                }
            }
            if (project.is_object()) {
                break;
            }
            auto parent = root.parent_path();
            if (projectPath || !root.has_parent_path() || parent == root) {
                break;
            }
            root = parent;
        }
        if (!project.is_object())
            return project;

        if (!projectPath)
            vars["projectPath"] = root;
        vars["projectName"] = root.filename();
        return project;
    }
};

int main(int argc, const char* argv[]) {
    Args::init(argc, argv);
    AppImpl app;
    app.run();
    return app.getReturnCode();
}

