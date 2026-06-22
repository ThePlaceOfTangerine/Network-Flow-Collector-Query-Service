# C++ FlowLog Collector

This service is the system programming core of the FlowLog project.

## Current Features

- NormalizedFlow model
- Plugin-style interfaces:
  - ICollector
  - IParser
  - ISink
- HTTP sink for publishing normalized flow records to the REST ingest API
- Fake collector for end-to-end pipeline testing

## Build

```bash
cmake -S . -B build
cmake --build build
