TRUNCATE TABLE
    signal_schedule,
    traffic_history,
    sensors,
    signal_phases,
    approaches,
    corridor_segments,
    corridors,
    road_segments,
    junctions
RESTART IDENTITY CASCADE;

INSERT INTO junctions (name, location) VALUES
('Junction A', 'North Square'),
('Junction B', 'Central Avenue'),
('Junction C', 'East Market'),
('Junction D', 'South Park'),
('Junction E', 'West Station');

INSERT INTO road_segments (from_junction_id, to_junction_id, road_name, distance_km, speed_limit_kmph, lane_count) VALUES
(1, 2, 'A-B Main', 0.50, 40, 2),
(2, 1, 'B-A Main', 0.50, 40, 2),

(2, 3, 'B-C Market', 0.70, 40, 2),
(3, 2, 'C-B Market', 0.70, 40, 2),

(2, 4, 'B-D South', 0.80, 35, 2),
(4, 2, 'D-B South', 0.80, 35, 2),

(4, 5, 'D-E Link', 0.60, 30, 1),
(5, 4, 'E-D Link', 0.60, 30, 1),

(5, 1, 'E-A Ring', 0.90, 45, 1),
(1, 5, 'A-E Ring', 0.90, 45, 1),

(3, 5, 'C-E Cross', 1.00, 50, 1),
(5, 3, 'E-C Cross', 1.00, 50, 1);

INSERT INTO approaches (junction_id, incoming_segment_id, approach_name) VALUES
(2, 1,  'A_to_B_in'),
(1, 2,  'B_to_A_in'),

(3, 3,  'B_to_C_in'),
(2, 4,  'C_to_B_in'),

(4, 5,  'B_to_D_in'),
(2, 6,  'D_to_B_in'),

(5, 7,  'D_to_E_in'),
(4, 8,  'E_to_D_in'),

(1, 9,  'E_to_A_in'),
(5, 10, 'A_to_E_in'),

(5, 11, 'C_to_E_in'),
(3, 12, 'E_to_C_in');

INSERT INTO signal_phases (junction_id, approach_id, phase_name, green_time, yellow_time, red_time, is_active, mode) VALUES
(1, 2, 'A_PHASE_1', 30, 5, 30, TRUE,  'DYNAMIC'),
(1, 9, 'A_PHASE_2', 30, 5, 30, FALSE, 'DYNAMIC'),

(2, 1, 'B_PHASE_1', 30, 5, 30, TRUE,  'DYNAMIC'),
(2, 4, 'B_PHASE_2', 30, 5, 30, FALSE, 'DYNAMIC'),
(2, 6, 'B_PHASE_3', 30, 5, 30, FALSE, 'DYNAMIC'),

(3, 3, 'C_PHASE_1', 30, 5, 30, TRUE,  'DYNAMIC'),
(3, 12, 'C_PHASE_2', 30, 5, 30, FALSE, 'DYNAMIC'),

(4, 5, 'D_PHASE_1', 30, 5, 30, TRUE,  'DYNAMIC'),
(4, 8, 'D_PHASE_2', 30, 5, 30, FALSE, 'DYNAMIC'),

(5, 7, 'E_PHASE_1', 30, 5, 30, TRUE,  'DYNAMIC'),
(5, 10, 'E_PHASE_2', 30, 5, 30, FALSE, 'DYNAMIC'),
(5, 11, 'E_PHASE_3', 30, 5, 30, FALSE, 'DYNAMIC');

INSERT INTO sensors (approach_id, vehicle_count, sensor_type, is_active) VALUES
(1, 0, 'QUEUE_COUNTER', TRUE),
(2, 0, 'QUEUE_COUNTER', TRUE),
(3, 0, 'QUEUE_COUNTER', TRUE),
(4, 0, 'QUEUE_COUNTER', TRUE),
(5, 0, 'QUEUE_COUNTER', TRUE),
(6, 0, 'QUEUE_COUNTER', TRUE),
(7, 0, 'QUEUE_COUNTER', TRUE),
(8, 0, 'QUEUE_COUNTER', TRUE),
(9, 0, 'QUEUE_COUNTER', TRUE),
(10, 0, 'QUEUE_COUNTER', TRUE),
(11, 0, 'QUEUE_COUNTER', TRUE),
(12, 0, 'QUEUE_COUNTER', TRUE);

INSERT INTO corridors (name) VALUES
('Main Corridor'),
('Ring Corridor');

INSERT INTO corridor_segments (corridor_id, segment_id, order_index) VALUES
(1, 1, 1),
(1, 3, 2),

(2, 10, 1),
(2, 8, 2),
(2, 6, 3);

SELECT 'junctions' AS section, COUNT(*) AS total FROM junctions;
SELECT 'road_segments' AS section, COUNT(*) AS total FROM road_segments;
SELECT 'approaches' AS section, COUNT(*) AS total FROM approaches;
SELECT 'signal_phases' AS section, COUNT(*) AS total FROM signal_phases;
SELECT 'sensors' AS section, COUNT(*) AS total FROM sensors;
SELECT 'corridors' AS section, COUNT(*) AS total FROM corridors;
