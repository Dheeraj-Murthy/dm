// Functions for updating DB and triggering events

#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <libpq-fe.h>

bool update_tables(PGconn *conn, int sensor_id, int vehicle_count, double avg_speed);
bool call_notify(PGconn *conn, int sensor_id, int vehicle_count, double avg_speed);
bool simulate_sensor(PGconn *conn, int sensor_id, int vehicle_count, double avg_speed);

#endif