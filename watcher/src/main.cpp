#include "watcher/config.hpp"
#include "watcher/connection.hpp"
#include "watcher/dispatcher.hpp"
#include "watcher/handlers/sensor_handler.hpp"
#include "watcher/handlers/signal_handler.hpp"
#include "watcher/listener.hpp"
#include "watcher/rules/rule_loader.hpp"
#include <atomic>
#include <csignal>
#include <cstdlib>
#include <iostream>

namespace {
std::atomic<bool> g_running{true};
}

void signalHandler(int) {
    g_running = false;
}

int main() {
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    try {
        auto config = watcher::Config::fromEnv();
        config.dbname = "traffic_db";
        config.user = "dheerajmurthy";
        config.password = "dheeraj";

        std::cout << "Connecting to " << config.dbname << "...\n";

        watcher::Connection conn(config.connectionString());
        std::cout << "Connected.\n";

        watcher::RuleLoader rule_loader;
        std::string rules_path;
        if (!config.rules_path.empty()) {
            rules_path = config.rules_path;
        } else {
            const char *cwd = std::getenv("WATCHER_ROOT");
            rules_path = cwd ? std::string(cwd) + "/sql/xml/eca-rules-example.xml"
                             : "/Users/dheerajmurthy/Downloads/IIITB/sem6/dm/project/sql/xml/eca-rules-example.xml";
        }
        if (rule_loader.load_from_file(rules_path)) {
            std::cout << "Loaded " << rule_loader.rules().size() << " rules (version " << rule_loader.version()
                      << ")\n";
            for (const auto &rule : rule_loader.rules()) {
                std::cout << "  - [" << rule.id << "] " << rule.name << " (priority " << rule.priority << ")\n";
            }
        } else {
            std::cerr << "Failed to load rules from " << rules_path << "\n";
        }

        watcher::EventDispatcher dispatcher;
        dispatcher.registerHandler("sensor_update_channel", std::make_unique<watcher::SensorHandler>());
        dispatcher.registerHandler("signal_update_channel", std::make_unique<watcher::SignalHandler>());

        conn.listen("sensor_update_channel");
        conn.listen("signal_update_channel");
        conn.listen("junction_event_channel");

        watcher::Listener listener(conn, dispatcher);
        listener.start();

        std::cout << "Listening for events... (Ctrl+C to exit)\n";

        while (g_running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        listener.stop();
        std::cout << "Stopped.\n";

    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
