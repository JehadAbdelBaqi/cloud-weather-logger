# Cloud Weather Logger

An STM32 weather logger: temperature, pressure and humidity from a BME280,
held in memory while offline, sent over MQTT to a Docker stack
(ingest → InfluxDB → Grafana) that runs on a local Ubuntu VM or on AWS EC2.

## Overview

```
 [BME280] ─I2C─┐
               ▼
        [Nucleo-F411RE] ─UART─► [Wi-Fi bridge] ─MQTT/TLS─► [broker]
         │   ▲                                                 │
         │   └── commands, acks, time ◄────────────────────────┤
         │                                                     ▼
         │                                           server: Docker Compose
         └─SPI─► [3.5" ILI9488 screen]               ingest → InfluxDB → Grafana

 LOCAL:  broker = Mosquitto            server = Multipass Ubuntu 24.04 VM
 CLOUD:  broker = AWS IoT Core         server = EC2
```

## What it does

- **Samples** temperature, pressure and humidity on a timer, and on demand.
- **Timestamps** every reading on the device from its real-time clock.
- **Keeps every reading in memory** until the server confirms it is stored, then
  resends anything missed — a short link, broker or database outage leaves no
  gap. When the memory is full it stops taking readings until there is room.
- **Takes commands** from the server over the same path the readings use.
- **Shows** the latest reading and device health on its own screen.
- **Graphs** the data in Grafana, and alerts when readings stop arriving.
- **Deploys the same way everywhere** — one Ansible playbook and one release
  script for the local VM and for EC2.

## Components

| Component | Job |
|-----------|-----|
| Nucleo-F411RE (STM32Cube HAL, PlatformIO) | Samples the sensor, stores and sends readings, answers commands |
| BME280 (I2C) | Temperature / pressure / humidity sensor |
| 3.5" ILI9488 screen (SPI) | Latest reading, time, link status, backlog |
| Wi-Fi bridge (ESP32-S3, `uart-mqtt-bridge` firmware) | The link: UART ↔ MQTT over Wi-Fi/TLS |
| Mosquitto / AWS IoT Core | MQTT broker — switched by config only |
| Ingest (Python container) | Subscribes to readings, writes them to InfluxDB, sends acks |
| InfluxDB | Time-series storage |
| Grafana | Dashboard and alerts, provisioned from files |
| Ansible | Configures the VM and EC2 with the same playbook |
| AWS CDK (Python) | Cloud infrastructure |
| GitHub Actions | Tests, builds, hardware-in-the-loop test, cloud release |

## Documentation

| Doc | Covers |
|-----|--------|
| [docs/system/architecture.md](docs/system/architecture.md) | The pieces, how data flows, local vs cloud |
| [docs/system/protocol.md](docs/system/protocol.md) | The UART line format between the Nucleo and the bridge |
| [docs/system/firmware.md](docs/system/firmware.md) | The Nucleo firmware: peripherals, store-and-forward, time, screen |
| [docs/system/server.md](docs/system/server.md) | The Linux host and the Docker stack |
| [docs/system/deployment.md](docs/system/deployment.md) | Provisioning, releases, CI and the cloud infrastructure |
| [docs/project-design/decision_logs.md](docs/project-design/decision_logs.md) | The log of design decisions, in the order they were made, and why |
| [docs/project-design/finalised-decisions.md](docs/project-design/finalised-decisions.md) | The decisions in force today, by subject |
