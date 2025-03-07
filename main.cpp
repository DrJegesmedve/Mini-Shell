#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <windows.h>
#include <urlmon.h>
#include <map>
#include <set>
#include <exception>
#include <algorithm>
#include <cctype>
#include <shellapi.h> // Needed for ShellExecuteA

#pragma comment(lib, "urlmon.lib")
namespace fs = std::filesystem;

// Helper function to trim whitespace from a string
std::string trim(const std::string& s) {
	auto start = s.begin();
	while (start != s.end() && std::isspace(*start)) {
		start++;
	}
	auto end = s.end();
	do {
		end--;
	} while (std::distance(start, end) > 0 && std::isspace(*end));
	return std::string(start, end + 1);
}

// Module interface
class Module {
public:
	virtual void execute(const std::vector<std::string>& args) = 0;
	virtual std::string getVersion() const = 0; // Get version info
	virtual ~Module() {}
};

// Module Manager
class ModuleManager {
public:
	static ModuleManager& getInstance() {
		static ModuleManager instance;
		return instance;
	}

	// Ensure that the "modules" directory exists
	void ensureModulesDirectoryExists() {
		try {
			if (!fs::exists("modules")) {
				fs::create_directory("modules");
			}
		}
		catch (const std::exception& e) {
			std::cerr << "Error creating 'modules' folder: " << e.what() << std::endl;
		}
	}

	// Download a module (if not already downloaded)
	void downloadModule(const std::string& moduleName) {
		try {
			ensureModulesDirectoryExists();
			std::string moduleDir = "modules/" + moduleName;
			std::string filePath = moduleDir + "/" + moduleName + ".dll";
			std::string url = "https://github.com/DrJegesmedve/Mini-Shell/raw/main/modules/" + moduleName + "/" + moduleName + ".dll";

			if (fs::exists(filePath)) {
				std::cerr << "Error: Module already installed" << std::endl;
				return;
			}

			fs::create_directories(moduleDir);
			HRESULT hr = URLDownloadToFileA(nullptr, url.c_str(), filePath.c_str(), 0, nullptr);
			if (SUCCEEDED(hr)) {
				std::cout << "Module installed" << std::endl;
			}
			else {
				std::cerr << "Download error" << std::endl;
			}
		}
		catch (const std::exception& e) {
			std::cerr << "Exception while downloading module: " << e.what() << std::endl;
		}
	}

	// Load a module into the session
	void loadModule(const std::string& moduleName) {
		try {
			ensureModulesDirectoryExists();
			std::string modulePath = "modules/" + moduleName + "/" + moduleName + ".dll";
			if (!fs::exists(modulePath)) {
				std::cerr << "Module load error: " << moduleName << " module does not installed." << std::endl;
				return;
			}
			HMODULE hModule = LoadLibraryA(modulePath.c_str());
			if (hModule) {
				typedef Module* (*CreateModuleFunc)();
				CreateModuleFunc createModule = (CreateModuleFunc)GetProcAddress(hModule, "createModule");
				if (createModule) {
					if (modules.find(moduleName) != modules.end()) {
						delete modules[moduleName];
					}
					modules[moduleName] = createModule();
					loadedModules.insert(moduleName);
					std::cout << "Module loaded" << std::endl;
				}
				else {
					std::cerr << "Module load error: createModule function not found." << std::endl;
					FreeLibrary(hModule);
				}
			}
			else {
				std::cerr << "Module load error: " << modulePath << std::endl;
			}
		}
		catch (const std::exception& e) {
			std::cerr << "Exception while loading module: " << e.what() << std::endl;
		}
	}

	// Load all modules from the "modules" folder
	void loadAllModules() {
		try {
			ensureModulesDirectoryExists();
			if (!fs::exists("modules") || fs::is_empty("modules")) {
				std::cerr << "Error: No available module" << std::endl;
				return;
			}
			for (const auto& entry : fs::directory_iterator("modules")) {
				if (entry.is_directory()) {
					std::string moduleName = entry.path().filename().string();
					loadModule(moduleName);
				}
			}
		}
		catch (const std::exception& e) {
			std::cerr << "Unknown Error while loading modules: " << e.what() << std::endl;
		}
	}

