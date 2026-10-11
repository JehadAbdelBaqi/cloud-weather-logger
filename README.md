# Cloud Weather Logger

An STM32 weather logger. A Nucleo board reads temperature, pressure and
humidity from a BME280 and sends each reading over MQTT, holding it in memory
until it is acknowledged. The full project carries those readings on to a
server that stores and graphs them, on a local machine or in the cloud.

**Version `v0.1.0`: the device side.** The Nucleo, the sensor and the link to
an MQTT broker work and are tested on the boards. The server and the cloud are
planned and not built yet.

## Where it stands

| Works in `v0.1.0` | Planned |
|-------------------|---------|
| Readings taken on a timer and sent to an MQTT broker | The real time on each reading, from the board's clock |
| Every reading held in memory until it is acknowledged, and sent again if it was not | A server that takes the readings in: ingest, InfluxDB, Grafana |
| Commands taken over the same path: list the commands, take a reading now, change the rate | A local virtual machine as the server, set up by Ansible |
| A local test broker, with TLS and client certificates | The same server in the cloud: AWS IoT Core as the broker, EC2 as the host |
| Tests of the firmware on the PC, and of both boards running, on every build | Tests and releases run by CI |

## Overview

```
                      works in v0.1.0                          │        planned
                                                               │
 [BME280] ─I2C─┐                                               │
               ▼                                               │
        [Nucleo-F411RE] ─UART─► [Wi-Fi bridge] ─MQTT/TLS─► [broker] ──► server: Docker Compose
             ▲                                                 │        ingest → InfluxDB → Grafana
             └── commands, and the answer to every line ◄──────┤
                                                               │
 TODAY:  broker = Mosquitto in a virtual machine on the PC     │
 LATER:  broker = Mosquitto on the server, or AWS IoT Core     │
```

