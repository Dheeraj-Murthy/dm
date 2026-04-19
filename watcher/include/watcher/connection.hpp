#pragma once

#include <libpq-fe.h>
#include <string>
#include <memory>

namespace watcher {

class Connection {
public:
    explicit Connection(const std::string& connStr);
    ~Connection();

    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;
    Connection(Connection&&) noexcept;
    Connection& operator=(Connection&&) noexcept;

    bool isConnected() const;
    PGconn* raw() { return conn_.get(); }

    bool execute(const std::string& sql);
    bool listen(const std::string& channel);
    bool consumeInput();
    PGnotify* getNotify();

private:
    struct PGconnDeleter {
        void operator()(PGconn* p) { PQfinish(p); }
    };
    std::unique_ptr<PGconn, PGconnDeleter> conn_;
};

}