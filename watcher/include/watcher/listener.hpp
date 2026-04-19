#pragma once

#include "watcher/connection.hpp"
#include "watcher/dispatcher.hpp"
#include <thread>
#include <atomic>
#include <functional>

namespace watcher {

class Listener {
public:
    using NotifyCallback = std::function<void(const std::string& channel, const std::string& payload)>;

    Listener(Connection& conn, EventDispatcher& dispatcher);
    ~Listener();

    void start();
    void stop();
    bool isRunning() const { return running_; }

    void setCallback(NotifyCallback cb) { callback_ = std::move(cb); }

private:
    void loop();
    void processNotification(PGnotify* notify);

    Connection& conn_;
    EventDispatcher& dispatcher_;
    std::thread thread_;
    std::atomic<bool> running_{false};
    NotifyCallback callback_;
};

}