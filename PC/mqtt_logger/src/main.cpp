#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <csignal>
#include <mqtt/async_client.h>
#include "config.h"
#include "logger.h"
#include "replace_macros.h"
#include <sstream>
#include <functional>


class MqttCallback : public virtual mqtt::callback,
                     public virtual mqtt::iaction_listener {
private:
    DataLogger& logger;
    std::string topicPattern;
    mqtt::async_client& client;
    
public:
    MqttCallback(DataLogger& log, const std::string& pattern, mqtt::async_client& cli)
        : logger(log), topicPattern(pattern), client(cli) {}
    
    void connection_lost(const std::string& cause) override {
        std::cout << "Connection lost";
        if (!cause.empty()) {
            std::cout << ": " << cause;
        }
        std::cout << std::endl;
    } 
    
    void message_arrived(mqtt::const_message_ptr msg) override {
        std::string topic = msg->get_topic();
        std::string payload = msg->to_string();
        
        std::cout << "Message received:" << std::endl;
        std::cout << "  Topic: " << topic << std::endl;
        std::cout << "  Payload: " << payload << std::endl;
        
        // Extract device ID from topic
        std::string deviceId = extractDeviceId(topic);
        if (deviceId != "unknown") {
            logger.logData(deviceId, payload);
        } else {
            std::cout << "Failed to extract device ID from topic: " << topic << std::endl;
        }
    }
    
    void delivery_complete(mqtt::delivery_token_ptr token) override {}
    
    void on_failure(const mqtt::token& token) override {
        std::cout << "Operation failed";
        if (token.get_message_id() != 0) {
            std::cout << " for token: " << token.get_message_id();
        }
        std::cout << std::endl;
    }
    
    void on_success(const mqtt::token& token) override {
        std::cout << "Operation successful";
        if (token.get_message_id() != 0) {
            std::cout << " for token: " << token.get_message_id();
        }
        auto topics = token.get_topics();
        if (topics && !topics->empty()) {
            std::cout << " for topics: ";
            for (const auto& topic : *topics) {
                std::cout << topic << " ";
            }
        }
        std::cout << std::endl;
    }

private:
    std::string extractDeviceId(const std::string& topic) {
        // Expect format: ORLOV/HUMT/HT_3C71BF29A68E/HUMT
        size_t lastSlash = topic.find_last_of('/');
        if (lastSlash == std::string::npos) {
            return "unknown";
        }
        
        size_t secondLastSlash = topic.find_last_of('/', lastSlash - 1);
        if (secondLastSlash == std::string::npos) {
            return "unknown";
        }
        
        return topic.substr(secondLastSlash + 1, lastSlash - secondLastSlash - 1);
    }
};

// helper: split/trim
static std::vector<std::string> split_and_trim(const std::string &s, char delim=';') {
    std::vector<std::string> out;
    std::string cur;
    for (char c : s) {
        if (c==delim) {
            size_t a = cur.find_first_not_of(" \t\r\n");
            size_t b = cur.find_last_not_of(" \t\r\n");
            if (a!=std::string::npos && b!=std::string::npos) out.push_back(cur.substr(a, b-a+1));
            else if (!cur.empty()) out.push_back("");
            cur.clear();
        } else cur.push_back(c);
    }
    if (!cur.empty()) {
        size_t a = cur.find_first_not_of(" \t\r\n");
        size_t b = cur.find_last_not_of(" \t\r\n");
        if (a!=std::string::npos && b!=std::string::npos) out.push_back(cur.substr(a, b-a+1));
        else out.push_back("");
    }
    return out;
}

// extract device id from topic (looks for segment starting with HT_)
static std::string extractDeviceFromTopic(const std::string &topic) {
    std::string cur;
    std::vector<std::string> parts;
    for (char c: topic) {
        if (c=='/') { parts.push_back(cur); cur.clear(); } else cur.push_back(c);
    }
    if (!cur.empty()) parts.push_back(cur);
    for (auto &p: parts) if (p.rfind("HT_",0) == 0) return p;
    if (parts.size() >= 3) return parts[2];
    return std::string();
}

// Example message handler to use instead of your current one.
// Adjust cfg.get(...) / logger.log(...) calls to your actual config/logger API.
void handleMessage(const std::string &topic, const std::string &payload,
                   const std::string &headerLine,
                   const std::string &searchPattern,
                   const std::string &replaceTemplate,
                   DataLogger &logger)
{
    std::smatch matches;
    try {
        std::regex re(searchPattern);
        if (!std::regex_search(payload, matches, re)) return;

        // Диагностика перед логированием (вместо/рядом с текущим вызовом expandReplace/logger.logData)
        {
            std::string deviceId = extractDeviceFromTopic(topic);
            std::vector<std::string> headerNames = split_and_trim(headerLine, ';');

            // diagnostic: print what we have
            std::cerr << "DBG replaceTemplate(raw) = [" << replaceTemplate << "]\n";
            std::cerr << "DBG deviceId = [" << deviceId << "]\n";
            if (matches.size() > 1) std::cerr << "DBG match1 = [" << matches[1].str() << "]\n";
            else std::cerr << "DBG no capture groups\n";

            // expand and print result
            std::string expanded = expandReplace(replaceTemplate, deviceId, headerNames, matches);
            std::cerr << "DBG expanded = [" << expanded << "]\n";

            // use DataLogger API
            logger.logData(deviceId, expanded);
        }
    } catch (const std::exception &e) {
        std::cerr << "Regex/processing error: " << e.what() << "\n";
    }
}

