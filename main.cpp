#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <windows.h>
#include <urlmon.h>
#include <map> // Hiányzó include a std::map használatához

#pragma comment(lib, "urlmon.lib")
namespace fs = std::filesystem;

// Modul interfész
class Module {
public:
    virtual void execute(const std::vector<std::string>& args) = 0;
    virtual ~Module() {}
};

// Modulkezelő
class ModuleManager {
public:
    static ModuleManager& getInstance() {
        static ModuleManager instance;
        return instance;
    }

    void loadModule(const std::string& moduleName) {
        // Csomag letöltése, felülírva a régit
        downloadModule(moduleName);
        std::string modulePath = "modules/" + moduleName + "/" + moduleName + ".dll";
        HMODULE hModule = LoadLibraryA(modulePath.c_str());
        if (hModule) {
            typedef Module* (*CreateModuleFunc)();
            CreateModuleFunc createModule = (CreateModuleFunc)GetProcAddress(hModule, "createModule");
            if (createModule) {
                if (modules.find(moduleName) != modules.end()) {
                    delete modules[moduleName];
                }
                modules[moduleName] = createModule();
                std::cout << "Modul betoltve: " << moduleName << std::endl;
            }
            else {
                std::cerr << "Modul betoltesi hiba: createModule funkciot nem talalom." << std::endl;
                FreeLibrary(hModule);
            }
        }
        else {
            std::cerr << "Modul betoltesi hiba: " << modulePath << std::endl;
        }
    }

    void executeModule(const std::string& moduleName, const std::vector<std::string>& args) {
        if (modules.find(moduleName) != modules.end()) {
            modules[moduleName]->execute(args);
        }
        else {
            std::cerr << "Modul nem talalhato: " << moduleName << std::endl;
        }
    }

    // A std::map-et publikus getterként szolgáltatjuk,
    // vagy módosítsuk a main függvényt, hogy ne férhessen hozzá közvetlenül
    const std::map<std::string, Module*>& getModules() const {
        return modules;
    }

private:
    ModuleManager() {}
    ModuleManager(const ModuleManager&) = delete;
    ModuleManager& operator=(const ModuleManager&) = delete;

    std::map<std::string, Module*> modules;

    void downloadModule(const std::string& moduleName) {
        std::string url = "https://github.com/DrJegesmedve/Mini-Shell/raw/main/modules/" + moduleName + "/" + moduleName + ".dll";
        std::string filePath = "modules/" + moduleName + "/" + moduleName + ".dll";
        fs::create_directories(fs::path(filePath).parent_path());
        HRESULT hr = URLDownloadToFileA(nullptr, url.c_str(), filePath.c_str(), 0, nullptr);
        if (SUCCEEDED(hr)) {
            std::cout << "Modul letoltve: " << moduleName << std::endl;
        }
        else {
            std::cerr << "Modul letoltesi hiba: " << moduleName << std::endl;
        }
    }
};

int main() {
    setlocale(LC_ALL, "hun");
    std::string command;
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, command);
        std::stringstream ss(command);
        std::string token;
        std::vector<std::string> args;
        while (ss >> token) {
            args.push_back(token);
        }
        if (args.empty()) continue;
        if (args[0] == "exit") {
            break;
        }
        else if (args[0] == "dwl") {
            if (args.size() > 1) {
                ModuleManager::getInstance().loadModule(args[1]);
            }
            else {
                std::cerr << "Hiba: dwl <modulnev>" << std::endl;
            }
        }
        // Módosítva, hogy a publikus getterrel férjen hozzá a modulokhoz
        else if (ModuleManager::getInstance().getModules().find(args[0]) != ModuleManager::getInstance().getModules().end()) {
            ModuleManager::getInstance().executeModule(args[0], std::vector<std::string>(args.begin() + 1, args.end()));
        }
        else {
            std::cerr << "Parancs nem talalhato: " << args[0] << std::endl;
        }
    }
    return 0;
}
