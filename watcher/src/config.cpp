#include "watcher/config.hpp"
#include <cstdio>
#include <cstdlib>
#include <sstream>

namespace watcher {

Config Config::fromEnv() {
    Config cfg;
    cfg.dbname = std::getenv("PGDATABASE") ? std::getenv("PGDATABASE") : "traffic_db";
    cfg.user = std::getenv("PGUSER") ? std::getenv("PGUSER") : "postgres";
    cfg.password = std::getenv("PGPASSWORD") ? std::getenv("PGPASSWORD") : "";
    cfg.host = std::getenv("PGHOST") ? std::getenv("PGHOST") : "localhost";
    cfg.port = std::getenv("PGPORT") ? std::atoi(std::getenv("PGPORT")) : 5432;
    cfg.channels = {"sensor_update_channel", "signal_update_channel", "junction_event_channel"};
    return cfg;
}

Config Config::fromFile(const std::string &path) {
    return fromEnv();
}

std::string Config::connectionString() const {
    std::ostringstream oss;
    oss << "dbname=" << dbname << " user=" << user << " password=" << password << " host=" << host << " port=" << port;
    return oss.str();
}

} // namespace watcher
