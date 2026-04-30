#pragma once

#include <functional>
#include <map>
#include <nlohmann/json.hpp>
#include <string>
#include "watcher/connection.hpp"
#include "watcher/rules/rule_loader.hpp"

namespace watcher {

using json = nlohmann::json;

class EventHandler {
public:
  virtual ~EventHandler() = default;
  virtual void handle(const std::string &channel, const json &data,
                      const RuleLoader &rules, Connection &conn) = 0;
  virtual std::string name() const = 0;
};

class EventDispatcher {
public:
  using HandlerPtr = std::unique_ptr<EventHandler>;
  using HandlerFactory = std::function<HandlerPtr()>;

  void setRuleLoader(const RuleLoader *rules) { rules_ = rules; }
  void setConnection(Connection *conn) { conn_ = conn; }

  void registerHandler(const std::string &channel, HandlerPtr handler);
  void registerFactory(const std::string &channel, HandlerFactory factory);
  void dispatch(const std::string &channel, const std::string &payload);

private:
  const RuleLoader *rules_ = nullptr;
  Connection *conn_ = nullptr;
  std::map<std::string, HandlerPtr> handlers_;
  std::map<std::string, HandlerFactory> factories_;
};

} // namespace watcher
