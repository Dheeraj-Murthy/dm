#pragma once

#include <string>
#include <vector>

namespace watcher {

struct Config {
    std::string dbname;
    std::string user;
    std::string password;
    std::string host;
    int port;
    std::vector<std::string> channels;
    std::string rules_path;

    static Config fromEnv();
    static Config fromFile(const std::string &path);

    std::string connectionString() const;
};

} // namespace watcher
