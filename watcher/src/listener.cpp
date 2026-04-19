#include "watcher/listener.hpp"
#include "watcher/connection.hpp"
#include "watcher/dispatcher.hpp"
#include <chrono>
#include <iostream>
#include <thread>

namespace watcher {

Listener::Listener(Connection &conn, EventDispatcher &dispatcher) : conn_(conn), dispatcher_(dispatcher) {}

Listener::~Listener() {
    stop();
}

void Listener::start() {
    if (running_)
        return;
    running_ = true;
    thread_ = std::thread(&Listener::loop, this);
}

void Listener::stop() {
    running_ = false;
    if (thread_.joinable()) {
        thread_.join();
    }
}

void Listener::loop() {
    while (running_) {
        if (!conn_.consumeInput()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        PGnotify *notify;
        while ((notify = conn_.getNotify()) != nullptr) {
            processNotification(notify);
        }
    }
}

void Listener::processNotification(PGnotify *notify) {
    std::string channel(notify->relname);
    std::string payload(notify->extra ? notify->extra : "");

    std::cout << "[" << channel << "] " << payload << std::endl;
    std::cout.flush();

    dispatcher_.dispatch(channel, payload);
    if (callback_) {
        callback_(channel, payload);
    }

    PQfreemem(notify);
}

} // namespace watcher
