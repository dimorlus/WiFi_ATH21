#pragma once
#include <string>
#include <vector>
#include <regex>
#include <map>

std::string expandReplace(const std::string& templ,
                          const std::string& deviceId,
                          const std::vector<std::string>& headerNames,
                          const std::smatch& matches,
                          const std::map<std::string,std::string>& nameMap = {});