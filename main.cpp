#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <windows.h>
#include <urlmon.h>
#include <map>
#include <set>
#include <exception>
#include <algorithm>
#include <cctype>

#pragma comment(lib, "urlmon.lib")
namespace fs = std::filesystem;

// Segédfüggvény a whitespace levágására
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

// Modul interfész
class Module {
public:
	virtual void execute(const std::vector<std::string>& args) = 0;
	virtual std::string getVersion() const = 0; // Verzió lekérdezése
	virtual ~Module() {}
};

// Modulkezelő
class ModuleManager {
public:
	static ModuleManager& getInstance() {
		static ModuleManager instance;
		return instance;
	}

	// Segédfüggvény: Biztosítja, hogy a "modules" könyvtár létezik
	void ensureModulesDirectoryExists() {
		try {
			if (!fs::exists("modules")) {
				fs::create_directory("modules");
				std::cout << "\"modules\" mappa létrehozva." << std::endl;
			}
		}
		catch (const std::exception& e) {
			std::cerr << "Hiba a 'modules' mappa létrehozása során: " << e.what() << std::endl;
		}
	}

	// Modul letöltése (ha még nincs letöltve)
	void downloadModule(const std::string& moduleName) {
		try {
			ensureModulesDirectoryExists();
			std::string moduleDir = "modules/" + moduleName;
			std::string filePath = moduleDir + "/" + moduleName + ".dll";
			std::string url = "https://github.com/DrJegesmedve/Mini-Shell/raw/main/modules/" + moduleName + "/" + moduleName + ".dll";

			if (fs::exists(filePath)) {
				std::cerr << "Hiba: A modul már le van töltve: " << moduleName << std::endl;
				return;
			}

			fs::create_directories(moduleDir);
			HRESULT hr = URLDownloadToFileA(nullptr, url.c_str(), filePath.c_str(), 0, nullptr);
			if (SUCCEEDED(hr)) {
				std::cout << "Modul letöltve: " << moduleName << std::endl;
			}
			else {
				std::cerr << "Modul letöltési hiba: " << moduleName << std::endl;
			}
		}
		catch (const std::exception& e) {
			std::cerr << "Kivétel a modul letöltése során: " << e.what() << std::endl;
		}
	}

