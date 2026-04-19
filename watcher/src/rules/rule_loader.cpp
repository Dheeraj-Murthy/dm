#include "watcher/rules/rule_loader.hpp"
#include <cstring>
#include <iostream>

using namespace std;

namespace watcher {

Comparison RuleLoader::parse_comparison(const char *value) {
    if (strcmp(value, "gt") == 0)
        return Comparison::GT;
    if (strcmp(value, "gte") == 0)
        return Comparison::GTE;
    if (strcmp(value, "lt") == 0)
        return Comparison::LT;
    if (strcmp(value, "lte") == 0)
        return Comparison::LTE;
    if (strcmp(value, "eq") == 0)
        return Comparison::EQ;
    return Comparison::GT;
}

JunctionEventType RuleLoader::parse_junction_event_type(const char *value) {
    if (strcmp(value, "phase-change") == 0)
        return JunctionEventType::PHASE_CHANGE;
    if (strcmp(value, "queue-overflow") == 0)
        return JunctionEventType::QUEUE_OVERFLOW;
    if (strcmp(value, "emergency") == 0)
        return JunctionEventType::EMERGENCY;
    if (strcmp(value, "manual-override") == 0)
        return JunctionEventType::MANUAL_OVERRIDE;
    return JunctionEventType::PHASE_CHANGE;
}

CorridorEventType RuleLoader::parse_corridor_event_type(const char *value) {
    if (strcmp(value, "congestion-detected") == 0)
        return CorridorEventType::CONGESTION_DETECTED;
    if (strcmp(value, "clear") == 0)
        return CorridorEventType::CLEAR;
    if (strcmp(value, "emergency-vehicles") == 0)
        return CorridorEventType::EMERGENCY_VEHICLES;
    return CorridorEventType::CONGESTION_DETECTED;
}

SignalMode RuleLoader::parse_signal_mode(const char *value) {
    if (strcmp(value, "DYNAMIC") == 0)
        return SignalMode::DYNAMIC;
    if (strcmp(value, "PERIODIC") == 0)
        return SignalMode::PERIODIC;
    if (strcmp(value, "MANUAL") == 0)
        return SignalMode::MANUAL;
    if (strcmp(value, "ECA_CONTROLLED") == 0)
        return SignalMode::ECA_CONTROLLED;
    return SignalMode::DYNAMIC;
}

LogicalOp RuleLoader::parse_logical_op(const char *value) {
    if (strcmp(value, "and") == 0)
        return LogicalOp::AND;
    if (strcmp(value, "or") == 0)
        return LogicalOp::OR;
    return LogicalOp::AND;
}

bool RuleLoader::load_from_file(const string &path) {
    pugi::xml_document doc;
    auto result = doc.load_file(path.c_str());

    if (!result) {
        cerr << "Failed to load XML: " << result.description() << endl;
        return false;
    }

    parse_eca_rules(doc.document_element());
    return true;
}

bool RuleLoader::load_from_string(const string &xml_content) {
    pugi::xml_document doc;
    auto result = doc.load_string(xml_content.c_str());

    if (!result) {
        cerr << "Failed to parse XML: " << result.description() << endl;
        return false;
    }

    parse_eca_rules(doc.document_element());
    return true;
}

void RuleLoader::parse_eca_rules(const pugi::xml_node &root) {
    if (strcmp(root.name(), "eca-rules") != 0) {
        cerr << "Expected <eca-rules> root element, got: " << root.name() << endl;
        return;
    }

    version_ = root.attribute("version").as_string("1.0");
    global_enabled_ = root.attribute("enabled").as_bool(true);

    for (auto rule_node : root.children("rule")) {
        auto rule = parse_rule(rule_node);
        if (!rule.id.empty()) {
            rules_.push_back(::std::move(rule));
        }
    }
}

EcaRule RuleLoader::parse_rule(const pugi::xml_node &rule_node) {
    EcaRule rule;
    rule.id = rule_node.attribute("id").as_string();
    rule.priority = rule_node.attribute("priority").as_int(0);
    rule.enabled = rule_node.attribute("enabled").as_bool(true);

    auto name_node = rule_node.child("name");
    if (name_node)
        rule.name = name_node.child_value();

    auto desc_node = rule_node.child("description");
    if (desc_node)
        rule.description = desc_node.child_value();

    auto event_node = rule_node.child("event");
    if (event_node)
        rule.event_type = parse_event_type(event_node);

    auto cond_node = rule_node.child("condition");
    if (cond_node)
        rule.condition = parse_condition(cond_node);

    for (auto action_node : rule_node.children("action")) {
        rule.actions.push_back(parse_action(action_node));
    }

    return rule;
}

EventType RuleLoader::parse_event_type(const pugi::xml_node &event_node) {
    if (event_node.child("sensor-update"))
        return EventType::SENSOR_UPDATE;
    if (event_node.child("junction-event"))
        return EventType::JUNCTION_EVENT;
    if (event_node.child("time-schedule"))
        return EventType::TIME_SCHEDULE;
    if (event_node.child("corridor-event"))
        return EventType::CORRIDOR_EVENT;
    return EventType::UNKNOWN;
}

optional<Condition> RuleLoader::parse_condition(const pugi::xml_node &cond_node) {
    Condition cond;

    auto queue_node = cond_node.child("queue-threshold");
    if (queue_node) {
        QueueThreshold qt;
        qt.approach_id = queue_node.attribute("approach-id").as_int(-1);
        qt.junction_id = queue_node.attribute("junction-id").as_int(-1);
        qt.threshold = queue_node.attribute("threshold").as_int(0);
        qt.comparison = parse_comparison(queue_node.attribute("comparison").as_string("gt"));
        cond.queue_threshold = qt;
    }

    auto time_node = cond_node.child("time-range");
    if (time_node) {
        TimeRange tr;
        tr.start = time_node.attribute("start").as_string();
        tr.end = time_node.attribute("end").as_string();
        tr.days = time_node.attribute("days").as_string();
        cond.time_range = tr;
    }

    auto mode_node = cond_node.child("junction-mode");
    if (mode_node) {
        JunctionMode jm;
        jm.junction_id = mode_node.attribute("junction-id").as_int(-1);
        jm.mode = parse_signal_mode(mode_node.attribute("mode").as_string("DYNAMIC"));
        cond.junction_mode = jm;
    }

    auto compound_node = cond_node.child("compound");
    if (compound_node) {
        CompoundCondition cc;
        cc.op = parse_logical_op(compound_node.attribute("operator").as_string("and"));
        for (auto child_cond : compound_node.children("condition")) {
            if (auto child = parse_condition(child_cond)) {
                cc.conditions.push_back(*child);
            }
        }
        cond.compound = cc;
    }

    return cond;
}

Action RuleLoader::parse_action(const pugi::xml_node &action_node) {
    Action action;

    auto phase_timing = action_node.child("set-phase-timing");
    if (phase_timing) {
        SetPhaseTimingAction a;
        a.phase_id = phase_timing.attribute("phase-id").as_int(-1);
        a.green_time = phase_timing.attribute("green-time").as_int(0);
        a.yellow_time = phase_timing.attribute("yellow-time").as_int(0);
        a.red_time = phase_timing.attribute("red-time").as_int(0);
        a.is_active = phase_timing.attribute("is-active").as_bool(false);
        action.set_phase_timing = a;
    }

    auto active_phase = action_node.child("set-active-phase");
    if (active_phase) {
        SetActivePhaseAction a;
        a.approach_id = active_phase.attribute("approach-id").as_int(-1);
        a.green_time = active_phase.attribute("green-time").as_int(30);
        a.junction_id = active_phase.attribute("junction-id").as_int(-1);
        action.set_active_phase = a;
    }

    auto corridor_free = action_node.child("enable-corridor-free");
    if (corridor_free) {
        EnableCorridorFreeAction a;
        a.corridor_id = corridor_free.attribute("corridor-id").as_int(-1);
        a.green_time = corridor_free.attribute("green-time").as_int(60);
        a.exclude_junctions = corridor_free.attribute("exclude-junctions").as_string();
        action.enable_corridor_free = a;
    }

    auto reset = action_node.child("reset-to-dynamic");
    if (reset) {
        ResetToDynamicAction a;
        a.junction_id = reset.attribute("junction-id").as_int(-1);
        a.corridor_id = reset.attribute("corridor-id").as_int(-1);
        action.reset_to_dynamic = a;
    }

    auto set_mode = action_node.child("set-mode");
    if (set_mode) {
        SetModeAction a;
        a.junction_id = set_mode.attribute("junction-id").as_int(-1);
        a.mode = parse_signal_mode(set_mode.attribute("mode").as_string("DYNAMIC"));
        action.set_mode = a;
    }

    auto adjust = action_node.child("adjust-green-time");
    if (adjust) {
        AdjustGreenTimeAction a;
        a.approach_id = adjust.attribute("approach-id").as_int(-1);
        a.adjustment_seconds = adjust.attribute("adjustment-seconds").as_int(0);
        a.min_green = adjust.attribute("min-green").as_int(10);
        a.max_green = adjust.attribute("max-green").as_int(120);
        action.adjust_green_time = a;
    }

    return action;
}

vector<const EcaRule *> RuleLoader::find_matching_rules(EventType type) const {
    vector<const EcaRule *> matches;
    for (const auto &rule : rules_) {
        if (rule.enabled && rule.event_type == type) {
            matches.push_back(&rule);
        }
    }
    return matches;
}

} // namespace watcher
