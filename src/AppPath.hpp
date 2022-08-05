#pragma once

#include <string>

#if defined(__linux__) && !defined(ANDROID)

#include <limits.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

std::string getAppPath() {
  char exePath[PATH_MAX];
  ssize_t len = readlink("/proc/self/exe", exePath, sizeof(exePath));
  if (len == -1 || len == sizeof(exePath))
    len = 0;
  exePath[len] = 0;
  while (len > 0 && exePath[len] != '/') {
    len--;
  }
  exePath[len] = 0;
  return exePath;
}

#elif defined(__APPLE__)

#include <mach-o/dyld.h>
#include <sys/syslimits.h>

std::string getAppPath() {
  std::string exePath;
  exePath.resize(PATH_MAX);
  uint32_t size = exePath.size();
  while (_NSGetExecutablePath(&exePath[0], &size) == -1)
    exePath.resize(size);
  while (!exePath.empty() && exePath.back() != '/')
    exePath.pop_back();
  if (!exePath.empty())
    exePath.pop_back();
  return exePath;
}

#endif