	// Modul betöltése a sessionbe
	void loadModule(const std::string& moduleName) {
		try {
			ensureModulesDirectoryExists();
			std::string modulePath = "modules/" + moduleName + "/" + moduleName + ".dll";
			if (!fs::exists(modulePath)) {
				std::cerr << "Modul betöltési hiba: " << moduleName << " nem létezik." << std::endl;
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
					std::cout << "Modul betöltve: " << moduleName << std::endl;
				}
				else {
					std::cerr << "Modul betöltési hiba: createModule függvényt nem találtam." << std::endl;
					FreeLibrary(hModule);
				}
			}
			else {
				std::cerr << "Modul betöltési hiba: " << modulePath << std::endl;
			}
		}
		catch (const std::exception& e) {
			std::cerr << "Kivétel a modul betöltése során: " << e.what() << std::endl;
		}
	}

	// Betölti az összes modult a "modules" könyvtárból
	void loadAllModules() {
		try {
			ensureModulesDirectoryExists();
			if (!fs::exists("modules") || fs::is_empty("modules")) {
				std::cerr << "Hiba: Egyetlen modul sincs letöltve." << std::endl;
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
			std::cerr << "Uknown Error (): " << e.what() << std::endl;
		}
	}

	// Modul frissítése: csak akkor, ha a modul le van töltve. Frissítés után nem töltődik be automatikusan.
	void updateModule(const std::string& moduleName) {
		try {
			ensureModulesDirectoryExists();
			std::string filePath = "modules/" + moduleName + "/" + moduleName + ".dll";
			if (!fs::exists(filePath)) {
				std::cerr << "ERROR: The modul is not installed " << moduleName << std::endl;
				return;
			}

			fs::remove(filePath);
			std::string url = "https://github.com/DrJegesmedve/Mini-Shell/raw/main/modules/" + moduleName + "/" + moduleName + ".dll";
			HRESULT hr = URLDownloadToFileA(nullptr, url.c_str(), filePath.c_str(), 0, nullptr);
			if (SUCCEEDED(hr)) {
				std::cout << "Modul updtated: " << moduleName << std::endl;
			}
			else {
				std::cerr << "Modul update error: " << moduleName << std::endl;
			}
		}
		catch (const std::exception& e) {
			std::cerr << "Uknown Error (): " << e.what() << std::endl;
		}
	}

	// Frissíti az összes letöltött modult
	void updateAllModules() {
		try {
			ensureModulesDirectoryExists();
			if (!fs::exists("modules") || fs::is_empty("modules")) {
				std::cerr << "ERROR: There is no any installed module" << std::endl;
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
			std::cerr << "Unknown Error (download modul): " << e.what() << std::endl;
		}
	}

	// Modul törlése (delete): a fájlrendszerből törli a DLL-t
	void removeModule(const std::string& moduleName) {
		try {
			ensureModulesDirectoryExists();
			std::string modulePath = "modules/" + moduleName + "/" + moduleName + ".dll";
			if (fs::exists(modulePath)) {
				fs::remove(modulePath);
				std::cout << "Modul deleted: " << moduleName << std::endl;
				if (modules.find(moduleName) != modules.end()) {
					delete modules[moduleName];
					modules.erase(moduleName);
				}
				loadedModules.erase(moduleName);
			}
			else {
				std::cerr << "ERROR: Modul is not installed: " << moduleName << std::endl;
			}
		}
		catch (const std::exception& e) {
			std::cerr << "Uknown Error (modul deletion): " << e.what() << std::endl;
		}
	}

	// Törli az összes modult (delete -all)
	void removeAllModules() {
		try {
			ensureModulesDirectoryExists();
			if (!fs::exists("modules") || fs::is_empty("modules")) {
				std::cerr << "ERROR: There is no any installed module" << std::endl;
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
			std::cerr << "Unkown Error (): " << e.what() << std::endl;
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
			std::cerr << "Unkown Error (): " << e.what() << std::endl;
		}
	}

	void executeModule(const std::string& moduleName, const std::vector<std::string>& args) {
		try {
			auto it = modules.find(moduleName);
			if (it == modules.end() || it->second == nullptr) {
				std::cerr << "ERROR: Modul is not loaded" << moduleName << std::endl;
				return;
			}
			it->second->execute(args);
		}
		catch (const std::exception& e) {
			std::cerr << "Kivétel a modul futtatása során: " << e.what() << std::endl;
		}
		catch (...) {
			std::cerr << "Uknown Error when tried to run the mÍ" << std::endl;
		}
	}

private:
	ModuleManager() {
		try {
			ensureModulesDirectoryExists();
			cleanupModules();
		}
		catch (const std::exception& e) {
			std::cerr << "Kivétel a modulkezelő inicializálása során: " << e.what() << std::endl;
		}
	}
	ModuleManager(const ModuleManager&) = delete;
	ModuleManager& operator=(const ModuleManager&) = delete;

	std::map<std::string, Module*> modules;
	std::set<std::string> loadedModules;
};

void printShellHelp() {
	std::cout << "Shell parancsok:" << std::endl;
	std::cout << "  exit                    - Kilépés a shellből" << std::endl;
	std::cout << "  help[/h/?]              - Súgó megjelenítése" << std::endl;
	std::cout << "  version[/h/?]           - Shell verzió kiírása" << std::endl;
	std::cout << "  dwl                     - Modul letöltése" << std::endl;
	std::cout << "  load                    - Modul betöltése" << std::endl;
	std::cout << "  update[/up]             - Modul frissítése" << std::endl;
	std::cout << "  delete[/del]            - Modul eltávolítása" << std::endl;
	std::cout << "  ls                      - Modulok listázása (pl.: ls mods)" << std::endl;
	std::cout << "  modules[/mods/ext]      - Modulok listázása" << std::endl;
	std::cout << "  <modulname> [args]      - Modul futtatása, kapcsolókkal (pl. -v a verzióhoz)" << std::endl;
}

int main() {
	setlocale(LC_ALL, "hun");
	std::string command;
	while (true) {
		std::cout << "> ";
		std::getline(std::cin, command);
		if (command.empty())
			continue;

		// Első szintű whitespace szerinti bontás
		std::vector<std::string> tokens;
		std::istringstream iss(command);
		std::string token;
		while (iss >> token) {
			tokens.push_back(token);
		}
		if (tokens.empty())
			continue;

		std::string mainCommand = tokens[0];

		// Shell parancsok kezelése
		if (mainCommand == "exit") {
			break;
		}
		else if (mainCommand == "help" || mainCommand == "h" || mainCommand == "?") {
			printShellHelp();
		}
		else if (mainCommand == "version" || mainCommand == "v" || mainCommand == "-v") {
			std::cout << "Shell Version 1.2.0" << std::endl;
		}
		else if (mainCommand == "ls") {
			if (tokens.size() >= 2 && (tokens[1] == "mods" || tokens[1] == "modules" || tokens[1] == "ext")) {
				if (fs::exists("modules") && !fs::is_empty("modules")) {
					for (const auto& entry : fs::directory_iterator("modules")) {
						if (entry.is_directory()) {
							std::cout << "- " << entry.path().filename().string() << std::endl;
						}
					}
				}
				else {
					std::cerr << "Hiba: Egyetlen modul sincs letöltve." << std::endl;
				}
			}
			else {
				std::cerr << "ERROR: ls <mods/modules/ext>" << std::endl;
			}
		}
		else if (mainCommand == "modules" || mainCommand == "mods" || mainCommand == "ext") {
			// Ugyanaz a listázás, mint az ls parancsnál
			if (fs::exists("modules") && !fs::is_empty("modules")) {
				for (const auto& entry : fs::directory_iterator("modules")) {
					if (entry.is_directory()) {
						std::cout << "- " << entry.path().filename().string() << std::endl;
					}
				}
			}
			else {
				std::cerr << "Hiba: Egyetlen modul sincs letöltve." << std::endl;
			}
		}
		// Kezeljük a modulokra vonatkozó parancsokat: dwl, load, update/up, delete/del
		else if (mainCommand == "dwl" || mainCommand == "load" || mainCommand == "update" ||
			mainCommand == "up" || mainCommand == "delete" || mainCommand == "del") {
			bool allFlag = false;
			std::vector<std::string> moduleNames;
			// Összefűzzük az összes argumentumot (az első után), majd vessző mentén feldaraboljuk
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
					// Ha a token nem kapcsoló, akkor modulnév
					if (mod[0] == '-' && mod != "-all") {
						// Egyéb kapcsolókat (ha jönne) most nem kezelünk
						continue;
					}
					moduleNames.push_back(mod);
				}
			}
			if (mainCommand == "dwl") {
				if (allFlag) {
					std::cerr << "Hiba: dwl -all nem támogatott." << std::endl;
				}
				else {
					if (moduleNames.empty()) {
						std::cerr << "ERROR: dwl <modulname>[, modulname...]" << std::endl;
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
						std::cerr << "ERROR: load <modulname>[, modulname...]" << std::endl;
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
						std::cerr << "ERROR: update/up <modulname>[, modulname...]" << std::endl;
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
						std::cerr << "ERROR: delete/del <modulname>[, modulname...]" << std::endl;
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
			// Ha a parancs nem ismert shell parancs, akkor feltételezzük, hogy modul futtatásáról van szó,
			// ahol több modul is lehet, vesszővel elválasztva, és kapcsolók (pl. -v) bárhol szerepelhetnek.
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
			// Az összes vesszővel elválasztott token közül a "-v", "v" vagy "version" kapcsolók felismerése,
			// a többi pedig modulnévnek számít.
			for (const auto& p : parts) {
				if (p == "-v" || p == "v" || p == "version")
					versionFlag = true;
				else
					moduleNames.push_back(p);
			}
			if (moduleNames.empty()) {
				std::cerr << "ERROR: Nem megfelelő modul parancs." << std::endl;
			}
			else {
				for (const auto& mod : moduleNames) {
					if (ModuleManager::getInstance().getModules().find(mod) == ModuleManager::getInstance().getModules().end()) {
						std::cerr << "Hiba: A modul nincs betöltve: " << mod << std::endl;
					}
					else {
						if (versionFlag) {
							std::cout << mod << " Version: "
								<< ModuleManager::getInstance().getModules().at(mod)->getVersion()
								<< std::endl;
						}
						else {
							// Ha extra argumentumokat szeretnénk továbbítani, azt tovább lehet fejleszteni
							ModuleManager::getInstance().executeModule(mod, {});
						}
					}
				}
			}
		}
	}
	return 0;
}


/* Extra features and current ones fixes:
- Add a command which clears the whole mess (delete every earlier command and answers). The command is simply 'c' or 'clear'. 
- Add a command for reboot. It reload the current state of the shell. Clears every earlier with 'c' then reloads the loaded modules. Could be simply 'rb'
- When delete a modul, delete the entire filesystem for that module like it was never there. Even the folder of the modul.
Some things about flags: Flags are only available in some commands. They can be placed anywhere after the first commandword. Like up -v
The flags are those which have the '-' character before them. Like '-v' is a flag. Multiple flag can be used at the same time for the most cases.
The '-v' flag doesn't work with other flags. The shell and each module has a version so this only works on them.
But functions like update or delete have the '-all' flag as a possible one. It means the command needs to be executed on every possible case.
The ls command lists everything on location (folders, files, etc.)
The ls command has a flag like attribute: the 'driver:'. By selecting a driver the ls knows which is the driver it should use. It also can be referenced as 'd:'. Like ls drive:C
There is another attribute for this command: the 'loc:'. It works exactly like the drive attribute but it takes an exact location.
This needs to be written beteen 's or "s Like ls loc:'C:\Users\kazig\OneDrive\Asztali gép\'
And yes, this list every files and folders in the place where the location is pointing.
There is a 'run' command. It runs, execute an executable file like: run "C:\Users\kazig\OneDrive\Asztali gép\kalkulatorv2.exe"
It does not need the 'loc:' like the ls but it can be added. If it takes a flag, then it's a must to use.
- Change the messages and everything to English.
*/
