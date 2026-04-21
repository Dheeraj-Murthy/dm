// Updates DB tables and triggers event notification for each sensor update

#include "simulator.h"
#include "db.h"
#include <string>

using namespace std;

bool update_tables(PGconn *conn, int sensor_id, int vehicle_count, double avg_speed)
{
    string sql1 =
        "UPDATE sensors SET vehicle_count = " + to_string(vehicle_count) +
        ", avg_speed = " + to_string(avg_speed) +
        ", last_updated = CURRENT_TIMESTAMP WHERE sensor_id = " + to_string(sensor_id) + ";";

    if (!exec_sql(conn, sql1))
        return false;

    string sql2 =
        "UPDATE approaches SET queue_count = " + to_string(vehicle_count) +
        ", last_updated = CURRENT_TIMESTAMP WHERE approach_id = "
        "(SELECT approach_id FROM sensors WHERE sensor_id = " +
        to_string(sensor_id) + ");";

    return exec_sql(conn, sql2);
}

bool call_notify(PGconn *conn, int sensor_id, int vehicle_count, double avg_speed)
{
    string sql =
        "SELECT notify_sensor_update(" +
        to_string(sensor_id) + ", " +
        to_string(vehicle_count) + ", " +
        to_string(avg_speed) + ");";

    return exec_sql(conn, sql);
}

bool simulate_sensor(PGconn *conn, int sensor_id, int vehicle_count, double avg_speed)
{
    if (!update_tables(conn, sensor_id, vehicle_count, avg_speed))
        return false;

    if (!call_notify(conn, sensor_id, vehicle_count, avg_speed))
        return false;

    return true;
}