# Software Requirements Specification (SRS)

## Smart Traffic Light Control System using Active Database

**Course:** Data Modelling  
**Project Type:** Active Database Design and Simulation  
**Technologies:** PostgreSQL, C/C++, libpq  
**Version:** 1.0

---

## 1. Introduction

### 1.1 Purpose

The purpose of this project is to design and implement an active database system
for smart traffic light control in a locality. The system uses simulated lane
sensors to detect the number of waiting vehicles and dynamically controls
traffic signals based on real-time traffic conditions.

### 1.2 Scope

The project focuses on simulation and academic demonstration. It does not
interface with physical traffic lights or real hardware sensors. Instead, the
database is connected to simulated sensors and simulated signal controllers to
mimic realistic junction behavior.

The system is intended to:

- model traffic junctions, lanes, sensors, and lights in a database
- support active database behavior using triggers
- update signal timing according to vehicle counts
- switch from dynamic timing to periodic timing after a cutoff threshold
- support smart corridor synchronization for improved traffic flow
- simulate traffic scenarios and evaluate outcomes

### 1.3 Definitions, Acronyms, and Abbreviations

- **SRS** — Software Requirements Specification
- **DBMS** — Database Management System
- **ECA** — Event-Condition-Action
- **PL/pgSQL** — Procedural Language for PostgreSQL
- **Dynamic Timing** — signal duration changes according to waiting vehicles
- **Periodic Mode** — fixed cyclic timing used after threshold is exceeded
- **Green Wave / Smart Corridor** — synchronized green signals along a route

### 1.4 Document Overview

This document presents the software requirements for the Smart Traffic Light
Control System, including the overall architecture, system features, active
database behavior, smart corridor extension, interfaces, and constraints.

---

## 2. Overall Description

### 2.1 Product Perspective

The system is a simulated traffic management platform built around an active
PostgreSQL database. Sensor readings are generated through a C/C++ simulation
program and stored in the database. PostgreSQL triggers process these updates
and modify traffic signal timings automatically.

The system consists of:

- simulated traffic sensors
- a PostgreSQL active database
- a C/C++ traffic simulation module
- a signal controller module that reads decisions from the database

### 2.2 Product Functions

The major functions of the system are:

- store information about junctions, lanes, sensors, and traffic lights
- accept sensor updates for waiting vehicle counts
- dynamically adjust green signal duration below a threshold
- switch to periodic operation above a threshold
- record traffic history for analysis
- coordinate green signals along a smart corridor
- simulate multiple traffic scenarios

### 2.3 User Classes and Characteristics

The main users of the system are:

- **Student Developers** — implement and test the database and simulation
- **Course Instructors / Evaluators** — verify correctness and assess design
  quality
- **Demonstration Users** — run the simulation and observe signal behavior

### 2.4 Operating Environment

The system shall run in an environment with:

- PostgreSQL installed and configured
- C/C++ compiler support
- `libpq` client library for PostgreSQL connectivity
- a terminal or command-line environment for simulation execution

### 2.5 Design Constraints

- The simulation logic must be implemented using **C/C++**
- The database must be implemented using **PostgreSQL**
- Active behavior must be implemented using **database triggers and procedural
  logic**
- Sensors and traffic lights are simulated
- The project must remain suitable for locality-scale academic simulation

### 2.6 Assumptions and Dependencies

- Each lane has an associated simulated sensor
- Sensor readings provide the number of waiting vehicles
- Signal timing decisions are derived from sensor updates
- Junction and corridor topology are predefined in the database
- Average speed values for corridor synchronization are estimated within the
  simulation

---

## 3. Objectives

The project objectives are:

- design a database schema representing traffic junctions, lanes, sensors, and
  traffic lights
- implement an active database using PostgreSQL triggers to automatically adjust
  signal timings
- develop a traffic simulation program in C/C++ that generates sensor data
- implement dynamic traffic light timing based on vehicle count
- extend the system to support smart corridor synchronization for efficient
  travel

---

## 4. System Architecture

### 4.1 Architectural Overview

The system includes the following components:

- simulated traffic sensors that detect the number of vehicles in each lane
- a PostgreSQL active database that stores traffic information and applies
  trigger-based updates
- a C/C++ simulation program that updates sensor values
- a signal controller that reads database decisions and simulates traffic light
  changes

### 4.2 Data Flow

1. The simulation program generates lane-wise vehicle counts.
2. Sensor values are updated in the database.
3. Database triggers are fired on sensor updates.
4. Signal timing is recalculated based on active rules.
5. The signal controller reads the updated values and simulates light
   transitions.

---

## 5. Database Design

### 5.1 Database Overview

The database stores information about traffic junctions, lanes, sensors, signal
timings, corridor definitions, and traffic history. The schema supports active
database triggers that automatically adjust signal timing based on traffic
conditions.

### 5.2 Main Entities

The core entities in the system are:

- **JUNCTIONS**
- **LANES**
- **SENSORS**
- **TRAFFIC_LIGHTS**
- **TRAFFIC_HISTORY**
- **SIGNAL_SCHEDULE**
- **CORRIDORS**
- **CORRIDOR_NODES**
- **SYSTEM_CONFIG**

### 5.3 Entity Descriptions

#### JUNCTIONS

Stores intersection-level information.

- `junction_id`
- `name`
- `location`

#### LANES

Stores incoming lanes associated with a junction.

- `lane_id`
- `junction_id`
- `direction`
- `length`

#### SENSORS

Stores simulated vehicle counts for each lane.

- `sensor_id`
- `lane_id`
- `vehicle_count`
- `last_updated`

#### TRAFFIC_LIGHTS

Stores current signal state and timing values for lanes.

