// PostgreSQL connection and SQL queries

#include "db.h"
#include <iostream>
#include <cstdlib>

using namespace std;

string getenv_or(const char *var, const char *def)
{
    const char *val = getenv(var);
    return val ? string(val) : string(def);
}

PGconn *connect_db()
{
    string conninfo =
        "host=" + getenv_or("PGHOST", "localhost") +
        " port=" + getenv_or("PGPORT", "5432") +
        " dbname=" + getenv_or("PGDATABASE", "traffic_db") +
        " user=" + getenv_or("PGUSER", "postgres") +
        " password=" + getenv_or("PGPASSWORD", "postgres");

    PGconn *conn = PQconnectdb(conninfo.c_str());

    if (PQstatus(conn) != CONNECTION_OK)
    {
        cerr << "Connection failed: " << PQerrorMessage(conn) << endl;
        PQfinish(conn);
        return nullptr;
    }

    return conn;
}

bool exec_sql(PGconn *conn, const string &sql)
{
    PGresult *res = PQexec(conn, sql.c_str());

    if (PQresultStatus(res) != PGRES_COMMAND_OK &&
        PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        cerr << "SQL error: " << PQerrorMessage(conn) << endl;
        PQclear(res);
        return false;
    }

    PQclear(res);
    return true;
}