#include "App.hpp"
#include "AppOp.hpp"
#include "BuildStep.hpp"

class InitAppOp : public AppOp {
public:
  InitAppOp() : AppOp{"init"} {}

  void run() override {
    auto& app = App::get();
    auto name = Args::get(2);
    if (name.empty()) {
      error("Usage: " + Args::get(0) + " init [projectName]");
      return;
    }
    auto tmpl = app.var("template", "1-C++ Game");
    std::cout << "Project " << name << " created from " << tmpl << "." << std::endl;
  }
} init;
