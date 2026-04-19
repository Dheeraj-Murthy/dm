-- =========================================================
-- db_test.sql
-- Verification / demo script for ECA-based system
-- Run after db_create.sql and db_data.sql
-- =========================================================

TRUNCATE TABLE
    signal_schedule,
    traffic_history
RESTART IDENTITY CASCADE;

UPDATE approaches
SET queue_count = 0,
    last_updated = CURRENT_TIMESTAMP;

UPDATE sensors
SET vehicle_count = 0,
    avg_speed = NULL,
    last_updated = CURRENT_TIMESTAMP;

UPDATE signal_phases
SET green_time = 30,
    yellow_time = 5,
    red_time = 30,
    is_active = CASE
        WHEN phase_name LIKE '%PHASE_1' THEN TRUE
        ELSE FALSE
    END,
    mode = 'DYNAMIC',
    last_changed = CURRENT_TIMESTAMP;

-- =========================
-- BASIC STRUCTURE CHECK
-- =========================
SELECT 'JUNCTIONS' AS section;
SELECT * FROM junctions ORDER BY junction_id;

SELECT 'ROAD SEGMENTS' AS section;
SELECT * FROM road_segments ORDER BY segment_id;

SELECT 'APPROACHES' AS section;
SELECT * FROM approaches ORDER BY approach_id;

SELECT 'SIGNAL PHASES' AS section;
SELECT * FROM signal_phases ORDER BY phase_id;

SELECT 'SENSORS' AS section;
SELECT * FROM sensors ORDER BY sensor_id;

SELECT 'CORRIDORS' AS section;
SELECT * FROM corridors ORDER BY corridor_id;

-- =========================
-- TEST 1: LOW TRAFFIC (queue < 10)
-- Sensor 1 -> approach 1 at Junction B
-- Watcher would evaluate conditions and decide action
-- =========================
SELECT 'TEST 1: UPDATE SENSOR' AS section;

UPDATE sensors
SET vehicle_count = 7, avg_speed = 21.5, last_updated = CURRENT_TIMESTAMP
WHERE sensor_id = 1;

UPDATE approaches
SET queue_count = 7, last_updated = CURRENT_TIMESTAMP
WHERE approach_id = 1;

SELECT notify_sensor_update(1, 7, 21.5);

SELECT * FROM traffic_history ORDER BY record_id DESC LIMIT 5;
SELECT * FROM approaches WHERE approach_id = 1;
SELECT * FROM signal_phases WHERE junction_id = 2 ORDER BY phase_id;

-- =========================
-- TEST 2: MODERATE TRAFFIC (10 <= queue <= 40)
-- =========================
SELECT 'TEST 2: MODERATE TRAFFIC' AS section;

UPDATE sensors
SET vehicle_count = 18, avg_speed = 30.0, last_updated = CURRENT_TIMESTAMP
WHERE sensor_id = 1;

UPDATE approaches
SET queue_count = 18, last_updated = CURRENT_TIMESTAMP
WHERE approach_id = 1;

SELECT notify_sensor_update(1, 18, 30.0);

SELECT * FROM approaches WHERE approach_id = 1;
SELECT * FROM signal_phases WHERE junction_id = 2 ORDER BY phase_id;

-- =========================
-- TEST 3: HIGH TRAFFIC (queue > 40)
-- =========================
SELECT 'TEST 3: HIGH TRAFFIC' AS section;

UPDATE sensors
SET vehicle_count = 50, avg_speed = 10.0, last_updated = CURRENT_TIMESTAMP
WHERE sensor_id = 1;

UPDATE approaches
SET queue_count = 50, last_updated = CURRENT_TIMESTAMP
WHERE approach_id = 1;

SELECT notify_sensor_update(1, 50, 10.0);

SELECT * FROM approaches WHERE approach_id = 1;
SELECT * FROM signal_phases WHERE junction_id = 2 ORDER BY phase_id;

-- =========================
-- TEST 4: ECA ACTION - Set Active Phase
-- =========================
SELECT 'TEST 4: SET ACTIVE PHASE' AS section;

SELECT eca_set_active_phase(2, 45);

SELECT * FROM signal_phases WHERE junction_id = 2 ORDER BY phase_id;

-- =========================
-- TEST 5: ECA ACTION - Set Phase Timing
-- =========================
SELECT 'TEST 5: SET PHASE TIMING' AS section;

SELECT eca_set_phase_timing(1, 60, TRUE);

SELECT * FROM signal_phases WHERE phase_id = 1;

-- =========================
-- TEST 6: ECA ACTION - Corridor Free Mode
-- =========================
SELECT 'TEST 6: CORRIDOR FREE MODE' AS section;

SELECT eca_enable_corridor_free(1, 60);

SELECT * FROM signal_phases ORDER BY phase_id;

-- =========================
-- TEST 7: ECA ACTION - Reset to Dynamic
-- =========================
SELECT 'TEST 7: RESET TO DYNAMIC' AS section;

SELECT eca_reset_to_dynamic(NULL);

SELECT * FROM signal_phases ORDER BY phase_id;

-- =========================
-- TEST 8: Query Functions
-- =========================
SELECT 'TEST 8: QUERY FUNCTIONS' AS section;

SELECT 'get_sensor_data' AS test;
SELECT * FROM get_sensor_data();

SELECT 'get_approaches_above_threshold(30)' AS test;
SELECT * FROM get_approaches_above_threshold(30);

SELECT 'get_junction_phases(2)' AS test;
SELECT * FROM get_junction_phases(2);

-- =========================
-- FINAL STATE
-- =========================
SELECT 'FINAL APPROACH STATES' AS section;
SELECT * FROM approaches ORDER BY approach_id;

SELECT 'FINAL SENSOR STATES' AS section;
SELECT * FROM sensors ORDER BY sensor_id;

SELECT 'FINAL PHASE STATES' AS section;
SELECT * FROM signal_phases ORDER BY phase_id;

SELECT 'FINAL COUNTS' AS section;
SELECT
    (SELECT COUNT(*) FROM traffic_history) AS traffic_history_count,
    (SELECT COUNT(*) FROM signal_schedule) AS signal_schedule_count;
