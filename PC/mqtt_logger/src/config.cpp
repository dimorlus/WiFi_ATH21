#include "config.h"
#include "logger.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <algorithm>
#include <regex>
#include <filesystem>
#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#else
#include <unistd.h>
#include <pwd.h>
#endif

static std::string getExecutableDir() {
#ifdef _WIN32
    char buf[MAX_PATH];
    DWORD len = GetModuleFileNameA(NULL, buf, MAX_PATH);
    if (len == 0 || len == MAX_PATH) return std::string();
    std::string p(buf, len);
    auto pos = p.find_last_of("\\/");
    if (pos == std::string::npos) return std::string();
    return p.substr(0, pos) + "\\";
#else
    char buf[4096];
    ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf)-1);
    if (len == -1) return std::string();
    buf[len] = '\0';
    std::string p(buf);
    auto pos = p.find_last_of('/');
    if (pos == std::string::npos) return std::string();
    return p.substr(0, pos) + "/";
#endif
}

static std::string expandEnv(const std::string& s) {
#ifdef _WIN32
    // Expand %VAR% and other Windows env patterns
    DWORD needed = ExpandEnvironmentStringsA(s.c_str(), nullptr, 0);
    if (needed == 0) return s;
    std::string out(needed, '\0');
    ExpandEnvironmentStringsA(s.c_str(), &out[0], needed);
    if (!out.empty() && out.back() == '\0') out.resize(out.size()-1);
    return out;
#else
    // Replace $VAR, ${VAR} and %VAR% with getenv results (simple)
    std::string out = s;
    std::regex env_re(R"(\$\{([^}]+)\}|\$([A-Za-z_][A-Za-z0-9_]*)|%([^%]+)%)");
    std::smatch m;
    std::string result;
    std::string::const_iterator searchStart(out.cbegin());
    while (std::regex_search(searchStart, out.cend(), m, env_re)) {
        result.append(searchStart, m[0].first);
        std::string var = m[1].length() ? m[1].str() : (m[2].length() ? m[2].str() : m[3].str());
        const char* val = getenv(var.c_str());
        if (val) result.append(val);
        searchStart = m.suffix().first;
    }
    result.append(searchStart, out.cend());
    return result;
#endif
}

Config::Config() {
    setDefaults();
}

void Config::setDefaults() {
    // Remove hard-coded MQTT defaults per request — keep empty so installer/config must set them.
    mqttUrl.clear();
    mqttUser.clear();
    mqttPassword.clear();
    topicBase.clear();

    // Keep data parsing defaults
    header = "SNTP;TEMP;HUM";
    searchPattern = "SNTP: ([0-9]+), TEMP: ([0-9\\.]+) C, HUM: ([0-9\\.]+) %, DEW: ([0-9\\.]+) C";
    replacePattern = "\\1;\\2;\\3";

    // logFolder left empty: will be set from INI or default to executable dir + "data"
    logFolder.clear();
}

std::string trim(const std::string& str) {
    if (str.empty()) return str;
    size_t first = 0;
    while (first < str.size() && std::isspace(static_cast<unsigned char>(str[first]))) ++first;
    if (first == str.size()) return std::string();
    size_t last = str.size() - 1;
    while (last > first && std::isspace(static_cast<unsigned char>(str[last]))) --last;
    return str.substr(first, last - first + 1);
}

