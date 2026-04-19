#pragma once

#include "watcher/dispatcher.hpp"
#include <iostream>

namespace watcher {

class SensorHandler : public EventHandler {
public:
    std::string name() const override { return "SensorHandler"; }
    
    void handle(const std::string& channel, const json& data) override {
        (void)channel;
        std::cout << "Processing sensor update...\n";
        std::cout << "  Data: " << data.dump(2) << "\n";
    }
};

}