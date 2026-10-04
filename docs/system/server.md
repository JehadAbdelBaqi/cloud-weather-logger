# Server

The Linux host (Ubuntu 24.04 — the local VM or EC2) and the Docker stack that
runs on it.

[← README](../../README.md)

## Stack

| Container | Job | Port published |
|-----------|-----|----------------|
| Mosquitto (local only) | MQTT broker, TLS + client certs | 8883 |
| Ingest (Python) | Subscribes to readings, writes to InfluxDB, publishes acks | none |
| InfluxDB | Time-series storage | none |
| Grafana | Dashboard + alerts, provisioned from files | 3000 |

In the cloud, AWS IoT Core replaces the Mosquitto container.

## Ingest

- Connects out to the broker with its own client certificate.
- Converts the protocol's integers to decimals before writing.
- **Two timestamps per reading:** the Nucleo's sample time is the InfluxDB
  point time; the server's arrival time is stored as a field, `received_at`.
- Publishes an ack for the last gap-free `seq`, only after InfluxDB has
  accepted the write.
- A resent reading (same `seq`) is written once.

## InfluxDB

- **Schema:** identifiers as tags (indexed), readings as fields.
- **Downsampling task:** writes hourly averages to a separate bucket, used by
  long-range graphs.
- **Backups:** taken through InfluxDB's API with a token, nightly, by a systemd
  timer.

## Grafana

- Datasource and dashboard provisioned from files.
- **Deadman alert:** fires when no reading has arrived for 5 minutes.

## Accounts

| Account | Used for | Access |
|---------|----------|--------|
| `deploy` | Ansible's login | SSH key only, sudo |
| `weather` | Owns `/srv/weather`, runs the backup | System user, no login, not in the `docker` group |
| `ubuntu` (default) | — | Locked |

Root SSH login and password authentication are disabled.

## Network

- **ufw:** deny incoming; allow 22 (SSH), 3000 (Grafana), 8883 (MQTT/TLS).
- **Published ports:** only Grafana (3000) and Mosquitto (8883). InfluxDB and
  ingest are reachable on the Compose network only.
- **Bind address per environment:** on the VM, Grafana is reachable from
  the development machine; on EC2 it binds to `127.0.0.1` and is reached through the SSM tunnel.

## systemd

| Unit | Job |
|------|-----|
| `weather-stack.service` | Runs `docker compose up`; ordered after `docker.service` and the data-disk mount |
| Backup `.timer` + `.service` | Nightly InfluxDB backup, `Persistent=true` |

## Logs

Containers use Docker's `journald` log driver. Host and container logs are read
with `journalctl`; journald caps the total size.

## Storage

InfluxDB's data lives on a separate data disk — a named volume locally, an EBS
data volume on EC2.

## See also

[architecture.md](architecture.md) · [deployment.md](deployment.md) · [protocol.md](protocol.md)
