#include "watcher/handlers/signal_handler.hpp"

namespace watcher {

void SignalHandler::handle(const std::string& channel, const json& data,
                           const RuleLoader& rules) {
    (void)channel;
    std::cout << "Processing signal update...\n";
    std::cout << "  Data: " << data.dump(2) << "\n";
    std::cout << "  Loaded rules: " << rules.rules().size() << "\n";
}

}