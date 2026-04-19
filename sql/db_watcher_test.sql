-- db_watcher_test.sql
-- Quick test for C++ watcher (run while ./build/watcher is running)
-- Tests notify_sensor_update which triggers notifications

-- Test 1: Low traffic
SELECT notify_sensor_update(1, 5, 20.0);
SELECT pg_sleep(0.5);

-- Test 2: Medium traffic  
SELECT notify_sensor_update(1, 15, 25.0);
SELECT pg_sleep(0.5);

-- Test 3: High traffic
SELECT notify_sensor_update(1, 45, 15.0);
SELECT pg_sleep(0.5);

-- Test 4: Different sensor
SELECT notify_sensor_update(5, 30, 30.0);
SELECT pg_sleep(0.5);

-- Test 5: Another junction
SELECT notify_sensor_update(3, 10, 28.0);
SELECT pg_sleep(0.5);

-- Test 6: Back to sensor 1
SELECT notify_sensor_update(1, 8, 22.0);
SELECT pg_sleep(0.5);

-- Test 7: Heavy load
SELECT notify_sensor_update(2, 60, 12.0);
SELECT pg_sleep(0.5);

SELECT 'Watcher tests complete! Check the C++ terminal for events.';
