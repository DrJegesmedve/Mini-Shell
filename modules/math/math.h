#pragma once
#include <vector>
#include <string>

class Module {
public:
    virtual void execute(const std::vector<std::string>& args) = 0;
    virtual std::string getVersion() const { return "Math Module Version 1.1.0"; }
    virtual ~Module() {}
};
