#include "replace_macros.h"
#include <iostream>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <map>

#ifndef REPLACE_MACROS_DEBUG
#define REPLACE_MACROS_DEBUG 0
#endif

#ifndef LOGGER_DEBUG
#define LOGGER_DEBUG 0
#endif

#if REPLACE_MACROS_DEBUG
#define RDBG(x) (std::cerr << x)
#else
#define RDBG(x) ((void)0)
#endif

// helper
static std::string to_lower_copy(const std::string &s) {
    std::string r = s; std::transform(r.begin(), r.end(), r.begin(), ::tolower); return r;
}

static std::string two_digits(int v) {
    std::ostringstream o; if (v<10) o<<'0'; o<<v; return o.str();
}

static std::string formatDateTimeFromTimeT(const std::string &fmt, std::time_t t) {
    std::tm tm{};
#if defined(_WIN32)
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif

    static const char* monthsShort[] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
    static const char* monthsLong[]  = {"January","February","March","April","May","June","July","August","September","October","November","December"};
    static const char* daysShort[] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
    static const char* daysLong[]  = {"Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"};

    std::string out;
    out.reserve(fmt.size()*2);

    for (size_t i=0;i<fmt.size();) {
        if (fmt[i] == '\'') { // quoted literal
            ++i;
            while (i < fmt.size() && fmt[i] != '\'') out.push_back(fmt[i++]);
            if (i < fmt.size() && fmt[i] == '\'') ++i;
            continue;
        }

        // 4-char tokens
        if (i+4 <= fmt.size()) {
            std::string tk = fmt.substr(i,4);
            if (tk=="yyyy") { out += std::to_string(1900 + tm.tm_year); i+=4; continue; }
            if (tk=="MMMM") { out += monthsLong[tm.tm_mon]; i+=4; continue; }
            if (tk=="dddd") { out += daysLong[tm.tm_wday]; i+=4; continue; }
        }
        // 3-char tokens
        if (i+3 <= fmt.size()) {
            std::string tk = fmt.substr(i,3);
            if (tk=="MMM") { out += monthsShort[tm.tm_mon]; i+=3; continue; }
            if (tk=="ddd") { out += daysShort[tm.tm_wday]; i+=3; continue; }
        }
        // 2-char tokens
        if (i+2 <= fmt.size()) {
            std::string tk = fmt.substr(i,2);
            // week number (ISO week, 01..53)
            if (tk == "ww") {
                // compute ISO week number robustly (portable)
                std::tm th = tm;
                int iso_wday = (tm.tm_wday == 0) ? 7 : tm.tm_wday; // Monday=1..Sunday=7
                th.tm_mday += (4 - iso_wday); // move to Thursday of current week
                std::mktime(&th); // normalize date (fills tm_yday etc.)
                int week = (th.tm_yday / 7) + 1;
                out += two_digits(week);
                i += 2;
                continue;
            }
            if (tk=="yy") { int y=(1900+tm.tm_year)%100; out += (y<10? "0"+std::to_string(y):std::to_string(y)); i+=2; continue; }
            if (tk=="MM") { out += two_digits(tm.tm_mon+1); i+=2; continue; }
            if (tk=="dd") { out += two_digits(tm.tm_mday); i+=2; continue; }
            if (tk=="HH") { out += two_digits(tm.tm_hour); i+=2; continue; }
            if (tk=="hh") { int h = tm.tm_hour%12; if (h==0) h=12; out += two_digits(h); i+=2; continue; }
            if (tk=="mm") { out += two_digits(tm.tm_min); i+=2; continue; }
            if (tk=="ss") { out += two_digits(tm.tm_sec); i+=2; continue; }
            if (tk=="AM" || tk=="PM") { out += (tm.tm_hour>=12? "PM":"AM"); i+=2; continue; }
            if (tk=="am" || tk=="pm") { out += (tm.tm_hour>=12? "pm":"am"); i+=2; continue; }
        }
        // single-char tokens
        char c = fmt[i];
        if (c=='M') { out += std::to_string(tm.tm_mon+1); ++i; continue; }
        if (c=='d') { out += std::to_string(tm.tm_mday); ++i; continue; }
        if (c=='H') { out += std::to_string(tm.tm_hour); ++i; continue; }
        if (c=='h') { int hh = tm.tm_hour%12; if (hh==0) hh=12; out += std::to_string(hh); ++i; continue; }
        if (c=='m') { out += std::to_string(tm.tm_min); ++i; continue; }
        if (c=='s') { out += std::to_string(tm.tm_sec); ++i; continue; }
        if (c=='z') { out += "000"; ++i; continue; } // milliseconds not available -> 000
        out.push_back(fmt[i++]);
    }

    return out;
}