bool Config::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        // Do not create defaults automatically — installer should place config.ini next to exe.
        std::cerr << "Config file not found: " << filename << std::endl;
        return false;
    }

    std::map<std::string, std::map<std::string, std::string>> sections;
    std::string currentSection;
    std::string line;

    while (std::getline(file, line)) {
        line = trim(line);

        if (line.empty() || line[0] == ';' || line[0] == '#') {
            continue; // Skip empty lines and comments
        }

        if (line[0] == '[' && line[line.length()-1] == ']') {
            currentSection = line.substr(1, line.length()-2);
            continue;
        }

        size_t equalPos = line.find('=');
        if (equalPos != std::string::npos) {
            std::string key = trim(line.substr(0, equalPos));
            std::string value = trim(line.substr(equalPos + 1));
            // strip optional surrounding quotes
            if (!value.empty() && value.front() == '"' && value.back() == '"') {
                value = value.substr(1, value.size()-2);
            }
            sections[currentSection][key] = value;
        }
    }

    // Apply settings
    if (sections.count("MQTT")) {
        auto& mqtt = sections["MQTT"];
        if (mqtt.count("URL")) mqttUrl = mqtt["URL"];
        if (mqtt.count("user")) mqttUser = mqtt["user"];
        if (mqtt.count("passwd")) mqttPassword = mqtt["passwd"];
        if (mqtt.count("topic_base")) topicBase = mqtt["topic_base"];
    }

    if (sections.count("DATA")) {
        auto& data = sections["DATA"];
        if (data.count("Header")) header = data["Header"];
        if (data.count("Search")) searchPattern = data["Search"];
        if (data.count("Replace")) replacePattern = data["Replace"];
    }

    if (sections.count("LOG")) {
        auto& lg = sections["LOG"];
        if (lg.count("Folder")) {
            logFolder = lg["Folder"];

            // remove UTF-8 BOM if present
            if (logFolder.size() >= 3 &&
                static_cast<unsigned char>(logFolder[0]) == 0xEF &&
                static_cast<unsigned char>(logFolder[1]) == 0xBB &&
                static_cast<unsigned char>(logFolder[2]) == 0xBF) {
                logFolder.erase(0, 3);
            }

            // expand environment variables
            logFolder = expandEnv(logFolder);

            // Remove surrounding quotes if any (extra safety)
            if (!logFolder.empty() && logFolder.front() == '"' && logFolder.back() == '"')
                logFolder = logFolder.substr(1, logFolder.size() - 2);

            // Aggressive trim: remove leading/trailing ASCII control/space (<= 0x20) and NBSP (0xA0)
            auto is_trim_char = [](unsigned char c){
                return c <= 0x20u || c == 0xA0u;
            };
            while (!logFolder.empty() && is_trim_char(static_cast<unsigned char>(logFolder.front())))
                logFolder.erase(logFolder.begin());
            while (!logFolder.empty() && is_trim_char(static_cast<unsigned char>(logFolder.back())))
                logFolder.pop_back();

            // Remove trailing separators (\\ or /), but keep drive-root like "C:\""
            while (!logFolder.empty()) {
                char last = logFolder.back();
                if (last == '\\' || last == '/') {
                    if (logFolder.size() == 3 && logFolder[1] == ':') {
                        // keep "C:\""
                        break;
                    }
                    logFolder.pop_back();
                    // after removing separator, also remove trailing control/space again
                    while (!logFolder.empty() && is_trim_char(static_cast<unsigned char>(logFolder.back())))
                        logFolder.pop_back();
                } else break;
            }
        }
    }

    // If there were no recognizable sections -> treat as malformed config
    if (sections.empty()) {
        std::cerr << "Config file found but appears empty or malformed: " << filename << std::endl;
        return false;
    }

    // Require [MQTT] section; provide clear error if missing
    if (!sections.count("MQTT")) {
        std::cerr << "Config file is missing required [MQTT] section: " << filename << std::endl;
        return false;
    }

    // If some required MQTT entries are missing, report them (but still allow continuation)
    {
        auto& mqtt = sections["MQTT"];
        if (!mqtt.count("URL") || !mqtt.count("topic_base")) {
            std::cerr << "Config [MQTT] section is missing required keys. Required: URL and topic_base. File: "
                      << filename << std::endl;
            return false;
        }
    }

    // If logFolder still empty, default to exe directory + data/
    if (logFolder.empty()) {
        logFolder = getExecutableDir();
        if (logFolder.empty()) logFolder = "./";
#ifdef _WIN32
        logFolder += "data\\";
#else
        logFolder += "data/";
#endif
    }
    
    // Ensure data folder exists
    try {
        std::filesystem::path p(logFolder);
        if (std::filesystem::exists(p)) {
            if (!std::filesystem::is_directory(p)) {
                std::cerr << "Configured log path exists but is not a directory: '" << logFolder
                          << "'. Falling back to executable data folder." << std::endl;
                // fallback to exeDir + data/
                std::string exeDir = getExecutableDir();
                if (exeDir.empty()) exeDir = "./";
    #ifdef _WIN32
                logFolder = exeDir + "data\\";
    #else
                logFolder = exeDir + "data/";
    #endif
                p = std::filesystem::path(logFolder);
            }
        }
        if (!std::filesystem::exists(p)) {
            std::filesystem::create_directories(p);
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Failed to create data directory '" << logFolder << "': " << e.what() << std::endl;
        return false;
    }

    return true;
}

