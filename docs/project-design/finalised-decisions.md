# Finalised Decisions

The design decisions in force today, by subject. This file is rewritten as
decisions change; [decision_logs.md](decision_logs.md) is the full log, in the order the
decisions were made, with the alternatives each one was chosen over.

[← README](../../README.md)

The **Log** column gives the numbers of the log entries a row comes from. A
decision listed here is a choice that has been made, not a statement that it is
built: the [README](../../README.md) says what exists today.

## The system

| Decision | Why | Log |
|----------|-----|-----|
| **Readings travel over MQTT through a broker**; the server's ingest service subscribes as its own client with its own certificate | The server needs no inbound port for data, since it connects out to the broker | 1 |
| **Mosquitto locally, AWS IoT Core in the cloud** — same topics and TLS client certificates; switching is config only | The whole pipeline is built and tested without cloud resources or cost | 2 |
| **Built and tested locally first, cloud last; local stays the primary environment and the cloud is a proof deployment** | The first cloud deploy runs software already known to work; cost and cloud debugging stay small | 3, 20 |
| **A local Multipass Ubuntu 24.04 VM mirrors the EC2 server and stays a working deploy target** — one Ansible playbook, one config per environment | The playbook is exercised locally before it touches EC2, and the stack can run without AWS at all | 4, 19 |
| **Server stack: ingest → InfluxDB → Grafana in Docker Compose** | InfluxDB is built for time-series data; Grafana gives a dashboard without a front end; each piece is replaceable | 5 |
| **Its own repository** | A different purpose and different components from the earlier clock-scaling project | 9 |

## The link and the bridge

| Decision | Why | Log |
|----------|-----|-----|
| **The Wi-Fi bridge (Genesis Mini, ESP32-S3) is the only link, from the first protocol test** — protocol lines on USART1; USART2, the ST-LINK's USB serial port, carries debug output only | One link means one firmware build and one code path, tested the same way at the bench and in the cloud | 40, 53 |
| **The bridge is its own repository, `uart-mqtt-bridge`, built at a named version with this project's config** | The bridge is general-purpose and passes lines through unchanged, so no bridge code belongs here | 52 |
| **This project's files for the bridge live with what they belong to** — `scripts/bridge-config.sh` and `scripts/build-config.sh` with the scripts, `firmware/bridge/bridge-config.h` with the firmware — and are handed to the bridge's scripts by path; nothing is written into the bridge's repository, and the bridge's scripts make the certificates | The bridge is used as it is; each file sits beside what it configures | 67, 70 |
| **The bridge's repository sits in the same folder as this one**; there is no setting for where it is | One layout to state, nothing to fill in | 71 |
| **One script builds and uploads both boards**; it does not use the bridge's dummy data source | One command brings both boards to a known state, with a single copy of the bridge's build steps | 68 |
| **`build.sh` takes no options: the steps are a list in `scripts/build-config.sh`, each with a switch in front of it** | What a run does is set in one file and the command is always the same | 72 |
| **Until this project's own server stack exists, the bridge repository's test setup is the broker side** — its Multipass VM with Mosquitto and its scripts, with `mosquitto_pub` / `mosquitto_sub` standing in for the server; anything the bridge is missing is added in the bridge's repository | The protocol is tested end to end over MQTT from the first firmware phase | 54 |
| **UART link at 115200 baud, 8N1, lines end in `\n`** (`\r\n` also accepted on receive) | Matches the bridge's UART settings and line handling | 44 |
| **Polled UART receive on the Nucleo** — no receive interrupt or ring buffer | Commands coming in are short and rare | 15 |

## The protocol

| Decision | Why | Log |
|----------|-----|-----|
| **One MQTT topic per line type, under `weather/<device>/`** — `readings`, `replies`, `acks`, `commands`; device ID `nucleo-01` | Each client's policy allows only the topics it needs | 45 |
| **The MQTT payload is the protocol line as it is; QoS 1 on every topic** | The format is defined in one place; a duplicate delivery is harmless because a reading with the same `seq` is written once | 46, 47 |
| **Reading line: `W,<seq>,<epoch>,<t_centi_C>,<p_Pa>,<rh_centi_pct>`**, integers only | No floating-point maths or printing on the Nucleo; the scale is fixed by the protocol | 33 |
| **Cumulative ack: `A,<seq>`** — every reading up to and including `seq` is stored | One small line acknowledges many readings; a lost reading cannot be freed by a later ack | 35 |
| **Time line `T,<epoch>`**, sent down on every connect and then hourly; no readings are taken after a power-up until the first one has set the RTC | Every stored reading has a true time, and RTC drift is corrected | 36 |
| **Commands and replies carry an ID** — `C,<id>,<command>` down, `R,<id>,...` up; commands are `help`, `read`, `rate` and are sent from the server through the broker | A reply can always be matched to its command; the command path is the one the readings use | 14, 38 |
| **`read` produces a normal `W` line** plus `R,<id>,ok` | One code path for every reading | 39 |

## The Nucleo firmware

