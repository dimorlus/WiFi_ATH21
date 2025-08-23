#include "logger.h"
#include "replace_macros.h"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <filesystem>
#include <thread>
#include <fstream>

#ifndef LOGGER_DEBUG
#define LOGGER_DEBUG 0
#endif

#if LOGGER_DEBUG
#define LDBG(x) (std::cerr << x)
#else
#define LDBG(x) ((void)0)
#endif

DataLogger::DataLogger(const std::string& dataDir, const std::string& header,
                       const std::string& searchPattern, const std::string& replacePattern)
    : dataDirectory(dataDir),
      csvHeader(header),
      searchRegex(searchPattern),
      replaceStr(replacePattern),
      logNamePattern("$DEV") // default: one file per device
{
    ensureDirectoryExists(dataDirectory);
}

DataLogger::~DataLogger() {
    // nothing persistent
}

void DataLogger::setPatterns(const std::string& searchPattern, const std::string& replacePattern) {
    searchRegex = std::regex(searchPattern);
    replaceStr = replacePattern;
}

void DataLogger::setLogNamePattern(const std::string& namePattern) {
    logNamePattern = namePattern.empty() ? "$DEV" : namePattern;
}

void DataLogger::setNameMap(const std::map<std::string,std::string>& names) {
    nameMap = names;
}

static std::string safe_join_path(const std::string &a, const std::string &b) {
    if (a.empty()) return b;
    std::filesystem::path p(a);
    p /= b;
    return p.string();
}

static std::string sanitize_filename(const std::string &name) {
    std::string r;
    for (char c : name) {
        // allow alnum, dot, hyphen, underscore; map others to underscore
        if (std::isalnum((unsigned char)c) || c=='.' || c=='-' || c=='_') r.push_back(c);
        else if (c==' ') r.push_back('_');
        else r.push_back('_');
    }
    if (r.empty()) r = "log";
    return r;
}

std::string DataLogger::getFilePath(const std::string& deviceId) {
    // expand logNamePattern using expandReplace (can use $NAME, $DEV, $TIME etc.)
    std::smatch emptyMatches;
    std::vector<std::string> emptyHeaders;
    std::string resolved = expandReplace(logNamePattern, deviceId, emptyHeaders, emptyMatches, nameMap);
    if (resolved.empty()) resolved = deviceId;
    std::string fname = sanitize_filename(resolved) + ".csv";
    return safe_join_path(dataDirectory, fname);
}

bool DataLogger::ensureDirectoryExists(const std::string& path) {
    try {
        if (path.empty()) return true;
        std::filesystem::path p(path);
        if (!std::filesystem::exists(p)) {
            return std::filesystem::create_directories(p);
        }
        return true;
    } catch (...) {
        return false;
    }
}

// optional helper if you keep old API: not used in this file
std::string DataLogger::extractDeviceId(const std::string& topic) {
    // naive: take last non-empty segment or first segment starting with "HT_"
    std::string cur;
    std::vector<std::string> parts;
    for (char c : topic) {
        if (c == '/') { parts.push_back(cur); cur.clear(); }
        else cur.push_back(c);
    }
    if (!cur.empty()) parts.push_back(cur);
    for (auto &p : parts) if (p.rfind("HT_", 0) == 0) return p;
    if (parts.size() >= 1) return parts.back();
    return std::string();
}

std::string DataLogger::processData(const std::string& rawData) {
    // stub if needed elsewhere
    return rawData;
}

bool DataLogger::logData(const std::string& deviceId, const std::string& rawData)
{
    std::lock_guard<std::mutex> g(logMutex);

    LDBG("DBG: logData called deviceId=[" << deviceId << "] rawData=[" << rawData << "]\n");

    std::smatch matches;
    try {
        bool matched = std::regex_search(rawData, matches, searchRegex);
        LDBG("DBG: regex_search matched=" << (matched? "yes":"no") << " (regex=[" << replaceStr << "])\n");
        if (!matched) {
            LDBG("DBG: payload did not match search regex\n");
            return false;
        }

        // build headerNames from csvHeader (split on ';' or ',')
        std::vector<std::string> headerNames;
        std::string cur;
        for (char c : csvHeader) {
            if (c == ';' || c == ',') {
                size_t a = cur.find_first_not_of(" \t\r\n");
                size_t b = cur.find_last_not_of(" \t\r\n");
                if (a != std::string::npos && b != std::string::npos)
                    headerNames.push_back(cur.substr(a, b-a+1));
                else if (!cur.empty())
                    headerNames.push_back(cur);
                cur.clear();
            } else cur.push_back(c);
        }
        if (!cur.empty()) {
            size_t a = cur.find_first_not_of(" \t\r\n");
            size_t b = cur.find_last_not_of(" \t\r\n");
            if (a != std::string::npos && b != std::string::npos)
                headerNames.push_back(cur.substr(a, b-a+1));
            else
                headerNames.push_back(cur);
        }

        // perform replacements (pass nameMap so $NAME works)
        std::string outLine = expandReplace(replaceStr, deviceId, headerNames, matches, nameMap);
        LDBG("DBG: expanded outLine=[" << outLine << "]\n");

        // write per-write (open/append/close)
        std::string filepath = getFilePath(deviceId);
        bool fileExists = std::filesystem::exists(filepath);
        bool wrote = false;

        for (int attempt = 0; attempt < 2 && !wrote; ++attempt) {
            std::ofstream ofs(filepath, std::ios::app);
            if (!ofs.is_open()) {
                LDBG("DBG: failed to open file (attempt " << attempt << "): " << filepath << "\n");
                if (attempt == 0) std::this_thread::sleep_for(std::chrono::milliseconds(50));
                continue;
            }

            if (!fileExists) {
                ofs << csvHeader << std::endl;
                fileExists = true;
            }

            ofs << outLine << std::endl;
            ofs.flush();

            if (ofs.good()) {
                std::cout << "Data logged for " << deviceId << ": " << outLine << std::endl;
                wrote = true;
                break;
            } else {
                LDBG("DBG: write failed (stream bad) attempt " << attempt << " for " << filepath << "\n");
                if (attempt == 0) std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        }

        if (!wrote) {
            LDBG("DBG: final write attempt failed for device " << deviceId << "\n");
            return false;
        }

        return true;
    } catch (const std::exception &e) {
        LDBG("DBG: exception in logData: " << e.what() << "\n");
        return false;
    }
}