	// Update a module (only if it is already downloaded). Note: After update, it is not automatically loaded.
	void updateModule(const std::string& moduleName) {
		try {
			ensureModulesDirectoryExists();
			std::string filePath = "modules/" + moduleName + "/" + moduleName + ".dll";
			if (!fs::exists(filePath)) {
				std::cerr << "ERROR: Module is not available" << std::endl;
				return;
			}

			fs::remove(filePath);
			std::string url = "https://github.com/DrJegesmedve/Mini-Shell/raw/main/modules/" + moduleName + "/" + moduleName + ".dll";
			HRESULT hr = URLDownloadToFileA(nullptr, url.c_str(), filePath.c_str(), 0, nullptr);
			if (SUCCEEDED(hr)) {
				std::cout << "Module updated" << std::endl;
			}
			else {
				std::cerr << "Module update error" << std::endl;
			}
		}
		catch (const std::exception& e) {
			std::cerr << "Exception while updating module: " << e.what() << std::endl;
		}
	}

	// Update all downloaded modules
	void updateAllModules() {
		try {
			ensureModulesDirectoryExists();
			if (!fs::exists("modules") || fs::is_empty("modules")) {
				std::cerr << "ERROR: No available modules" << std::endl;
				return;
			}
			for (const auto& entry : fs::directory_iterator("modules")) {
				if (entry.is_directory()) {
					std::string moduleName = entry.path().filename().string();
					updateModule(moduleName);
				}
			}
		}
		catch (const std::exception& e) {
			std::cerr << "Unknown Error (module update): " << e.what() << std::endl;
		}
	}

	// Delete a module: remove the DLL and the entire module folder from the filesystem
	void removeModule(const std::string& moduleName) {
		try {
			ensureModulesDirectoryExists();
			std::string moduleFolder = "modules/" + moduleName;
			if (fs::exists(moduleFolder)) {
				fs::remove_all(moduleFolder);
				std::cout << "Module deleted" << std::endl;
				if (modules.find(moduleName) != modules.end()) {
					delete modules[moduleName];
					modules.erase(moduleName);
				}
				loadedModules.erase(moduleName);
			}
			else {
				std::cerr << "ERROR: Module is not available" << std::endl;
			}
		}
		catch (const std::exception& e) {
			std::cerr << "Exception while deleting module: " << e.what() << std::endl;
		}
	}

	// Delete all modules (remove the entire modules folder content)
	void removeAllModules() {
		try {
			ensureModulesDirectoryExists();
			if (!fs::exists("modules") || fs::is_empty("modules")) {
				std::cerr << "ERROR: No available module" << std::endl;
				return;
			}
			for (const auto& entry : fs::directory_iterator("modules")) {
				if (entry.is_directory()) {
					std::string moduleName = entry.path().filename().string();
					removeModule(moduleName);
				}
			}
		}
		catch (const std::exception& e) {
			std::cerr << "Unknown Error while deleting modules: " << e.what() << std::endl;
		}
	}

	const std::map<std::string, Module*>& getModules() const {
		return modules;
	}

	void cleanupModules() {
		try {
			ensureModulesDirectoryExists();
			if (!fs::exists("modules")) return;
			std::map<std::string, std::string> latestVersions;
			for (const auto& entry : fs::directory_iterator("modules")) {
				if (entry.is_directory()) {
					std::string moduleName = entry.path().filename().string();
					std::string modulePath = "modules/" + moduleName + "/" + moduleName + ".dll";
					if (fs::exists(modulePath)) {
						latestVersions[moduleName] = modulePath;
					}
				}
			}
			for (const auto& entry : fs::directory_iterator("modules")) {
				if (entry.is_directory()) {
					std::string moduleName = entry.path().filename().string();
					std::string modulePath = "modules/" + moduleName + "/" + moduleName + ".dll";
					if (latestVersions.find(moduleName) != latestVersions.end() && latestVersions[moduleName] != modulePath) {
						fs::remove(modulePath);
					}
				}
			}
		}
		catch (const std::exception& e) {
			std::cerr << "Unknown Error during cleanup: " << e.what() << std::endl;
		}
	}

