#pragma once
#include <string>
#include "logger.h"

class Config {
public:
    Config();
    bool loadFromFile(const std::string& filename);
    bool saveToFile(const std::string& filename);

    // config file location (next to exe)
    static std::string getConfigPath();

    // data directory getter (used as static in main)
    static std::string getDataDirectory();

    // simple accessors used by main.cpp
    std::string getMqttUrl() const;
    std::string getMqttUser() const;
    std::string getMqttPassword() const;
    std::string getTopicBase() const;
    std::string getHeader() const;
    std::string getSearchPattern() const;
    std::string getReplacePattern() const;
    std::string getLogFolder() const;

    // public config values (kept for direct access if needed)
    std::string mqttUrl;
    std::string mqttUser;
    std::string mqttPassword;
    std::string topicBase;
    std::string header;
    std::string searchPattern;
    std::string replacePattern;

private:
    // initialize defaults (declared so config.cpp's definition matches)
    void setDefaults();

    // storage for LOG:Folder
    std::string logFolder;
};

// Apply config (read NAME/LOG.Name) and pass to DataLogger
void applyConfigToLogger(const std::string &cfgPath, DataLogger &logger);