volatile std::sig_atomic_t running = 1;

void signalHandler(int signal) {
    std::cout << "\nTermination signal received (" << signal << "). Shutting down..." << std::endl;
    running = 0;
}

int main() {
    // Install signal handlers
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
    
    std::cout << "=== MQTT Logger v1.0 ===" << std::endl;
    
    // Load configuration
    Config config;
    std::string configPath = Config::getConfigPath();

    if (!config.loadFromFile(configPath)) {
        // loadFromFile already prints clear diagnostic messages (missing/malformed keys, create dir errors, etc.)
        return 1;
    }
    //
    std::cout << "Configuration loaded from: " << configPath << std::endl;
    std::cout << "MQTT broker: " << config.getMqttUrl() << std::endl;
    std::cout << "Base topic: " << config.getTopicBase() << std::endl;
    
    // Create data logger
    std::string dataDir = Config::getDataDirectory();
    DataLogger logger(dataDir, config.getHeader(), 
                     config.getSearchPattern(), config.getReplacePattern());
    applyConfigToLogger(configPath, logger); // use same config file that was loaded
    
    std::cout << "Data directory: " << dataDir << std::endl;
    
    // Configure MQTT client
    std::string serverURI = "tcp://" + config.getMqttUrl();
    std::string clientId = "mqtt_logger_" + std::to_string(std::time(nullptr));
    
    try {
        mqtt::async_client client(serverURI, clientId);
        
        // Set callback
        MqttCallback callback(logger, config.getTopicBase(), client);
        client.set_callback(callback);
        
        // Connection options
        mqtt::connect_options connOpts;
        connOpts.set_user_name(config.getMqttUser());
        connOpts.set_password(config.getMqttPassword());
        connOpts.set_clean_session(true);
        connOpts.set_automatic_reconnect(true);
        connOpts.set_keep_alive_interval(60);
        
        std::cout << "Connecting to MQTT broker..." << std::endl;
        
        // Connect
        mqtt::token_ptr connTok = client.connect(connOpts, nullptr, callback);
        connTok->wait();
        
        std::cout << "Connected to MQTT broker!" << std::endl;
        
        // Subscribe to topics
        std::string topicFilter = config.getTopicBase() + "/HUMT/+/HUMT";
        std::cout << "Subscribing to topics: " << topicFilter << std::endl;
        
        client.subscribe(topicFilter, 1, nullptr, callback)->wait();
        std::cout << "Subscription active!" << std::endl;
        
        std::cout << "Waiting for messages... (Ctrl+C to exit)" << std::endl;
        
        // Main loop
        while (running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
            if (!client.is_connected()) {
                std::cout << "Connection lost, reconnecting..." << std::endl;
                try {
                    connTok = client.connect(connOpts, nullptr, callback);
                    connTok->wait();
                    client.subscribe(topicFilter, 1, nullptr, callback)->wait();
                    std::cout << "Reconnected!" << std::endl;
                } catch (const mqtt::exception& exc) {
                    std::cerr << "Reconnect error: " << exc.what() << std::endl;
                    std::this_thread::sleep_for(std::chrono::seconds(5));
                }
            }
        }
        
        std::cout << "Disconnecting from MQTT broker..." << std::endl;
        client.disconnect()->wait();
        
    } catch (const mqtt::exception& exc) {
        std::cerr << "MQTT error: " << exc.what() << std::endl;
        return 1;
    } catch (const std::exception& exc) {
        std::cerr << "Error: " << exc.what() << std::endl;
        return 1;
    }
    
    std::cout << "Program exited." << std::endl;
    return 0;
}

// Example usage inside your message handler:
//
// processDataLine(topic, payload,
//                 "SNTP;TEMP;HUM",                              // Header from config.ini
//                 R"(SNTP: ([0-9]+), TEMP: ([0-9\.]+) C, HUM: ([0-9\.]+) %, DEW: ([0-9\.]+) C)",
//                 "$DEV;$DateTime('ddd, MMM dd  HH:mm:ss yyyy', \\1);\\2;\\3",
//                 [&](const std::string &line){ logger.log(line); });
//
// Note: in Replace use \\1 in C++ literal (or \1 in ini file); expandReplace handles both \1 and $1 forms.