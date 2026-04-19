# XML Input Specification for Smart Traffic Active Database

## Overview

This document defines the XML-based initialization inputs for the Smart Traffic
Light Control System.

---

## System Mapping

```
junctions → road_segments → approaches → signal_phases → sensors
```

---

## Required Inputs

### 1. Junctions

#### XML

```xml
<junctions>
    <junction name="J1" location="A"/>
    <junction name="J2" location="B"/>
</junctions>
```

#### SQL

```sql
INSERT INTO junctions (name, location)
VALUES ('J1', 'A'),
       ('J2', 'B');
```

---

### 2. Road Segments

#### XML

```xml
<road_segments>
    <segment from="J1" to="J2"
             road_name="Main Road"
             distance_km="0.5"
             speed_limit_kmph="40"
             lane_count="2"/>
</road_segments>
```

#### SQL

```sql
INSERT INTO road_segments (
    from_junction_id,
    to_junction_id,
    road_name,
    distance_km,
    speed_limit_kmph,
    lane_count
)
VALUES (
    (SELECT junction_id FROM junctions WHERE name = 'J1'),
    (SELECT junction_id FROM junctions WHERE name = 'J2'),
    'Main Road',
    0.5,
    40,
    2
);
```

---

### 3. Approaches

#### XML

```xml
<approaches>
    <approach junction="J2"
              incoming_segment="Main Road"
              name="J2_Main_Entry"/>
</approaches>
```

#### SQL

```sql
INSERT INTO approaches (
    junction_id,
    incoming_segment_id,
    approach_name
)
VALUES (
    (SELECT junction_id FROM junctions WHERE name = 'J2'),
    (SELECT segment_id FROM road_segments WHERE road_name = 'Main Road'),
    'J2_Main_Entry'
);
```

---

### 4. Signal Phases

#### XML

```xml
<signal_phases>
    <phase junction="J2"
           approach="J2_Main_Entry"
           name="PHASE_1"
           green_time="30"
           yellow_time="5"
           red_time="30"
           mode="DYNAMIC"/>
</signal_phases>
```

#### SQL

```sql
INSERT INTO signal_phases (
    junction_id,
    approach_id,
    phase_name,
    green_time,
    yellow_time,
    red_time,
    mode
)
VALUES (
    (SELECT junction_id FROM junctions WHERE name = 'J2'),
    (SELECT approach_id FROM approaches WHERE approach_name = 'J2_Main_Entry'),
    'PHASE_1',
    30,
    5,
    30,
    'DYNAMIC'
);
```

---

### 5. Sensors

#### XML

```xml
<sensors>
    <sensor approach="J2_Main_Entry"/>
</sensors>
```

#### SQL

```sql
INSERT INTO sensors (approach_id)
VALUES (
    (SELECT approach_id FROM approaches WHERE approach_name = 'J2_Main_Entry')
);
```

---

## Runtime Input (Simulator)

```sql
INSERT INTO sensor_readings (sensor_id, vehicle_count, avg_speed)
VALUES (1, 25, 30.5);
```

---

## Optional Inputs

### Corridors

```sql
INSERT INTO corridors (name)
VALUES ('Main Corridor');
```

---

### Config

```sql
UPDATE system_config
SET config_value = '15'
WHERE config_key = 'low_threshold';
```

---

## Insert Order

```
1. junctions
2. road_segments
3. approaches
4. signal_phases
5. sensors
6. corridors (optional)
7. config (optional)
```

---

## Summary

- XML → structure
- Simulator → events
- DB → logic (ECA)
