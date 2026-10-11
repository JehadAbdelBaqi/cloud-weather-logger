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
- Only the Nucleo asks in the handshake; the bridge answers, and takes any line from the Nucleo as proof it is there
- The Nucleo counts the connection as made only when the bridge answers the Nucleo's own request
- The bridge answers the handshake request with the same line type as any other line, and the result says whether it can reach Wi-Fi and the broker
- Interrupt-driven UART receive on the Nucleo, into a buffer in memory

## The protocol

- Text lines over UART, integers only, routed by their first letter
- The MQTT payload is the protocol line, unchanged
- One MQTT topic per line type, with the device ID in the topic
- QoS 1 on every topic
- Readings carry a sequence number and are acknowledged cumulatively
- The time is sent down to the Nucleo to set its RTC
- Commands and replies carry an ID
- The first field of every line sent up is its sequence number or ID
- The bridge answers a line with one line type, carrying that line's letter, its first field and the result
- Every response to the Nucleo is that one line type, from the bridge or from the server; a command is the other kind of line it receives
- A command is sent as a number, with its value as a field of its own
- A reply is the answer itself; nothing is sent only to say a command arrived
- A `read` command is answered with the reading, carrying the command's ID

## The Nucleo firmware

- STM32Cube HAL for the peripherals
- PlatformIO as the build tool, with the set-up code written by hand
- Drivers and building blocks in `firmware/lib/`, application code in `firmware/src/app/`
- The project's values in `firmware/include/config/`, the application's headers in `firmware/include/app/`
- One folder per library; a file's name says whether it operates a device, does one job with no hardware, or sits on a driver
- Number-to-text and text-to-number conversion in one library, `format`
- Every pin assignment in `board_config.h`; the libraries are given their pins when they are set up
- Tunable values in `config.h`
- The protocol's line types and fixed words in `protocol.h`
- The commands in a table, with the function for each at its number's position, beside the other route tables
- The MQTT topics in `topic_routes.h`
- A value is defined once; the bridge is handed its settings in `bridge_config.h`, filled from the project's own definitions
- One set of link settings and timings for both boards
- One function is one pass of the main loop: the jobs done in every state, then the function of the state the Nucleo is in
- What lasts from one pass to the next is one struct, declared in `main.c` and handed to every function that needs it
- The sensor code in `src/app/sensor/`: it only measures, and hands each reading back to its caller
- Building a message, and getting a field out of one, in one library, `message`, which is handed the field separator
- This project's messages in `src/app/messaging/`; deciding what is sent, and handling what arrives, in `src/app/operations/`
- An incoming message routed by its first letter, and a response then by its second, through tables in one file
- The status of a response sets the state the Nucleo is in
- A message built by one function from a description of its type, with every field already text
- The internal 16 MHz oscillator as the system clock
- The datasheet's integer formulas for the BME280 compensation
- Readings are timestamped on the Nucleo, with a stand-in time until the RTC is set up
- Readings are put into a store in memory and sent from there, one at a time, each staying until it is acknowledged
- One message is out at a time, whatever its kind: the next is sent once the one before is answered
- A reading's sequence number and time are given when it is stored
- The stores in `src/app/stores/`: one for readings and one for replies to commands, each built on the `ring` library
- A reply to a command is put into its store and sent from there, staying until it is acknowledged
- A waiting reply is sent ahead of the waiting readings
- The bridge answers every line: an acknowledgement once the broker has it, or a report that it has not gone through
- The bridge reports the state of its connection upstream
- A line the bridge does not answer in time sends the Nucleo back to the handshake
- One quiet limit for the link: it decides when an answer is given up on and when a quiet link is checked
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
