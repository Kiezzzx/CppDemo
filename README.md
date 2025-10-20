# CppDemo - Multi-threaded Sensor System

A compact C++1 project that simulates a FreeRTOS-style multi-task system on PC, demonstrating task scheduling, thread-safe queues, data processing, and bit-level GPIO control.

## Features

- **Sensor Task**: Generates temperature data (200ms interval)
- **Data Processor**: Analyzes sensor data with anomaly detection  
- **LED Task**: Simulates GPIO operations (500ms interval)
- **Thread-safe Queue**: Custom TSQueue for data communication
- **Mutex Protection**: Safe console output across threads

## Build & Run

```bash
g++ -std=c++17 -pthread -I./include src/main.cpp -o main
./main
```

## Project Structure

```
include/
├── bitops.hpp      # Bit manipulation utilities
└── ts_queue.hpp    # Thread-safe queue implementation
src/
└── main.cpp        # Main application
```

## Sample Output

```
[SENSOR] temp=23.4degC
[PROCESSOR] temp=23.4degC, avg=23.4degC, samples=1
[LED] ON  REG=0x1
[WARNING] High temperature detected!
```
