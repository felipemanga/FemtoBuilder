#pragma once

#include <string>
#include <functional>

int shell(const std::string& cmd);
int shell(const std::string& cmd, const std::function<void(const char*)>& cb);
