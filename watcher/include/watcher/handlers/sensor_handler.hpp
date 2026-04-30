#pragma once

#include "watcher/dispatcher.hpp"
#include "watcher/rules/rule_loader.hpp"
#include <iostream>

namespace watcher {

class SensorHandler : public EventHandler {
public:
    std::string name() const override { return "SensorHandler"; }
    void handle(const std::string& channel, const json& data,
                const RuleLoader& rules, Connection& conn) override;
};

}