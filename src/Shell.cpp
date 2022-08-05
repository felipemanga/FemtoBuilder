#include "Shell.hpp"

#include <cstdio>
#include <ios>
#include <iostream>
#include <stdio.h>

int shell(const std::string& cmd) {
  int result = -1;
  auto onClose = [&](FILE* file) {
    result = pclose(file);
    result = WEXITSTATUS(result);
  };
  std::unique_ptr<FILE, decltype(onClose)> pipe(popen(cmd.c_str(), "r"), onClose);
  if (pipe) {
    std::cout << std::unitbuf;
    char buffer[2];
    while (fgets(buffer, 2, pipe.get()) != nullptr) {
      std::cout << buffer;
    }
    std::cout << std::nounitbuf;
  }
  return result;
}

int shell(const std::string& cmd, const std::function<void(const char*)>& cb) {
  int result = -1;
  {
      auto onClose = [&](FILE* file) {
          result = pclose(file);
          result = WEXITSTATUS(result);
      };
      std::unique_ptr<FILE, decltype(onClose)> pipe(popen(cmd.c_str(), "r"), onClose);
      if (pipe) {
          char buffer[1024];
          while (fgets(buffer, sizeof(buffer), pipe.get()) != nullptr) {
              cb(buffer);
          }
      }
  }
  return result;
}