The Wi-Fi bridge is a second board with firmware of its own,
[uart-mqtt-bridge](https://github.com/JehadAbdelBaqi/uart-mqtt-bridge). This
release is built with its `v0.2.0`.

## What `v0.1.0` does

- **Samples** temperature, pressure and humidity on a timer.
- **Keeps every reading in memory** until the broker has it, and sends again
  anything that was not acknowledged. A link or broker outage leaves no gap:
  readings taken meanwhile go out in order once the path is back. When the
  memory is full it stops taking readings until there is room.
- **Sends one message at a time**, and waits for the bridge's answer to each.
- **Takes commands** from the broker: list the commands, take a reading now,
  change how often readings are taken.
- **Notices a lost link.** When the bridge stops answering, the Nucleo asks
  whether it is there, and carries on from the same reading once it is.
- **Shows its state** on the board's LED.
- **Gives each reading a time**, which for now is a stand-in: a fixed starting
  time plus the seconds since the board was switched on. The real time is
  planned.

Readings are lost if the Nucleo restarts or loses power: the store is memory.

## What the full project will be

- **Real time on every reading**, from the Nucleo's real-time clock, set
  through the bridge.
- **A server** that subscribes to the readings, writes them to InfluxDB and
  graphs them in Grafana, with an alert when readings stop arriving.
- **A local virtual machine as the server**, configured by Ansible, as a twin
  of the cloud one.
- **The same stack in the cloud**: AWS IoT Core as the broker and EC2 as the
  host, with the infrastructure in AWS CDK.
- **CI** for the tests, the builds and the releases.

The docs for these parts describe the design, and say so at their top:
[server.md](docs/system/server.md) and
[deployment.md](docs/system/deployment.md).

## Getting started

It runs on Linux.

### What you need

| Part | Used for |
|------|----------|
| Nucleo-F411RE | Runs this firmware |
| BME280 module (I2C) | The sensor |
| Genesis Mini (ESP32-S3) | Runs the Wi-Fi bridge |
| Jumper wires and a breadboard | The wiring below |
| Two USB cables | One to each board: power, uploading, the serial output |
| A 2.4 GHz Wi-Fi network, with the PC on it | The bridge's route to the broker |

The software is PlatformIO, Multipass, `openssl`, `ssh`, `iptables` and
`mosquitto-clients`. The list with versions is the bridge's:
[resources.md](https://github.com/JehadAbdelBaqi/uart-mqtt-bridge/blob/v0.2.0/docs/system/resources.md).

### Wiring

The sensor to the Nucleo:

| BME280 module | Nucleo |
|---------------|--------|
| Vin | 3.3 V |
| GND | GND |
| SCL | PB8 (Arduino D15) |
| SDA | PB9 (Arduino D14) |

The bridge to the Nucleo:

| Genesis Mini | Nucleo |
|--------------|--------|
| GPIO6 (RX) | PA9 (Arduino D8), the Nucleo's TX |
| GPIO7 (TX) | PA10 (Arduino D2), the Nucleo's RX |
| GND | GND |

Every pin is set in `firmware/include/config/board_config.h`.

### Building and running it

1. Clone the bridge's repository **beside this one**, at the version this
   release is built with:
   ```
   git clone https://github.com/JehadAbdelBaqi/uart-mqtt-bridge.git
   git -C uart-mqtt-bridge checkout v0.2.0
   ```
   The two folders have to sit in the same folder: the build reaches the
   bridge at `../uart-mqtt-bridge`.
2. Copy `scripts/bridge-config.example.sh` to `scripts/bridge-config.sh` and
   put in your Wi-Fi network's name and password. That file is not committed.
3. Plug both boards in, and from `scripts/`:
   ```
   bash build.sh
   ```

That one command runs the firmware's unit tests on the PC, builds and uploads
the Nucleo, creates the test broker, builds and uploads the bridge with this
project's config, and then tests both boards running. It asks for the `sudo`
password once, for the rule that lets the bridge reach the broker. It ends
with a report:

```
---------------- Report ----------------
 1. unit-tests                         success
 2. build-and-upload-nucleo            success
 3. bridge: create-vm                  success
 4. bridge: setup-certs                success
 5. bridge: set-port-rule              success
 6. bridge: build-and-upload           success
 7. test: readings arrive              success
 8. test: commands answered            success
 9. test: rate changes the timing      success
10. test: reading interval and order   success
11. test: kept while broker stopped    success
12. test: Nucleo lost and regained     success
13. test: kept while bridge restarts   success
```

Which steps run is set in `scripts/build-config.sh`, each with a switch in
front of it. After the first run, the steps that create the broker can be
switched off.

After a restart of the PC the rule is gone, and the bridge cannot reach the
broker until it is put back: from the bridge's `scripts/` folder,
`SCRIPT_CONFIG=<full path of this project's scripts/bridge-config.sh> bash steps/set-port-rule.sh`.

## Watching it, and sending a command

`<connection>` below stands for the four options every command takes, with the
PC's own address on the Wi-Fi network. The folder under `~/certs/` has the
project name from your `bridge-config.sh`:

```
-h <PC's address> -p 8883 --cafile ~/certs/cloud-weather-logger/ca.crt --cert ~/certs/cloud-weather-logger/client.crt --key ~/certs/cloud-weather-logger/client.key
```

| To | Command |
|----|---------|
| Watch the readings | `mosquitto_sub <connection> -t weather/nucleo-01/readings -v` |
| Watch the replies to commands | `mosquitto_sub <connection> -t weather/nucleo-01/replies -v` |
| Send a command | `mosquitto_pub <connection> -t weather/nucleo-01/commands -m "<command>" -q 1` |

A command is `C,<id>,<number>`, with a value after it where the command takes
one. The ID is any short text of your own; the reply carries it back.

| Command | Does | Reply |
|---------|------|-------|
| `C,7,1` | Lists the commands | `R,7,1=help 2=read 3=rate` |
| `C,7,2` | Takes a reading now | The reading: `R,7,<time>,<temperature>,<pressure>,<humidity>` |
| `C,7,3,10` | Takes a reading every 10 s from now on | `R,7,ok` |

A reading is `W,<number>,<time>,<temperature>,<pressure>,<humidity>`, with the
temperature in hundredths of a degree Celsius, the pressure in pascals and the
humidity in hundredths of a percent. The lines in full are in
[protocol.md](docs/system/protocol.md).

## Status LED

The Nucleo's green user LED (LD2):

| LED | Meaning |
|-----|---------|
| Blinking fast | No response from the bridge: the Nucleo is asking whether it is there |
| Blinking slowly | The bridge is answering, and readings are going up |

The bridge has an LED of its own, described in its README.

## Components

**In `v0.1.0`:**

| Component | Job |
|-----------|-----|
| Nucleo-F411RE (STM32Cube HAL, PlatformIO) | Samples the sensor, stores and sends readings, answers commands |
| BME280 (I2C) | Temperature / pressure / humidity sensor |
| Wi-Fi bridge (ESP32-S3, `uart-mqtt-bridge` firmware) | The link: UART ↔ MQTT over Wi-Fi/TLS |
| Mosquitto in a Multipass virtual machine | The test broker, made by the bridge's scripts |
| Scripts | One command to build both boards and test them |

**Planned:**

| Component | Job |
|-----------|-----|
| Mosquitto / AWS IoT Core | The broker, switched by config only |
| Ingest (Python container) | Subscribes to readings, writes them to InfluxDB, acknowledges them |
| InfluxDB | Time-series storage |
| Grafana | Dashboard and alerts, provisioned from files |
| Ansible | Configures the virtual machine and EC2 with the same playbook |
| AWS CDK (Python) | Cloud infrastructure |
| GitHub Actions | Tests, builds, a test on the hardware, cloud releases |

## Repository layout

```
 firmware/                  the Nucleo's firmware, a PlatformIO project
   src/main.c                 set-up, then the main loop
   src/app/                   the application: the sensor, the stores, the messages, what is sent and when
   include/config/            the project's values: pins, timings, the protocol's letters, the topics,
                              and the config handed to the bridge
   include/app/               the application's headers
   lib/                       the libraries: drivers and building blocks, one folder each
   test/                      unit tests, run on the PC
 scripts/
   build.sh                   everything: unit tests, both boards, the tests of the running boards
   build-config.sh            which steps build.sh runs
   steps/                     the steps, each a script of its own
   helpers/                   the functions the steps are built from
   bridge-config.example.sh   template for your own bridge-config.sh
 docs/                      see Documentation below
```

## Documentation

| Doc | Covers | State |
|-----|--------|-------|
| [docs/system/architecture.md](docs/system/architecture.md) | The pieces and how data flows | Built and planned, marked |
| [docs/system/protocol.md](docs/system/protocol.md) | The lines between the Nucleo, the bridge and the server | Built and planned, marked |
| [docs/system/firmware.md](docs/system/firmware.md) | The Nucleo firmware: code layout, config, how readings are held and sent, commands, testing | Built and planned, marked |
| [firmware/test/README.md](firmware/test/README.md) | The firmware's unit tests: running them, what they cover, adding one | Built |
| [docs/system/server.md](docs/system/server.md) | The Linux host and the Docker stack | Planned |
| [docs/system/deployment.md](docs/system/deployment.md) | Provisioning, releases, CI and the cloud infrastructure | Planned |
| [docs/project-design/decision_logs.md](docs/project-design/decision_logs.md) | The log of design decisions, in the order they were made, and why | |
| [docs/project-design/finalised-decisions.md](docs/project-design/finalised-decisions.md) | The decisions in force today, by subject: choices made, built or not | |
| [docs/project-design/risks.md](docs/project-design/risks.md) | What could go wrong, and what is done about each | |

## How this was built

No vibes were coded in the making of this project.

This project was built with heavy use of AI, specifically Claude Code. I
directed the architecture and made every design decision, each one recorded
with its reasoning in [decision_logs.md](docs/project-design/decision_logs.md). Claude
was used to find information, write and refactor code under my direction, and
explain anything I didn't yet understand, so that I could review it, question
it and test it on the hardware myself.

## Licence

MIT — see [LICENSE](LICENSE). Use it, change it, build on it.
