# Finalised Decisions

What this project uses and how it is put together: the decisions made and still in force.

[← README](../../README.md)

## How this file is used

- **It holds only decisions in force.** A decision that is dropped is removed from here; one that is replaced has its line rewritten. Nothing is listed as "not used".
- **One short line each.** The choice itself, without its settings or values (those belong in [docs/system/](../system/)) and without its reasons.
- **Not numbered.** The reasons, the alternatives and the full history are in [decision_logs.md](decision_logs.md), which is only ever added to.
- **A choice made, not a statement that it is built.** The [README](../../README.md) says what exists today.

## The system

- Readings travel over MQTT, through a broker
- Mosquitto as the broker locally, AWS IoT Core in the cloud, with the same topics and TLS client certificates
- Built and tested locally first; the cloud is a proof deployment
- A local Multipass Ubuntu VM as the twin of the EC2 server
- Server stack: ingest service → InfluxDB → Grafana
- Docker Compose for the server stack
- Its own repository

## Hardware

- Nucleo-F411RE as the microcontroller board
- BME280 over I2C for temperature, pressure and humidity
- Genesis Mini (ESP32-S3) as the Wi-Fi bridge
- The on-board user LED for link status
- The on-board RTC and its 32.768 kHz crystal for the time of each reading

## The link and the bridge

- The Wi-Fi bridge is the only link between the Nucleo and the broker
- USART1 for protocol lines, USART2 (the ST-LINK's USB serial port) for debug output
- The bridge's firmware comes from its own repository, `uart-mqtt-bridge`, built with this project's config
- The bridge's repository sits beside this one
- One script, `build.sh`, builds and uploads both boards; its steps are switched in `build-config.sh`
- The bridge repository's test broker is the broker side until this project's server stack exists
- A handshake between the Nucleo and the bridge before the link is used, and a watch on it afterwards
- The Nucleo counts the connection as made only when the bridge answers the Nucleo's own request
- Polled UART receive on the Nucleo

## The protocol

- Text lines over UART, integers only, routed by their first letter
- The MQTT payload is the protocol line, unchanged
- One MQTT topic per line type, with the device ID in the topic
- QoS 1 on every topic
- Readings carry a sequence number and are acknowledged cumulatively
- The time is sent down to the Nucleo to set its RTC
- Commands and replies carry an ID
- A `read` command produces a normal reading line

## The Nucleo firmware

- STM32Cube HAL for the peripherals
- PlatformIO as the build tool, with the set-up code written by hand
- Drivers and building blocks in `firmware/lib/`, application code in `firmware/src/app/`
- The project's values in `firmware/include/config/`, the application's headers in `firmware/include/app/`
- One folder per library; a file's name says whether it operates a device, does one job with no hardware, or sits on a driver
- Number-to-text conversion in one library, `format`
- Every pin assignment in `board_config.h`; the libraries are given their pins when they are set up
- Tunable values in `config.h`
- The protocol's line types and fixed words in `protocol.h`
- The commands in a table of words and the functions that carry them out
- The MQTT topics in `topic_routes.h`
- A value is defined once; the bridge is handed its settings in `bridge_config.h`, filled from the project's own definitions
- One set of link settings and handshake timings for both boards
- The code for lines to and from the bridge in `src/app/messaging/`: one handler for each line type, built from shared functions
- Building a line and finding its fields in one hardware-free file, tested on the PC
- The internal 16 MHz oscillator as the system clock
- The datasheet's integer formulas for the BME280 compensation
- Readings are timestamped on the Nucleo, with a stand-in time until the RTC is set up
- Readings are put into a store in memory and sent from there, one at a time, each staying until it is acknowledged
- The bridge acknowledges a reading once the broker has it, and reports the state of its connection upstream
- Store-and-forward driven by acknowledgements
- The sequence number starts again at power-on
- Unit tests on the PC, for the code that touches no hardware, written alongside new logic
- Unity as the test framework, run by PlatformIO
- A test of the running boards through the broker, as the last step of the build script
- A short report at the end of a build: one line for each step and check

## The server

- InfluxDB with identifiers as tags and readings as fields, plus a downsampling task for long-range graphs
- Two timestamps per reading: when it was taken and when it arrived
- A systemd unit runs the Compose stack, a systemd timer runs the backups, and container logs go to journald
- Separate least-privilege accounts for Ansible and for the application
- Backups through InfluxDB's API
- Only Grafana and Mosquitto publish ports
- A Grafana alert when readings stop arriving

## Provisioning and deployment

- Ubuntu as the development machine
- Ansible for configuration, run from the development machine, with one playbook for the VM and EC2
- cloud-init for the first-boot set-up
- Ansible over SSH locally, over SSM on EC2
- Images built locally, and pulled from ECR in the cloud
- A release is a new image tag plus a playbook re-run, through one release script for both environments
- AWS CDK in Python for the cloud infrastructure
- Grafana on EC2 reached through SSM port forwarding
- GitHub Actions for CI, with a hardware-in-the-loop test on a self-hosted runner

## Working method

- All work goes through pull requests, each with a description and a checklist
- Decisions are kept in two files: the log, and this file

**See also:** [decision_logs.md](decision_logs.md) · [architecture.md](../system/architecture.md) · [protocol.md](../system/protocol.md)
