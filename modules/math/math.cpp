#include <iostream>
#include <vector>
#include "math.h"

class MathModule : public Module {
public:
    void execute(const std::vector<std::string>& args) override {
        if (args.size() == 3 && args[1] == "+") {
            try {
                double a = std::stod(args[0]);
                double b = std::stod(args[2]);
                std::cout << a + b << std::endl;
            } catch (const std::invalid_argument& e) {
                std::cerr << "Hiba: Ervenytelen argumentumok." << std::endl;
            }
        } else {
            std::cout << "Math modul: osszeadas (a + b)" << std::endl;
        }
    }
};

extern "C" __declspec(dllexport) Module* createModule() {
    return new MathModule();
}
