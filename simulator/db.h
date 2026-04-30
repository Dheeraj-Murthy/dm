// DB connection and SQL execution helpers

#ifndef DB_H
#define DB_H

#include <string>
#include <libpq-fe.h>

PGconn *connect_db();
bool exec_sql(PGconn *conn, const std::string &sql);

#endif