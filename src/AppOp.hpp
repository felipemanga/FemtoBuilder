#pragma once

#include <unordered_map>
#include <string>

class AppOp {
    static std::unordered_map<std::string, AppOp*>& getOps() {
        static std::unordered_map<std::string, AppOp*> ops;
        return ops;
    }

public:
    static bool has(const std::string& name) {
        return getOps().find(name) != getOps().end();
    }

    static AppOp* get(const std::string& name) {
        auto it = getOps().find(name);
        if (it != getOps().end())
            return it->second;
        it = getOps().find("*");
        if (it != getOps().end())
            return it->second;
        return nullptr;
    }

    AppOp(const std::string& name) {
        getOps()[name] = this;
    }

    virtual void run() = 0;
};
