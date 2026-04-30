#include "watcher/dispatcher.hpp"
#include <iostream>

namespace watcher {

void EventDispatcher::registerHandler(const std::string &channel, HandlerPtr handler) {
    handlers_[channel] = std::move(handler);
}

void EventDispatcher::registerFactory(const std::string &channel, HandlerFactory factory) {
    factories_[channel] = std::move(factory);
}

void EventDispatcher::dispatch(const std::string &channel, const std::string &payload) {
    auto it = handlers_.find(channel);
    if (it != handlers_.end()) {
        try {
            json data = json::parse(payload);
            it->second->handle(channel, data, *rules_, *conn_);
        } catch (const std::exception &e) {
            std::cerr << "Failed to parse JSON: " << e.what() << "\n";
        }
    }
}

} // namespace watcher
