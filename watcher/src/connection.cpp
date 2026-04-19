#include "watcher/connection.hpp"
#include <memory>
#include <stdexcept>
#include <string>

namespace watcher {

Connection::Connection(const std::string &connStr) : conn_(PQconnectdb(connStr.c_str()), PGconnDeleter{}) {
    if (!isConnected()) {
        throw std::runtime_error("Failed to connect: " + std::string(PQerrorMessage(raw())));
    }
}

Connection::~Connection() = default;

Connection::Connection(Connection &&) noexcept = default;
Connection &Connection::operator=(Connection &&) noexcept = default;

bool Connection::isConnected() const {
    return conn_ && PQstatus(conn_.get()) == CONNECTION_OK;
}

bool Connection::execute(const std::string &sql) {
    PGresult *res = PQexec(raw(), sql.c_str());
    ExecStatusType status = PQresultStatus(res);
    PQclear(res);
    return status == PGRES_COMMAND_OK;
}

bool Connection::listen(const std::string &channel) {
    return execute("LISTEN " + channel);
}

bool Connection::consumeInput() {
    return PQconsumeInput(raw()) != 0;
}

PGnotify *Connection::getNotify() {
    return PQnotifies(raw());
}

} // namespace watcher
