# SQL Watcher - Real-Time Traffic Management System

## Overview

C++ application connects to PostgreSQL and listens for NOTIFY events. Enables
"active database" pattern - C++ app reacts to DB changes in real-time.

## Architecture

```
PostgreSQL DB (Triggers) → NOTIFY → C++ Watcher (Listener → Dispatcher → Handlers → ECA Rules → DB Actions)
```

## Components

| File                                          | Purpose                               |
| --------------------------------------------- | ------------------------------------- |
| `include/watcher/connection.hpp`              | PostgreSQL connection wrapper (RAII)  |
| `include/watcher/listener.hpp`                | Background thread polling NOTIFY      |
| `include/watcher/dispatcher.hpp`              | Route events to handlers + pass rules |
| `include/watcher/rules/rule_loader.hpp`       | Parse ECA rules from XML (pugixml)    |
| `include/watcher/handlers/sensor_handler.hpp` | Process sensor events + execute rules |
| `include/watcher/handlers/signal_handler.hpp` | Process signal events + execute rules |
| `src/main.cpp`                                | Entry point - init + run              |

## Build

```bash
cd watcher
rm -rf build/* _deps
cd build
cmake .. && make -j4
```

**Portable**: Auto-detects PostgreSQL via Homebrew or system paths. No hardcoded
dirs.

## Run

```bash
# Terminal 1: Start watcher
cd watcher/build
./watcher

# Output:
# Connecting to traffic_db...
# Connected.
# Loaded 9 rules (version 1.0)
#   - [rule-001] High Traffic Response (priority 10)
#   - [rule-002] Low Traffic Optimization (priority 5)
#   ...
# Listening for events... (Ctrl+C to exit)
```

## Data Flow

1. **Sensor data inserted** → PostgreSQL trigger fires
2. **Trigger sends NOTIFY** with JSON payload
3. **Listener** (background thread) polls `PQconsumeInput()`
4. **Dispatcher** routes to correct handler by channel name
5. **Handler** matches ECA rules + **executes SQL actions** against DB

### Example Event

```
[sensor_update_channel] {"type":"SENSOR_UPDATE","sensor_id":1,"vehicle_count":50}
Processing sensor update...
  Data: { "sensor_id": 1, "vehicle_count": 50, ... }
  Matching rules: 3
    Rule: High Traffic Response (priority 10)
      ACTION: Set phase 1 green=90
    Rule: Low Traffic Optimization (priority 5)
      ACTION: Adjust green time 10s (min=10 max=120)
```

## ECA Rules

Rules loaded from `../sql/xml/eca-rules-example.xml` (or set `WATCHER_ROOT` env
var).

```xml
<rule id="rule-001" priority="10" enabled="true">
  <name>High Traffic Response</name>
  <event><sensor-update vehicle-count-min="41"/></event>
  <condition><queue-threshold threshold="40" comparison="gt"/></condition>
  <action><set-phase-timing green-time="90" is-active="true"/></action>
</rule>
```

Supported actions:

- `set-phase-timing` → UPDATE phases
- `set-active-phase` → UPDATE approaches
- `enable-corridor-free` → UPDATE corridors
- `reset-to-dynamic` → UPDATE junctions
- `set-mode` → UPDATE junctions
- `adjust-green-time` → UPDATE approaches

## Key Features

- **RAII Connection**: `unique_ptr<PGconn, PGconnDeleter>` auto-cleans
- **Thread Safety**: `atomic<bool>` for graceful shutdown
- **ECA Rule Engine**: XML-based rules with pugixml (header-only)
- **Rule Matching**: `find_matching_rules(EventType)` in handlers
- **Action Execution**: Rules auto-execute SQL against DB
- **Portable Build**: CMake auto-detects PostgreSQL paths

## Dependencies

- **PostgreSQL** (libpq) - via `find_package()`
- **nlohmann/json** v3.11.3 - via FetchContent
- **pugixml** v1.14 - via FetchContent (header-only)
- **C++20** required

## Running the Full System

```bash
# 1. Setup DB
createdb traffic_db
psql -d traffic_db -f sql/db_create.sql
psql -d traffic_db -f sql/db_data.sql

# 2. Build + run watcher
cd watcher/build && cmake .. && make && ./watcher

# 3. Trigger test events (another terminal)
psql -d traffic_db -f sql/db_watcher_test.sql
```

## Project Structure

```
watcher/
├── CMakeLists.txt           # Portable build config
├── include/watcher/
│   ├── config.hpp
│   ├── connection.hpp      # libpq wrapper
│   ├── dispatcher.hpp      # Event routing + rule passing
│   ├── listener.hpp        # Background NOTIFY polling
│   └── rules/
│       └── rule_loader.hpp # ECA XML parser
├── src/
│   ├── main.cpp            # Entry point
│   ├── connection.cpp
│   ├── listener.cpp
│   ├── dispatcher.cpp
│   ├── rules/
│   │   └── rule_loader.cpp
│   └── handlers/
│       ├── sensor_handler.cpp  # Executes rules → SQL
│       └── signal_handler.cpp # Executes rules → SQL
└── build/                  # (gitignored)
```