bool Config::saveToFile(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }

    file << "[MQTT]\n";
    file << "URL=" << mqttUrl << "\n";
    file << "user=" << mqttUser << "\n";
    file << "passwd=" << mqttPassword << "\n";
    file << "topic_base=" << topicBase << "\n\n";

    file << "[DATA]\n";
    file << "Header=" << header << "\n";
    file << "Search=" << searchPattern << "\n";
    file << "Replace=" << replacePattern << "\n\n";

    file << "[LOG]\n";
    // write raw folder value; if it's under APP dir keep expanded form
    file << "Folder=" << logFolder << "\n";

    return true;
}

std::string Config::getConfigPath() {
    // Store config.ini next to the executable (not in %APPDATA%)
    std::string exeDir = getExecutableDir();
    if (exeDir.empty()) exeDir = "./";
#ifdef _WIN32
    return exeDir + "config.ini";
#else
    return exeDir + "config.ini";
#endif
}

std::string Config::getDataDirectory() {
    // Static method cannot access instance members directly.
    // Create a temporary Config, try to load config.ini next to the exe
    // and return the configured LOG Folder if present. Otherwise fall back
    // to exe directory + "data/".
    Config cfg;
    std::string cfgPath = Config::getConfigPath();
    if (!cfgPath.empty() && cfg.loadFromFile(cfgPath)) {
        if (!cfg.logFolder.empty()) {
            return cfg.logFolder;
        }
    }

    std::string exeDir = getExecutableDir();
    if (exeDir.empty()) exeDir = "./";
#ifdef _WIN32
    return exeDir + "data\\";
#else
    return exeDir + "data/";
#endif
}

// --- Add missing Config getters to satisfy linker ---

std::string Config::getMqttUrl() const {
    return mqttUrl;
}

std::string Config::getMqttUser() const {
    return mqttUser;
}

std::string Config::getMqttPassword() const {
    return mqttPassword;
}

std::string Config::getTopicBase() const {
    return topicBase;
}

std::string Config::getHeader() const {
    return header;
}

std::string Config::getSearchPattern() const {
    return searchPattern;
}

std::string Config::getReplacePattern() const {
    return replacePattern;
}

std::string Config::getLogFolder() const {
    return logFolder;
}

// Read config.ini, fill name map and log name pattern, and apply to logger
static inline std::string trim_cfg(const std::string &s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a==std::string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b-a+1);
}
static inline std::string unquote_cfg(const std::string &s) {
    std::string t = trim_cfg(s);
    if (t.size() >= 2 && ((t.front()=='"' && t.back()=='"') || (t.front()=='\'' && t.back()=='\'')))
        return t.substr(1, t.size()-2);
    return t;
}

void applyConfigToLogger(const std::string &cfgPath, DataLogger &logger)
{
    std::ifstream ifs(cfgPath);
    if (!ifs.is_open()) {
        std::cerr << "applyConfigToLogger: cannot open config file: " << cfgPath << std::endl;
        return;
    }

    std::map<std::string,std::string> nameMap;
    std::string logNamePattern; // if empty -> keep default ($DEV)
    std::string section;
    std::string line;
    while (std::getline(ifs, line)) {
        auto s = trim_cfg(line);
        if (s.empty() || s[0] == ';' || s[0] == '#') continue;
        if (s.front() == '[' && s.back() == ']') {
            section = s.substr(1, s.size()-2);
            std::transform(section.begin(), section.end(), section.begin(), ::toupper);
            continue;
        }
        auto eq = s.find('=');
        if (eq == std::string::npos) continue;
        std::string key = trim_cfg(s.substr(0, eq));
        std::string val = trim_cfg(s.substr(eq+1));
        if (section == "NAME") {
            std::string dev = key;
            std::string name = unquote_cfg(val);
            if (!dev.empty()) nameMap[dev] = name;
        } else if (section == "LOG") {
            std::string ukey = key;
            std::transform(ukey.begin(), ukey.end(), ukey.begin(), ::toupper);
            if (ukey == "NAME") {
                // keep raw (unquoted) pattern, but remove surrounding quotes if present
                logNamePattern = unquote_cfg(val.empty() ? val : val);
            }
        }
    }

    if (!nameMap.empty()) logger.setNameMap(nameMap);
    if (!logNamePattern.empty()) logger.setLogNamePattern(logNamePattern);

    std::cerr << "applyConfigToLogger: loaded " << nameMap.size() << " names";
    if (!logNamePattern.empty()) std::cerr << ", log name pattern = [" << logNamePattern << "]";
    std::cerr << std::endl;
}