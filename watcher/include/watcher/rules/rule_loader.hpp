#pragma once

#define PUGIXML_HEADER_ONLY
#include <optional>
#include <pugixml.hpp>
#include <string>
#include <vector>

namespace watcher {

enum class EventType { SENSOR_UPDATE, JUNCTION_EVENT, TIME_SCHEDULE, CORRIDOR_EVENT, UNKNOWN };

enum class Comparison { GT, GTE, LT, LTE, EQ };

enum class SignalMode { DYNAMIC, PERIODIC, MANUAL, ECA_CONTROLLED };

enum class JunctionEventType { PHASE_CHANGE, QUEUE_OVERFLOW, EMERGENCY, MANUAL_OVERRIDE };

enum class CorridorEventType { CONGESTION_DETECTED, CLEAR, EMERGENCY_VEHICLES };

enum class LogicalOp { AND, OR };

struct SensorEvent {
    int sensor_id = -1;
    int approach_id = -1;
    int junction_id = -1;
    int vehicle_count_min = -1;
    int vehicle_count_max = -1;
};

struct JunctionEvent {
    int junction_id = -1;
    JunctionEventType event_type = JunctionEventType::PHASE_CHANGE;
};

struct ScheduleEvent {
    std::string cron;
    int interval_seconds = -1;
    std::string time_range_start;
    std::string time_range_end;
};

struct CorridorEvent {
    int corridor_id = -1;
    CorridorEventType event_type = CorridorEventType::CONGESTION_DETECTED;
};

struct QueueThreshold {
    int approach_id = -1;
    int junction_id = -1;
    int threshold = -1;
    Comparison comparison = Comparison::GT;
};

struct TimeRange {
    std::string start;
    std::string end;
    std::string days;
};

struct JunctionMode {
    int junction_id = -1;
    SignalMode mode = SignalMode::DYNAMIC;
};

struct CompoundCondition {
    LogicalOp op = LogicalOp::AND;
    std::vector<struct Condition> conditions;
};

struct Condition {
    std::optional<QueueThreshold> queue_threshold;
    std::optional<TimeRange> time_range;
    std::optional<JunctionMode> junction_mode;
    std::optional<CompoundCondition> compound;
};

struct SetPhaseTimingAction {
    int phase_id = -1;
    int green_time = -1;
    int yellow_time = -1;
    int red_time = -1;
    bool is_active = false;
};

struct SetActivePhaseAction {
    int approach_id = -1;
    int green_time = 30;
    int junction_id = -1;
};

struct EnableCorridorFreeAction {
    int corridor_id = -1;
    int green_time = 60;
    std::string exclude_junctions;
};

struct ResetToDynamicAction {
    int junction_id = -1;
    int corridor_id = -1;
};

struct SetModeAction {
    int junction_id = -1;
    SignalMode mode = SignalMode::DYNAMIC;
};

struct AdjustGreenTimeAction {
    int approach_id = -1;
    int adjustment_seconds = 0;
    int min_green = 10;
    int max_green = 120;
};

struct Action {
    std::optional<SetPhaseTimingAction> set_phase_timing;
    std::optional<SetActivePhaseAction> set_active_phase;
    std::optional<EnableCorridorFreeAction> enable_corridor_free;
    std::optional<ResetToDynamicAction> reset_to_dynamic;
    std::optional<SetModeAction> set_mode;
    std::optional<AdjustGreenTimeAction> adjust_green_time;
};

struct EcaRule {
    std::string id;
    int priority = 0;
    bool enabled = false;
    std::string name;
    std::string description;
    EventType event_type;
    SensorEvent sensor_event;
    JunctionEvent junction_event;
    ScheduleEvent schedule_event;
    CorridorEvent corridor_event;
    std::optional<Condition> condition;
    std::vector<Action> actions;
};

class RuleLoader {
  public:
    bool load_from_file(const std::string &path);
    bool load_from_string(const std::string &xml_content);

    const std::vector<EcaRule> &rules() const { return rules_; }
    std::string version() const { return version_; }
    bool global_enabled() const { return global_enabled_; }

    std::vector<const EcaRule *> find_matching_rules(EventType type) const;

  private:
    void parse_eca_rules(const pugi::xml_node &root);
    EcaRule parse_rule(const pugi::xml_node &rule_node);
    EventType parse_event_type(const pugi::xml_node &event_node);
    std::optional<Condition> parse_condition(const pugi::xml_node &cond_node);
    Action parse_action(const pugi::xml_node &action_node);

    static Comparison parse_comparison(const char *value);
    static JunctionEventType parse_junction_event_type(const char *value);
    static CorridorEventType parse_corridor_event_type(const char *value);
    static SignalMode parse_signal_mode(const char *value);
    static LogicalOp parse_logical_op(const char *value);

    std::vector<EcaRule> rules_;
    std::string version_;
    bool global_enabled_ = true;
};

} // namespace watcher
