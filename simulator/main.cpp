// Runs simulation loop generating random traffic data and sending it to DB

#include <iostream>
#include <thread>
#include <chrono>
#include <random>
#include <vector>
#include "db.h"
#include "simulator.h"

using namespace std;

int main()
{
    PGconn *conn = connect_db();
    if (!conn)
        return 1;

    vector<int> sensor_ids = {1, 2, 3, 4, 5};

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> vehicle_dist(0, 60);
    uniform_real_distribution<> speed_dist(10.0, 60.0);

    while (true)
    {
        for (int id : sensor_ids)
        {
            int v = vehicle_dist(gen);
            double s = speed_dist(gen);

            if (simulate_sensor(conn, id, v, s))
            {
                cout << "Sensor " << id
                     << " → vehicles=" << v
                     << ", speed=" << s << endl;
            }
        }

        this_thread::sleep_for(chrono::seconds(2));
    }

    PQfinish(conn);
    return 0;
}