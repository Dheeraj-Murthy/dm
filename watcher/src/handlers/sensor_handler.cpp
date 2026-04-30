#include "watcher/handlers/sensor_handler.hpp"

namespace watcher {

void SensorHandler::handle(const std::string& channel, const json& data,
                           const RuleLoader& rules) {
    (void)channel;
    std::cout << "Processing sensor update...\n";
    std::cout << "  Data: " << data.dump(2) << "\n";

    auto matches = rules.find_matching_rules(EventType::SENSOR_UPDATE);
    std::cout << "  Matching rules: " << matches.size() << "\n";
    for (auto rule : matches) {
        std::cout << "    Rule: " << rule->name << " (priority " << rule->priority << ")\n";
    }
}

}