	void executeModule(const std::string& moduleName, const std::vector<std::string>& args) {
		try {
			auto it = modules.find(moduleName);
			if (it == modules.end() || it->second == nullptr) {
				std::cerr << "ERROR: Module is not loaded" << std::endl;
				return;
			}
			it->second->execute(args);
		}
		catch (const std::exception& e) {
			std::cerr << "Exception while executing module: " << e.what() << std::endl;
		}
		catch (...) {
			std::cerr << "Unknown Error while running module command" << std::endl;
		}
	}

private:
	ModuleManager() {
		try {
			ensureModulesDirectoryExists();
			cleanupModules();
		}
		catch (const std::exception& e) {
			std::cerr << "Exception during ModuleManager initialization: " << e.what() << std::endl;
		}
	}
	ModuleManager(const ModuleManager&) = delete;
	ModuleManager& operator=(const ModuleManager&) = delete;

	std::map<std::string, Module*> modules;
	std::set<std::string> loadedModules;
};

void printShellHelp() {
	std::cout << "Shell commands:" << std::endl;
	std::cout << "  exit                        - Exit the shell" << std::endl;
	std::cout << "  help[/h/?]                  - Display help" << std::endl;
	std::cout << "  version[/v/-v]              - Display shell version" << std::endl;
	std::cout << "  install[/dwl]               - Download module(s)" << std::endl;
	std::cout << "  load                        - Load module(s)" << std::endl;
	std::cout << "  update[/up]                 - Update module(s)" << std::endl;
	std::cout << "  delete[/del]                - Delete module(s)" << std::endl;
	std::cout << "  ls [attribute]              - List modules or directory contents" << std::endl;
	std::cout << "     Use 'mods/modules/ext':  list modules." << std::endl;
	std::cout << "     Use 'drive:' or 'd:'     followed by a drive letter (e.g. ls drive:C) to list a drive." << std::endl;
	std::cout << "     Use 'loc:'               followed by a path (e.g. ls loc:\"C:\\My Folder\") to list that directory." << std::endl;
	std::cout << "  run <executable_path>       - Run an executable file" << std::endl;
	std::cout << "  c or clear                  - Clear the screen" << std::endl;
	std::cout << "  rb                          - Reboot the shell (clear screen and reload modules)" << std::endl;
	std::cout << "  <module_name> [args]        - Run a module with arguments (use -v for version)" << std::endl;
}