| Decision | Why | Log |
|----------|-----|-----|
| **Peripherals are driven through the STM32Cube HAL**, built with PlatformIO (`framework = stm32cube`); setup is written by hand, and STM32CubeMX is a reference only | ST's supported driver layer; one build tool for both firmwares; every file in the repository is hand-written | 58, 59 |
| **The firmware lives in `firmware/`, with each module in its own folder under `firmware/lib/`**; `src/` holds only `main.c`, `include/` only `board.h` | The firmware is one part of the repository among several; a module's files sit together | 60, 62 |
| **System clock is the internal 16 MHz oscillator, no PLL, no bus dividers** | Nothing here needs more speed; no PLL factors or flash wait states to get right | 61 |
| **The UART code is two layers: a `uart` driver and a `link` module on top** | Each module has one job; the debug UART uses the same driver | 63 |
| **Readings are timestamped on the Nucleo by its RTC**, on the on-board 32.768 kHz crystal, set from the bridge's time line | Readings replayed from flash keep their true time | 32 |
| **SPI flash (W25Q64) and store-and-forward are in the core build, driven by acknowledgements** — every reading is kept until its `seq` is acked; unacked readings are resent | Nothing is lost while the link, broker, ingest or database is down; they all look the same to the device | 17, 31 |
| **`seq` carries on across reboots** — found by scanning the flash log on boot, with the last value also kept in its own sector | A reused `seq` would collide with readings already acknowledged | 34 |
| **A 3.5" ILI9488 screen on the Nucleo, in the core build**, sharing the SPI bus with the flash | Readings and device health are visible with no PC, network or server | 42 |

## The server

| Decision | Why | Log |
|----------|-----|-----|
| **InfluxDB schema planned for queries, plus a downsampling task** — identifiers as tags, readings as fields, hourly averages in a separate bucket | Filters use the index; long-range graphs read a few hourly points | 21 |
| **Two timestamps per reading** — the Nucleo's sample time is the point time; the server's arrival time is a field | Arrival minus sample time shows delivery delay and which readings were replayed | 37 |
| **The Compose stack is run by a systemd unit**, InfluxDB backups run on a systemd timer, and container logs go to journald | Ordered start after Docker and the data disk; missed backups are caught up; one place for logs, with their size capped | 22, 23, 24 |
| **Separate accounts with least privilege** — `deploy` for Ansible, `weather` for the application; root login and password login off; backups go through InfluxDB's API with a token | A compromised application process reaches only its own files; `weather` stays out of the `docker` group | 25, 26 |
| **Minimal published ports, bound per environment** — only Grafana and Mosquitto publish ports | Docker's own firewall rules bypass ufw, so not publishing is what keeps a port closed | 27 |
| **A deadman alert in the core build** — Grafana alerts when no reading has arrived for 5 minutes | Shows that data has stopped arriving from anywhere in the chain | 41 |

## Provisioning and deployment

| Decision | Why | Log |
|----------|-----|-----|
| **The development machine is Ubuntu; Ansible runs on it directly.** Windows is not supported | The control node needs Linux, with no layer between the repository and the playbook | 55 |
| **cloud-init bootstraps, Ansible configures** — one cloud-init file creates the `deploy` user at first boot; Ansible does everything else on every run | Ansible logs in the same way from its first run, with its own account | 28 |
| **Ansible over SSH locally, over SSM on EC2** | EC2 needs no open SSH port; access is controlled by IAM | 6 |
| **Images are built locally, and pulled from ECR in the cloud** | No cloud dependency while developing; in the cloud the server runs exactly what CI produced | 7 |
| **Updates ship as a new image tag plus a playbook re-run, through one release script for both environments** | Every release takes the path that built the server; the release path is tested locally before CI runs it | 29, 30 |
| **Cloud infrastructure in Python CDK** | Versioned, reviewable and unit-testable | 8 |
| **Grafana on EC2 is reached through SSM port forwarding** | No public port, domain or certificate to manage | 18 |

## Testing and working method

| Decision | Why | Log |
|----------|-----|-----|
| **A hardware-in-the-loop test in CI** — a self-hosted runner with the Nucleo attached flashes each firmware build and checks its replies. *How the commands are sent is open, now that the USB serial port carries debug output only* | Every firmware change is tested on the real board, not only compiled | 43 |
| **All work goes through pull requests** with a description and a checklist | Each change is reviewable as one unit; the main branch holds only merged, working states | 56 |
| **No step-by-step build guide and no per-phase git tags** — the README carries the instructions for the finished system; the pull requests are the record of how it was built | Nothing extra to keep correct as earlier code changes | 57 |
| **Two decision files: the log, and this file of decisions in force** | The log shows how the design got here; this file states the current design directly | 69 |

## Stretch

| Decision | Why | Log |
|----------|-----|-----|
| **A read-now button** — a button press takes and sends a reading at once | Timed readings and the `read` command already cover on-demand readings | 16 |

**See also:** [decision_logs.md](decision_logs.md) · [architecture.md](../system/architecture.md) · [protocol.md](../system/protocol.md)
