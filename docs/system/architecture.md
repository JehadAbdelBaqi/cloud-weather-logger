# Architecture

The pieces of the system, how data moves between them, and how the local and
cloud environments differ.

[← README](../../README.md)

## Data flow

```
 UP    Nucleo ─UART─► bridge ─MQTT─► broker ─► ingest ─► InfluxDB ─► Grafana
       readings (W), command replies (R)

 DOWN  Nucleo ◄─UART─ bridge ◄─MQTT─ broker ◄─ ingest / any container
       acks (A), time (T), commands (C)
```

- **Readings travel over MQTT through a broker.** The ingest service connects
  out to the broker as its own client with its own certificate; the server has
  no inbound port for data.
- **The bridge is two-way.** Readings go up; commands, acknowledgements and the
  time come down the same path.
- **The Nucleo speaks plain text lines over UART** (see
  [protocol.md](protocol.md)). Wi-Fi, TLS and MQTT live in the bridge.

## The Wi-Fi bridge

The Nucleo-F411RE has no Wi-Fi. An ESP32-S3 board wired to its USART1 carries
its UART lines to and from MQTT over Wi-Fi/TLS.

- **It is the only link**, locally and in the cloud. Only the broker it points
  at changes.
- **It runs the `uart-mqtt-bridge` firmware**, a separate general-purpose
  project in its own repository. This repository holds only the bridge's
  config (the routing table) and secrets headers, and names the bridge version
  it was tested with.
- **It routes each line by its first letter** and never changes a line. It
  also sends the time down to the Nucleo, which it gets from NTP.
- The Nucleo never drives Wi-Fi, MQTT or certificates itself.
- The bridge and the Nucleo act as **one device**: the bridge holds no health
  logic of its own.

## Server stack

```
 broker ─► ingest ─► InfluxDB ─► Grafana
```

Three containers in Docker Compose. Details in [server.md](server.md).

## Local vs cloud

| Piece | Local VM | EC2 |
|-------|----------|-----|
| Server | Multipass Ubuntu 24.04 VM | EC2, same OS |
| MQTT broker | Mosquitto container (TLS + client certs) | AWS IoT Core |
| Nucleo → broker link | Wi-Fi bridge | Wi-Fi bridge |
| Ansible connection | SSH | SSM, no port 22 |
| Images | Built locally | Pulled from ECR (pushed by CI) |
| Grafana access | VM's address, port 3000 | SSM port forwarding to `localhost:3000` |

- **Same topics and TLS client certificates** on both brokers; switching is
  config only.
- **The VM mirrors EC2** — same OS, managed over the network the same way.
- **One config per environment** picks the broker, certificates and image
  source; the same playbook deploys to either.
- **Local is the primary environment.** The cloud is a proof deployment showing
  the same stack runs on AWS; day-to-day running stays on the VM.
- **Local first, cloud last.** Everything is built and tested locally before
  the first cloud deploy.
- **One Ubuntu development machine** builds the firmware, hosts the VM and
  runs Ansible.

## See also

[protocol.md](protocol.md) · [firmware.md](firmware.md) · [server.md](server.md) · [deployment.md](deployment.md)
