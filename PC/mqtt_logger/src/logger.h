#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <regex>
#include <mutex>
#include <filesystem>
#include <map>

class DataLogger {
public:
    DataLogger(const std::string& dataDir, const std::string& header,
               const std::string& searchPattern, const std::string& replacePattern);
    ~DataLogger();

    void setPatterns(const std::string& searchPattern, const std::string& replacePattern);
    void setLogNamePattern(const std::string& namePattern); // e.g. "$NAME" or "$DEV" or "MyLog"
    void setNameMap(const std::map<std::string,std::string>& names);

    bool logData(const std::string& deviceId, const std::string& rawData);

private:
    std::string dataDirectory;
    std::string csvHeader;
    std::regex searchRegex;
    std::string replaceStr;

    // per-write policy; no persistent streams
    std::string logNamePattern; // pattern for logfile name (without extension)
    std::map<std::string,std::string> nameMap;

    std::mutex logMutex;

    std::string extractDeviceId(const std::string& topic);
    std::string getFilePath(const std::string& deviceId);
    bool ensureDirectoryExists(const std::string& path);
    std::string processData(const std::string& rawData);
};

#endif // LOGGER_H