DROP TRIGGER IF EXISTS trg_handle_sensor_update ON sensor_readings;
DROP FUNCTION IF EXISTS handle_sensor_update();

DROP FUNCTION IF EXISTS get_config_int(VARCHAR);
DROP FUNCTION IF EXISTS log_traffic_history;
DROP FUNCTION IF EXISTS create_corridor_schedule;

DROP TABLE IF EXISTS signal_schedule CASCADE;
DROP TABLE IF EXISTS traffic_history CASCADE;
DROP TABLE IF EXISTS sensor_readings CASCADE;
DROP TABLE IF EXISTS sensors CASCADE;
DROP TABLE IF EXISTS signal_phases CASCADE;
DROP TABLE IF EXISTS approaches CASCADE;
DROP TABLE IF EXISTS corridor_segments CASCADE;
DROP TABLE IF EXISTS corridors CASCADE;
DROP TABLE IF EXISTS road_segments CASCADE;
DROP TABLE IF EXISTS junctions CASCADE;
DROP TABLE IF EXISTS system_config CASCADE;

CREATE TABLE junctions (
    junction_id SERIAL PRIMARY KEY,
    name VARCHAR(100) NOT NULL UNIQUE,
    location TEXT,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE road_segments (
    segment_id SERIAL PRIMARY KEY,
    from_junction_id INT NOT NULL REFERENCES junctions(junction_id) ON DELETE CASCADE,
    to_junction_id   INT NOT NULL REFERENCES junctions(junction_id) ON DELETE CASCADE,
    road_name VARCHAR(100) NOT NULL,
    distance_km NUMERIC(8,2) NOT NULL CHECK (distance_km > 0),
    speed_limit_kmph NUMERIC(8,2) NOT NULL DEFAULT 40 CHECK (speed_limit_kmph > 0),
    lane_count INT NOT NULL DEFAULT 1 CHECK (lane_count > 0),
    CONSTRAINT chk_distinct_junctions CHECK (from_junction_id <> to_junction_id)
);

CREATE TABLE approaches (
    approach_id SERIAL PRIMARY KEY,
    junction_id INT NOT NULL REFERENCES junctions(junction_id) ON DELETE CASCADE,
    incoming_segment_id INT NOT NULL UNIQUE REFERENCES road_segments(segment_id) ON DELETE CASCADE,
    approach_name VARCHAR(100) NOT NULL,
    queue_count INT NOT NULL DEFAULT 0 CHECK (queue_count >= 0),
    last_updated TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE signal_phases (
    phase_id SERIAL PRIMARY KEY,
    junction_id INT NOT NULL REFERENCES junctions(junction_id) ON DELETE CASCADE,
    approach_id INT NOT NULL REFERENCES approaches(approach_id) ON DELETE CASCADE,
    phase_name VARCHAR(100) NOT NULL,
    green_time INT NOT NULL DEFAULT 30 CHECK (green_time BETWEEN 5 AND 180),
    yellow_time INT NOT NULL DEFAULT 5 CHECK (yellow_time BETWEEN 3 AND 15),
    red_time INT NOT NULL DEFAULT 30 CHECK (red_time BETWEEN 5 AND 180),
    is_active BOOLEAN NOT NULL DEFAULT FALSE,
    mode VARCHAR(20) NOT NULL DEFAULT 'DYNAMIC'
        CHECK (mode IN ('DYNAMIC', 'PERIODIC', 'MANUAL', 'ECA_CONTROLLED')),
    last_changed TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    UNIQUE (junction_id, approach_id)
);

CREATE TABLE sensors (
    sensor_id SERIAL PRIMARY KEY,
    approach_id INT NOT NULL UNIQUE REFERENCES approaches(approach_id) ON DELETE CASCADE,
    vehicle_count INT NOT NULL DEFAULT 0 CHECK (vehicle_count >= 0),
    avg_speed NUMERIC(6,2),
    last_updated TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    sensor_type VARCHAR(30) NOT NULL DEFAULT 'QUEUE_COUNTER',
    is_active BOOLEAN NOT NULL DEFAULT TRUE
);

CREATE TABLE traffic_history (
    record_id SERIAL PRIMARY KEY,
    approach_id INT NOT NULL REFERENCES approaches(approach_id) ON DELETE CASCADE,
    vehicle_count INT NOT NULL CHECK (vehicle_count >= 0),
    avg_speed NUMERIC(6,2),
    recorded_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE corridors (
    corridor_id SERIAL PRIMARY KEY,
    name VARCHAR(100) NOT NULL UNIQUE
);

CREATE TABLE corridor_segments (
    corridor_segment_id SERIAL PRIMARY KEY,
    corridor_id INT NOT NULL REFERENCES corridors(corridor_id) ON DELETE CASCADE,
    segment_id INT NOT NULL REFERENCES road_segments(segment_id) ON DELETE CASCADE,
    order_index INT NOT NULL,
    UNIQUE (corridor_id, order_index),
    UNIQUE (corridor_id, segment_id)
);

CREATE TABLE signal_schedule (
    schedule_id SERIAL PRIMARY KEY,
    corridor_id INT REFERENCES corridors(corridor_id) ON DELETE CASCADE,
    junction_id INT NOT NULL REFERENCES junctions(junction_id) ON DELETE CASCADE,
    phase_id INT NOT NULL REFERENCES signal_phases(phase_id) ON DELETE CASCADE,
    start_time TIMESTAMP NOT NULL,
    end_time TIMESTAMP NOT NULL,
    CONSTRAINT chk_schedule_window CHECK (end_time > start_time)
);

CREATE TABLE system_config (
    config_key VARCHAR(100) PRIMARY KEY,
    config_value VARCHAR(100) NOT NULL
);

CREATE INDEX idx_approaches_junction ON approaches(junction_id);
CREATE INDEX idx_sensors_approach ON sensors(approach_id);
CREATE INDEX idx_signal_phases_junction ON signal_phases(junction_id);

INSERT INTO system_config (config_key, config_value) VALUES
('low_threshold', '10'),
('high_threshold', '40'),
('min_green_time', '15'),
('max_green_time', '90'),
('periodic_green_time', '30'),
('default_yellow_time', '5'),
('default_red_time', '30'),
('default_speed_kmph', '40');

CREATE OR REPLACE FUNCTION get_config_int(p_key VARCHAR)
RETURNS INT AS $$
DECLARE v_value INT;
BEGIN
    SELECT config_value::INT INTO v_value
    FROM system_config WHERE config_key = p_key;

    IF v_value IS NULL THEN
        RAISE EXCEPTION 'Missing config key: %', p_key;
    END IF;

    RETURN v_value;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION log_traffic_history(
    p_approach_id INT,
    p_vehicle_count INT,
    p_avg_speed NUMERIC DEFAULT NULL,
    p_recorded_at TIMESTAMPTZ DEFAULT CURRENT_TIMESTAMP
)
RETURNS VOID AS $$
BEGIN
    INSERT INTO traffic_history (approach_id, vehicle_count, avg_speed, recorded_at)
    VALUES (p_approach_id, p_vehicle_count, p_avg_speed, p_recorded_at::timestamp);
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION get_sensor_data()
RETURNS TABLE (
    sensor_id INT,
    approach_id INT,
    junction_id INT,
    vehicle_count INT,
    avg_speed NUMERIC(6,2),
    last_updated TIMESTAMPTZ
) AS $$
BEGIN
    RETURN QUERY
    SELECT 
        s.sensor_id,
        s.approach_id,
        a.junction_id,
        s.vehicle_count,
        s.avg_speed,
        s.last_updated
    FROM sensors s
    JOIN approaches a ON s.approach_id = a.approach_id
    WHERE s.is_active = TRUE;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION get_approach_queue_status(p_approach_id INT)
RETURNS TABLE (
    approach_id INT,
    junction_id INT,
    queue_count INT,
    last_updated TIMESTAMPTZ
) AS $$
BEGIN
    RETURN QUERY
    SELECT 
        a.approach_id,
        a.junction_id,
        a.queue_count,
        a.last_updated
    FROM approaches a
    WHERE a.approach_id = p_approach_id;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION notify_sensor_update(
    p_sensor_id INT,
    p_vehicle_count INT,
    p_avg_speed NUMERIC DEFAULT NULL
)
RETURNS VOID AS $$
DECLARE
    v_approach_id INT;
    v_junction_id INT;
BEGIN
    SELECT s.approach_id, a.junction_id
    INTO v_approach_id, v_junction_id
    FROM sensors s
    JOIN approaches a ON s.approach_id = a.approach_id
    WHERE s.sensor_id = p_sensor_id;

    PERFORM log_traffic_history(v_approach_id, p_vehicle_count, p_avg_speed);

    PERFORM pg_notify(
        'sensor_update_channel',
        json_build_object(
            'type', 'SENSOR_UPDATE',
            'sensor_id', p_sensor_id,
            'approach_id', v_approach_id,
            'junction_id', v_junction_id,
            'vehicle_count', p_vehicle_count,
            'avg_speed', p_avg_speed,
            'timestamp', CURRENT_TIMESTAMP
        )::text
    );
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION eca_set_phase_timing(
    p_phase_id INT,
    p_green_time INT,
    p_is_active BOOLEAN DEFAULT NULL
)
RETURNS VOID AS $$
BEGIN
    UPDATE signal_phases
    SET 
        green_time = p_green_time,
        is_active = COALESCE(p_is_active, is_active),
        mode = 'ECA_CONTROLLED',
        last_changed = CURRENT_TIMESTAMP
    WHERE phase_id = p_phase_id;
    
    PERFORM pg_notify(
        'signal_update_channel',
        json_build_object(
            'type', 'PHASE_UPDATE',
            'phase_id', p_phase_id,
            'green_time', p_green_time,
            'is_active', COALESCE(p_is_active, FALSE),
            'timestamp', CURRENT_TIMESTAMP
        )::text
    );
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION eca_set_active_phase(
    p_approach_id INT,
    p_green_time INT DEFAULT 30
)
RETURNS VOID AS $$
DECLARE
    v_junction_id INT;
    v_phase_id INT;
BEGIN
    SELECT junction_id INTO v_junction_id
    FROM approaches WHERE approach_id = p_approach_id;

    UPDATE signal_phases
    SET is_active = FALSE, mode = 'ECA_CONTROLLED', last_changed = CURRENT_TIMESTAMP
    WHERE junction_id = v_junction_id;

    UPDATE signal_phases
    SET is_active = TRUE, green_time = p_green_time, mode = 'ECA_CONTROLLED', last_changed = CURRENT_TIMESTAMP
    WHERE approach_id = p_approach_id
    RETURNING phase_id INTO v_phase_id;

    PERFORM pg_notify(
        'junction_event_channel',
        json_build_object(
            'type', 'ACTIVE_PHASE_CHANGE',
            'junction_id', v_junction_id,
            'approach_id', p_approach_id,
            'phase_id', v_phase_id,
            'green_time', p_green_time,
            'timestamp', CURRENT_TIMESTAMP
        )::text
    );
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION eca_enable_corridor_free(
    p_corridor_id INT,
    p_green_time INT DEFAULT 60
)
RETURNS VOID AS $$
DECLARE
    v_junction_id INT;
    v_phase_id INT;
    rec RECORD;
BEGIN
    FOR rec IN 
        SELECT DISTINCT j.junction_id
        FROM junctions j
        JOIN approaches a ON a.junction_id = j.junction_id
        JOIN corridor_segments cs ON cs.segment_id = a.incoming_segment_id
        WHERE cs.corridor_id = p_corridor_id
    LOOP
        SELECT phase_id INTO v_phase_id
        FROM signal_phases
        WHERE junction_id = rec.junction_id
        LIMIT 1;

        IF v_phase_id IS NOT NULL THEN
            UPDATE signal_phases
            SET green_time = p_green_time, is_active = TRUE, mode = 'ECA_CONTROLLED', last_changed = CURRENT_TIMESTAMP
            WHERE junction_id = rec.junction_id;

            PERFORM pg_notify(
                'junction_event_channel',
                json_build_object(
                    'type', 'CORRIDOR_FREE',
                    'corridor_id', p_corridor_id,
                    'junction_id', rec.junction_id,
                    'green_time', p_green_time,
                    'timestamp', CURRENT_TIMESTAMP
                )::text
            );
        END IF;
    END LOOP;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION eca_reset_to_dynamic(p_junction_id INT DEFAULT NULL)
RETURNS VOID AS $$
BEGIN
    UPDATE signal_phases
    SET mode = 'DYNAMIC'
    WHERE p_junction_id IS NULL OR junction_id = p_junction_id;

    PERFORM pg_notify(
        'junction_event_channel',
        json_build_object(
            'type', 'MODE_RESET',
            'junction_id', COALESCE(p_junction_id, -1),
            'timestamp', CURRENT_TIMESTAMP
        )::text
    );
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION get_approaches_above_threshold(p_threshold INT)
RETURNS TABLE (approach_id INT, junction_id INT, queue_count INT) AS $$
BEGIN
    RETURN QUERY
    SELECT a.approach_id, a.junction_id, a.queue_count
    FROM approaches a
    WHERE a.queue_count > p_threshold
    ORDER BY a.queue_count DESC;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION get_junction_phases(p_junction_id INT)
RETURNS TABLE (phase_id INT, approach_id INT, green_time INT, is_active BOOLEAN, mode VARCHAR) AS $$
BEGIN
    RETURN QUERY
    SELECT sp.phase_id, sp.approach_id, sp.green_time, sp.is_active, sp.mode
    FROM signal_phases sp
    WHERE sp.junction_id = p_junction_id;
END;
$$ LANGUAGE plpgsql;
