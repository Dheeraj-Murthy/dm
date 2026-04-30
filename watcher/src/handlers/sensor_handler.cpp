#include "watcher/handlers/sensor_handler.hpp"

namespace watcher {

void SensorHandler::handle(const std::string &channel, const json &data, const RuleLoader &rules, Connection &conn) {
    (void)channel;
    std::cout << "Processing sensor update...\n";
    std::cout << "  Data: " << data.dump(2) << "\n";

    auto matches = rules.find_matching_rules(EventType::SENSOR_UPDATE);
    std::cout << "  Matching rules: " << matches.size() << "\n";
    for (auto rule : matches) {
        std::cout << "    Rule: " << rule->name << " (priority " << rule->priority << ")\n";
        for (const auto &action : rule->actions) {
            if (action.set_phase_timing) {
                std::cout << "      ACTION: Set phase " << action.set_phase_timing->phase_id
                          << " green=" << action.set_phase_timing->green_time << "\n";
                std::string sql =
                    "UPDATE phases SET green_time=" + std::to_string(action.set_phase_timing->green_time) +
                    " WHERE phase_id=" + std::to_string(action.set_phase_timing->phase_id);
                conn.execute(sql);
            }
            if (action.set_active_phase) {
                std::cout << "      ACTION: Set active phase approach=" << action.set_active_phase->approach_id
                          << " green=" << action.set_active_phase->green_time << "\n";
                std::string sql =
                    "UPDATE approaches SET green_time=" + std::to_string(action.set_active_phase->green_time) +
                    " WHERE approach_id=" + std::to_string(action.set_active_phase->approach_id);
                conn.execute(sql);
            }
            if (action.enable_corridor_free) {
                std::cout << "      ACTION: Enable corridor " << action.enable_corridor_free->corridor_id
                          << " green=" << action.enable_corridor_free->green_time << "\n";
                std::string sql =
                    "UPDATE corridors SET green_time=" + std::to_string(action.enable_corridor_free->green_time) +
                    ", mode='CORRIDOR_FREE' WHERE corridor_id=" +
                    std::to_string(action.enable_corridor_free->corridor_id);
                conn.execute(sql);
            }
            if (action.reset_to_dynamic) {
                std::cout << "      ACTION: Reset to dynamic\n";
                if (action.reset_to_dynamic->junction_id != -1) {
                    std::string sql = "UPDATE junctions SET mode='DYNAMIC' WHERE junction_id=" +
                                      std::to_string(action.reset_to_dynamic->junction_id);
                    conn.execute(sql);
                }
            }
            if (action.set_mode) {
                std::cout << "      ACTION: Set mode " << static_cast<int>(action.set_mode->mode) << "\n";
                std::string mode_str = action.set_mode->mode == SignalMode::DYNAMIC    ? "DYNAMIC"
                                       : action.set_mode->mode == SignalMode::PERIODIC ? "PERIODIC"
                                       : action.set_mode->mode == SignalMode::MANUAL   ? "MANUAL"
                                                                                       : "ECA_CONTROLLED";
                std::string sql = "UPDATE junctions SET mode='" + mode_str +
                                  "' WHERE junction_id=" + std::to_string(action.set_mode->junction_id);
                conn.execute(sql);
            }
            if (action.adjust_green_time) {
                std::cout << "      ACTION: Adjust green time " << action.adjust_green_time->adjustment_seconds
                          << "s (min=" << action.adjust_green_time->min_green
                          << " max=" << action.adjust_green_time->max_green << ")\n";
                std::string sql = "UPDATE approaches SET green_time=green_time+" +
                                  std::to_string(action.adjust_green_time->adjustment_seconds) +
                                  " WHERE approach_id=" + std::to_string(action.adjust_green_time->approach_id);
                conn.execute(sql);
            }
        }
    }
}

} // namespace watcher