static bool getValueByHeaderOrIndex(const std::vector<std::string>& headerNames,
                                    const std::smatch& matches,
                                    const std::string& key,
                                    std::string& outValue)
{
  if (key.empty()) return false;
  bool allDigits = std::all_of(key.begin(), key.end(), ::isdigit);
  if (allDigits) {
    int idx = std::stoi(key);
    if (idx >= 0 && idx < (int)matches.size()) { outValue = matches[idx].str(); return true; }
    // If numeric but not a valid capture index, treat as literal value (timestamp)
    outValue = key;
    return true;
  }
  // find header
  std::string keyl = to_lower_copy(key);
  for (size_t i=0;i<headerNames.size();++i) {
    if (to_lower_copy(headerNames[i]) == keyl) {
      if ((int)i < (int)matches.size()) { outValue = matches[i].str(); return true; }
      return false;
    }
  }
  return false;
}

std::string expandReplace(const std::string& templ,
                          const std::string& deviceId,
                          const std::vector<std::string>& headerNames,
                          const std::smatch& matches,
                          const std::map<std::string,std::string>& nameMap)
{
    std::string s = templ;
    std::smatch m;

    RDBG("DBG expandReplace() initial template=[" << templ << "]\n");
    RDBG("DBG deviceId=[" << deviceId << "]\n");
    RDBG("DBG headerNames=[");
    for (size_t i=0;i<headerNames.size();++i) {
        if (i) RDBG("," );
        RDBG(headerNames[i]);
    }
    RDBG("]\n");
    RDBG("DBG matches count=" << matches.size() << "\n");
    for (size_t i=0;i<matches.size();++i) {
        RDBG("  DBG match[" << i << "] = [" << matches[i].str() << "]\n");
    }

    // first: $TIME('fmt') or $TIME{'fmt'}  -> current time formatted
    std::regex time_re_paren(R"(\$TIME\(\s*'([^']*)'\s*\))");
    std::regex time_re_brace(R"(\$TIME\{\s*'([^']*)'\s*\})");
    while (std::regex_search(s, m, time_re_paren)) {
        std::string fmt = m.str(1);
        std::string repl = formatDateTimeFromTimeT(fmt, std::time(nullptr));
        RDBG("DBG TIME replacing [" << m.str(0) << "] with [" << repl << "]\n");
        s.replace(m.position(0), m.length(0), repl);
    }
    while (std::regex_search(s, m, time_re_brace)) {
        std::string fmt = m.str(1);
        std::string repl = formatDateTimeFromTimeT(fmt, std::time(nullptr));
        RDBG("DBG TIME replacing [" << m.str(0) << "] with [" << repl << "]\n");
        s.replace(m.position(0), m.length(0), repl);
    }

    // 1) Process $DateTime(...) occurrences first, but expand any backrefs inside its key
    std::regex dt_re(R"(\$DateTime\(\s*'([^']*)'\s*,\s*([^\)]*?)\s*\))");
    std::regex inner_backref_re(R"(\\([0-9]+)|\$(\d+))");
    while (std::regex_search(s, m, dt_re)) {
        std::string full = m.str(0);
        std::string fmt = m.str(1);
        std::string key = m.str(2);

        // trim
        auto trim = [](const std::string &x)->std::string {
            size_t a = x.find_first_not_of(" \t\r\n");
            if (a==std::string::npos) return std::string();
            size_t b = x.find_last_not_of(" \t\r\n");
            return x.substr(a, b-a+1);
        };
        key = trim(key);
        if (key.size() >= 2 && ((key.front()=='\'' && key.back()=='\'') || (key.front()=='"' && key.back()=='"')))
            key = key.substr(1, key.size()-2);

        RDBG("DBG DateTime found: full=[" << full << "], fmt=[" << fmt << "], rawKey=[" << m.str(2) << "], trimmedKey=[" << key << "]\n");

        // replace backrefs inside key (e.g. "\1" or "$1") with actual capture values
        std::string expandedKey;
        size_t last = 0;
        for (std::sregex_iterator it(key.begin(), key.end(), inner_backref_re), end; it != end; ++it) {
            std::smatch km = *it;
            size_t pos = km.position(0);
            expandedKey.append(key.substr(last, pos - last));
            std::string numStr = km[1].matched ? km.str(1) : km.str(2);
            int idx = -1;
            try { idx = std::stoi(numStr); }
            catch(...) { idx = -1; }
            if (idx >= 0 && idx < (int)matches.size()) {
                RDBG("DBG inner backref: \\" << numStr << " -> [" << matches[idx].str() << "]\n");
                expandedKey.append(matches[idx].str());
            } else {
                RDBG("DBG inner backref: literal num [" << numStr << "] kept\n");
                expandedKey.append(numStr); // keep numeric literal
            }
            last = pos + km.length(0);
        }
        expandedKey.append(key.substr(last));
        key = trim(expandedKey);
        if (!key.empty() && (key[0]=='\\' || key[0]=='$')) key = key.substr(1);

        RDBG("DBG DateTime resolved key=[" << key << "]\n");

        // resolve key -> value (index/header or literal)
        std::string val;
        bool ok = getValueByHeaderOrIndex(headerNames, matches, key, val);
        std::time_t ts = 0;
        if (ok) {
            RDBG("DBG getValueByHeaderOrIndex returned [" << val << "] for key [" << key << "]\n");
            bool allDigits = !val.empty() && std::all_of(val.begin(), val.end(), ::isdigit);
            if (allDigits) {
                try {
                    long long v = std::stoll(val);
                    if (v > 100000000000LL) v /= 1000; // ms -> s heuristics
                    ts = (std::time_t)v;
                } catch(...) { ts = 0; }
            }
        } else {
            std::string ktrim = key;
            if (!ktrim.empty() && std::all_of(ktrim.begin(), ktrim.end(), ::isdigit)) {
                try {
                    long long v = std::stoll(ktrim);
                    if (v > 100000000000LL) v /= 1000;
                    ts = (std::time_t)v;
                    RDBG("DBG key is numeric literal, using ts=" << ts << "\n");
                } catch(...) { ts = 0; }
            } else {
                RDBG("DBG nothing resolved for key [" << key << "], ts will be 0\n");
            }
        }

        std::string repl = formatDateTimeFromTimeT(fmt, ts);
        RDBG("DBG DateTime replacing [" << full << "] with [" << repl << "]\n");
        s.replace(m.position(0), m.length(0), repl);
    }

    RDBG("DBG after DateTime pass: [" << s << "]\n");

    // 2) Replace remaining backreferences in the template (\1 and $1 ...)
    std::regex backref_re(R"(\\([0-9]+))");
    while (std::regex_search(s, m, backref_re)) {
        int idx = std::stoi(m.str(1));
        std::string repl = (idx >= 0 && idx < (int)matches.size()) ? matches[idx].str() : std::string();
        RDBG("DBG replacing backref \\" << idx << " with [" << repl << "]\n");
        s.replace(m.position(0), m.length(0), repl);
    }
    std::regex dollar_re(R"(\$([0-9]+))");
    while (std::regex_search(s, m, dollar_re)) {
        int idx = std::stoi(m.str(1));
        std::string repl = (idx >= 0 && idx < (int)matches.size()) ? matches[idx].str() : std::string();
        RDBG("DBG replacing $"<< idx << " with [" << repl << "]\n");
        s.replace(m.position(0), m.length(0), repl);
    }

    RDBG("DBG after backrefs pass: [" << s << "]\n");

    // 3) Replace $DEV
    {
        std::string token = "$DEV";
        size_t pos = 0;
        while ((pos = s.find(token, pos)) != std::string::npos) {
            RDBG("DBG replacing $DEV at pos " << pos << " with [" << deviceId << "]\n");
            s.replace(pos, token.size(), deviceId);
            pos += deviceId.size();
        }
    }

    // 4) Replace $NAME -> lookup in nameMap, fallback to deviceId
    {
        std::string token = "$NAME";
        size_t pos = 0;
        std::string replVal;
        auto it = nameMap.find(deviceId);
        if (it != nameMap.end() && !it->second.empty()) replVal = it->second;
        else replVal = deviceId;
        while ((pos = s.find(token, pos)) != std::string::npos) {
            RDBG("DBG replacing $NAME at pos " << pos << " with [" << replVal << "]\n");
            s.replace(pos, token.size(), replVal);
            pos += replVal.size();
        }
    }

    RDBG("DBG final expanded = [" << s << "]\n");
    return s;
}