- `light_id`
- `lane_id`
- `state`
- `green_time`
- `red_time`
- `last_changed`

#### TRAFFIC_HISTORY

Stores historical traffic readings.

- `record_id`
- `lane_id`
- `vehicle_count`
- `recorded_at`

#### SIGNAL_SCHEDULE

Stores active lane scheduling information at each junction.

- `schedule_id`
- `junction_id`
- `active_lane`
- `start_time`
- `end_time`

#### CORRIDORS

Stores corridor definitions.

- `corridor_id`
- `name`

#### CORRIDOR_NODES

Maps ordered junctions inside a corridor.

- `node_id`
- `corridor_id`
- `junction_id`
- `order_index`
- `distance_to_next`

#### SYSTEM_CONFIG

Stores configurable system parameters.

- `config_key`
- `config_value`

---

## 6. Active Database Rules

The system uses Event–Condition–Action (ECA) rules.

### 6.1 Rule 1: Dynamic Signal Update

- **Event:** Sensor update
- **Condition:** Vehicle count below threshold
- **Action:** Increase green signal duration dynamically

### 6.2 Rule 2: Periodic Mode Activation

- **Event:** Sensor update
- **Condition:** Vehicle count exceeds predefined threshold
- **Action:** Switch signal timing to a periodic pattern to maintain stability

### 6.3 Rule 3: Traffic History Logging

- **Event:** Sensor update
- **Condition:** Sensor record modified
- **Action:** Insert corresponding traffic data into history log

### 6.4 Trigger Requirement

The active behavior shall be implemented using PostgreSQL triggers and PL/pgSQL
functions.

---

## 7. Functional Requirements

### 7.1 Junction and Lane Management

- The system shall store multiple junctions.
- The system shall store multiple lanes for each junction.
- The system shall support lane properties such as direction and length.

### 7.2 Sensor Data Handling

- The system shall maintain one sensor record for each monitored lane.
- The system shall accept simulated sensor updates from the C/C++ program.
- The system shall store current vehicle counts and update timestamps.

### 7.3 Dynamic Light Timing

- The system shall compute green signal duration based on vehicle count when the
  count is below a threshold.
- The system shall update the timing automatically after a sensor update.
- The system shall associate timing changes with the affected lane.

### 7.4 Periodic Operation

- The system shall switch to fixed periodic timing when vehicle count exceeds
  the cutoff threshold.
- The threshold shall be configurable using the configuration table.

### 7.5 Signal Scheduling

- The system shall maintain the current state of each traffic light.
- The system shall support lane scheduling through the signal schedule table.
- The controller module shall read the updated timing and state from the
  database.

### 7.6 Traffic History

- The system shall log traffic readings whenever a sensor update occurs.
- The system shall support later analysis of traffic scenarios.

### 7.7 Smart Corridor Support

- The system shall support corridor definitions using ordered junction lists.
- The system shall store the distance between consecutive corridor nodes.
- The system shall estimate travel time using average speed.
- The system shall coordinate green windows along a corridor.

### 7.8 Simulation

- The system shall support simulated traffic input.
- The system shall support multiple scenarios including varying traffic density.
- The system shall demonstrate traffic flow improvement through dynamic control
  and corridor synchronization.

---

## 8. Smart Corridor Extension

The system includes a smart corridor feature in which traffic signals along a
road corridor are synchronized using distance and average vehicle speed to
create a green wave.

### Requirements

- The system shall define a corridor as an ordered sequence of junctions.
- The system shall compute inter-junction travel timing using distance and
  average speed.
- The system shall schedule coordinated green intervals across the corridor.
- The system shall aim to reduce stoppage for vehicles moving along the
  corridor.

---

## 9. External Interface Requirements

### 9.1 User Interface

The system may use a terminal-based output for demonstration.

- The system should display current vehicle counts by lane.
- The system should display current traffic light states.
- The system should display active timing values and schedule information.

### 9.2 Software Interface

The system interfaces with:

- **PostgreSQL** for data storage and active rules
- **PL/pgSQL** for procedural trigger logic
- **C/C++** for simulation and controller logic
- **libpq** for database communication

### 9.3 Hardware Interface

No physical hardware interface is required in the current simulated version.

### 9.4 Communication Interface

The C/C++ modules shall communicate with the database using SQL statements
through the PostgreSQL client library.

---

## 10. Non-Functional Requirements

### 10.1 Performance

- The system should process sensor updates quickly enough for smooth simulation.
- Trigger execution should support repeated updates during continuous runs.

### 10.2 Reliability

- The database should maintain consistent signal, lane, and sensor data.
- Trigger execution should not leave the system in an invalid state.

### 10.3 Maintainability

- The schema should remain modular and understandable.
- Threshold values and timing constants should be configurable through database
  tables.
- Simulation logic and database logic should remain separated.

### 10.4 Scalability

- The schema should support additional junctions, lanes, and corridors.
- The system should remain usable for locality-scale network simulations.

### 10.5 Portability

- The system should compile and run on standard environments supporting
  PostgreSQL and C/C++.

---

## 11. Expected Outcomes

The expected outcomes of the project are:

- a working active database for traffic control
- dynamic adjustment of traffic signal timings
- simulation of multiple traffic scenarios
- demonstration of improved traffic flow using smart corridors

---

## 12. Technologies Used

- PostgreSQL (Active Database)
- C/C++ (Traffic simulation + Database Wrapper)
- libpq (Database connectivity)

---

## 13. Conclusion

This project demonstrates the practical application of active databases in
traffic management systems. By integrating sensor data with automated database
triggers, the system dynamically adapts traffic signals to changing traffic
conditions and improves overall flow in a simulated locality.
