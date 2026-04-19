#pragma once

#include "watcher/dispatcher.hpp"
#include <iostream>

namespace watcher {

class SignalHandler : public EventHandler {
public:
    std::string name() const override { return "SignalHandler"; }
    
    void handle(const std::string& channel, const json& data) override {
        (void)channel;
        (void)data;
        std::cout << "Processing signal update...\n";
        std::cout << "  Data: " << data.dump(2) << "\n";
    }
};

}