int main() {
	setlocale(LC_ALL, "English");
	std::string command;
	while (true) {
		std::cout << "> ";
		std::getline(std::cin, command);
		if (command.empty())
			continue;

		// Tokenize the command (splitting by whitespace)
		std::vector<std::string> tokens;
		std::istringstream iss(command);
		std::string token;
		while (iss >> token) {
			tokens.push_back(token);
		}
		if (tokens.empty())
			continue;

		std::string mainCommand = tokens[0];

		// Shell built-in commands
		if (mainCommand == "exit") {
			break;
		}
		else if (mainCommand == "help" || mainCommand == "h" || mainCommand == "?") {
			printShellHelp();
		}
		else if (mainCommand == "version" || mainCommand == "v" || mainCommand == "-v") {
			std::cout << "Shell Version 1.2.0" << std::endl;
		}
		else if (mainCommand == "c" || mainCommand == "clear") {
			system("cls");
		}
		else if (mainCommand == "rb") {
			system("cls");
			std::cout << "Rebooting shell and reloading modules..." << std::endl;
			ModuleManager::getInstance().loadAllModules();
		}
		else if (mainCommand == "ls") {
			// Enhanced ls command: list modules or directory contents
			if (tokens.size() >= 2) {
				std::string param = tokens[1];
				if (param.rfind("drive:", 0) == 0 || param.rfind("d:", 0) == 0) {
					std::string driveLetter;
					if (param.rfind("drive:", 0) == 0)
						driveLetter = param.substr(6);
					else
						driveLetter = param.substr(2);
					std::string path = driveLetter + ":\\";
					std::cout << "Listing drive " << driveLetter << ":" << std::endl;
					try {
						for (auto& entry : fs::directory_iterator(path)) {
							std::cout << entry.path().filename().string() << std::endl;
						}
					}
					catch (const std::exception& e) {
						std::cerr << "Error listing drive: " << e.what() << std::endl;
					}
				}
				else if (param.rfind("loc:", 0) == 0) {
					std::string location = param.substr(4);
					// Remove surrounding quotes if any
					if ((location.front() == '\'' && location.back() == '\'') ||
						(location.front() == '"' && location.back() == '"')) {
						location = location.substr(1, location.size() - 2);
					}
					std::cout << "Listing location: " << location << std::endl;
					try {
						for (auto& entry : fs::directory_iterator(location)) {
							std::cout << entry.path().filename().string() << std::endl;
						}
					}
					catch (const std::exception& e) {
						std::cerr << "Error listing location: " << e.what() << std::endl;
					}
				}
				else if (param == "mods" || param == "modules" || param == "ext") {
					if (fs::exists("modules") && !fs::is_empty("modules")) {
						for (const auto& entry : fs::directory_iterator("modules")) {
							if (entry.is_directory()) {
								std::cout << "- " << entry.path().filename().string() << std::endl;
							}
						}
					}
					else {
						std::cerr << "Error: No available module" << std::endl;
					}
				}
				else {
					// Treat parameter as a directory path
					std::string path = param;
					std::cout << "Listing location: " << path << std::endl;
					try {
						for (auto& entry : fs::directory_iterator(path)) {
							std::cout << entry.path().filename().string() << std::endl;
						}
					}
					catch (const std::exception& e) {
						std::cerr << "Error listing location: " << e.what() << std::endl;
					}
				}
			}
			else {
				// No parameter provided: list current directory
				std::string path = fs::current_path().string();
				std::cout << "Listing current directory: " << path << std::endl;
				try {
					for (auto& entry : fs::directory_iterator(path)) {
						std::cout << entry.path().filename().string() << std::endl;
					}
				}
				catch (const std::exception& e) {
					std::cerr << "Error listing current directory: " << e.what() << std::endl;
				}
			}
		}
		else if (mainCommand == "run") {
			// Run an executable file
			if (tokens.size() < 2) {
				std::cerr << "ERROR: run <executable_path>" << std::endl;
			}
			else {
				std::string execPath;
				for (size_t i = 1; i < tokens.size(); i++) {
					if (!execPath.empty())
						execPath += " ";
					execPath += tokens[i];
				}
				// Remove surrounding quotes if any
				if ((execPath.front() == '"' && execPath.back() == '"') ||
					(execPath.front() == '\'' && execPath.back() == '\'')) {
					execPath = execPath.substr(1, execPath.size() - 2);
				}
				std::cout << "Executing: " << execPath << std::endl;
				HINSTANCE result = ShellExecuteA(NULL, "open", execPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
				if ((int)result <= 32) {
					std::cerr << "Failed to execute: " << execPath << std::endl;
				}
			}
		}
		// Handle module-related commands: dwl, load, update/up, delete/del
		else if (mainCommand == "dwl" || mainCommand == "install" || mainCommand == "load" || mainCommand == "update" ||
			mainCommand == "up" || mainCommand == "delete" || mainCommand == "del") {
			bool allFlag = false;
			std::vector<std::string> moduleNames;
			// Combine all arguments (after the command) into one string and split by commas
			std::string argsCombined;
			for (size_t i = 1; i < tokens.size(); i++) {
				if (!argsCombined.empty())
					argsCombined += " ";
				argsCombined += tokens[i];
			}
			std::istringstream argStream(argsCombined);
			std::string part;
			while (std::getline(argStream, part, ',')) {
				std::string mod = trim(part);
				if (mod.empty())
					continue;
				if (mod == "-all") {
					allFlag = true;
				}
				else {
					// If the token is a flag (starting with '-') and not "-all", skip for now
					if (mod[0] == '-' && mod != "-all")
						continue;
					moduleNames.push_back(mod);
				}
			}
			if (mainCommand == "dwl" || mainCommand == "install") {
				if (allFlag) {
					std::cerr << "Error: install[/dwl] -all is not supported." << std::endl;
				}
				else {
					if (moduleNames.empty()) {
						std::cerr << "ERROR: install[/dwl]  <module_name>[, module_name...]" << std::endl;
					}
					else {
						for (const auto& mod : moduleNames) {
							ModuleManager::getInstance().downloadModule(mod);
						}
					}
				}
			}
			else if (mainCommand == "load") {
				if (allFlag) {
					ModuleManager::getInstance().loadAllModules();
				}
				else {
					if (moduleNames.empty()) {
						std::cerr << "ERROR: load <module_name>[, module_name...]" << std::endl;
					}
					else {
						for (const auto& mod : moduleNames) {
							ModuleManager::getInstance().loadModule(mod);
						}
					}
				}
			}
			else if (mainCommand == "update" || mainCommand == "up") {
				if (allFlag) {
					ModuleManager::getInstance().updateAllModules();
				}
				else {
					if (moduleNames.empty()) {
						std::cerr << "ERROR: update/up <module_name>[, module_name...]" << std::endl;
					}
					else {
						for (const auto& mod : moduleNames) {
							ModuleManager::getInstance().updateModule(mod);
						}
					}
				}
			}
			else if (mainCommand == "delete" || mainCommand == "del") {
				if (allFlag) {
					ModuleManager::getInstance().removeAllModules();
				}
				else {
					if (moduleNames.empty()) {
						std::cerr << "ERROR: delete/del <module_name>[, module_name...]" << std::endl;
					}
					else {
						for (const auto& mod : moduleNames) {
							ModuleManager::getInstance().removeModule(mod);
						}
					}
				}
			}
		}
		else {
			// Assume it's a module execution command.
			// Combine the entire command (splitting by commas for arguments and flags)
			std::string argsCombined;
			for (size_t i = 0; i < tokens.size(); i++) {
				if (!argsCombined.empty())
					argsCombined += " ";
				argsCombined += tokens[i];
			}
			std::istringstream argStream(argsCombined);
			std::vector<std::string> parts;
			while (std::getline(argStream, token, ',')) {
				std::string part = trim(token);
				if (!part.empty())
					parts.push_back(part);
			}
			bool versionFlag = false;
			std::vector<std::string> moduleNames;
			for (const auto& p : parts) {
				if (p == "-v" || p == "v" || p == "version")
					versionFlag = true;
				else
					moduleNames.push_back(p);
			}
			if (moduleNames.empty()) {
				std::cerr << "ERROR: Invalid module command." << std::endl;
			}
			else {
				for (const auto& mod : moduleNames) {
					if (ModuleManager::getInstance().getModules().find(mod) == ModuleManager::getInstance().getModules().end()) {
						std::cerr << "Error: Module not loaded: " << mod << std::endl;
					}
					else {
						if (versionFlag) {
							std::cout << mod << " Version: "
								<< ModuleManager::getInstance().getModules().at(mod)->getVersion()
								<< std::endl;
						}
						else {
							// Execute the module (extra arguments can be handled here)
							ModuleManager::getInstance().executeModule(mod, {});
						}
					}
				}
			}
		}
	}
	return 0;
}
