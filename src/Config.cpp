#include "Config.h"
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <algorithm>

static void trim(std::string& s)
{
    auto notSpace = [](char c) { return !std::isspace(static_cast<unsigned char>(c)); };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), notSpace));
    s.erase(std::find_if(s.rbegin(), s.rend(), notSpace).base(), s.end());
}

bool Config::load(const std::string& path)
{
    std::ifstream f(path);
    if (!f.is_open()) return false;

    std::string line;
    while (std::getline(f, line))
    {
        trim(line);
        if (line.empty() || line[0] == '#') continue;
        size_t eq = line.find('=');
        if (eq == std::string::npos) continue;

        std::string key = line.substr(0, eq);
        std::string val = line.substr(eq + 1);
        trim(key);
        trim(val);

        if (key == "ip") ip = val;
        else if (key == "port") port = static_cast<uint16_t>(std::stoi(val));
        else if (key == "io_threads") io_threads = std::stoi(val);
        else if (key == "work_threads") work_threads = std::stoi(val);
        else if (key == "conn_timeout") conn_timeout = std::stoi(val);
        else if (key == "timer_interval") timer_interval = std::stoi(val);
        else if (key == "stats_interval") stats_interval = std::stoi(val);
    }
    return true;
}

void Config::parseArgs(int argc, char* argv[])
{
    if (argc >= 3)
    {
        ip = argv[1];
        port = static_cast<uint16_t>(std::atoi(argv[2]));
        return;
    }
    if (argc >= 2)
        load(argv[1]);
}
