# Risks and Limits

What can go wrong with the device as it stands, and what is done about each.

[← README](../../README.md)

These are the risks of what is built: the Nucleo, the sensor and the link to
the broker. The server and the cloud are planned, and their risks come with
them.

## The device

| # | Risk / limit | What is done about it | Status |
|---|--------------|-----------------------|--------|
| 1 | **Readings held in memory are lost when the Nucleo restarts or loses power** — the store is RAM | A link or broker outage loses nothing: only a restart of the Nucleo does. Keeping readings across a restart needs storage the board does not have yet | Accepted |
| 2 | **The sequence number starts again at every power-on** — the same number can then be a different reading, so a repeat cannot be told from a new reading across a restart | The time on a reading would tell them apart, once it is the real time | Open |
| 3 | **The time on a reading is a stand-in** — a fixed starting time plus the seconds since power-on, so it starts again with every restart | Readings keep their order and their spacing. The real time, from the board's clock, is planned | Open |
| 4 | **The same reading can arrive at the broker twice** — when its answer is lost on the way back, or the bridge restarts between publishing it and answering, the Nucleo sends it again | Whatever takes the readings in treats one with the same sequence number and time as a repeat | Accepted |
| 5 | **Every answer other than `ok` is treated as no response** — whether the bridge reports no Wi-Fi, no broker or a broker that did not confirm, the Nucleo goes back to the handshake. While the bridge has no Wi-Fi or no broker, each handshake is answered at once and the next goes out straight away, every few milliseconds | Nothing is lost: readings are still taken and held. The traffic is on the wire between the two boards only. A state of its own for each answer is to come | Open |
| 6 | **One message is out at a time** — a reply to a command waits for the message already out to be answered, and a reply store that is full drops the next reply | Replies go ahead of the readings that are waiting, and the store holds more replies than the tests ever queue. A full store should be reported, not passed over | Open |
| 7 | **An answer that arrives after the Nucleo has restarted is acted on** — it sets the state before a handshake has been answered | The window is the bridge's answer time after a restart, and the first reading is not due until after it | Accepted |

## The bench

| # | Risk / limit | What is done about it | Status |
|---|--------------|-----------------------|--------|
| 8 | **A loose jumper wire looks like a firmware fault** — with the wire into the Nucleo's RX loose, the bridge shows solid green and logs its answers while the Nucleo stays on the fast blink | Reseat the wire from the bridge's TX (GPIO7) to the Nucleo's RX (PA10) first. The bridge's log shows whether its answers are going down | Handled |
| 9 | **The Wi-Fi password is in a file on the PC**, `scripts/bridge-config.sh` | The file is not in the repository: `.gitignore` leaves it out, and an example without the password is committed beside it | Handled |
| 10 | **The bridge's firmware holds secrets that can be read from its flash chip** — the Wi-Fi password and the client's private key | Use a test network and test certificates. This is the bridge's own limit, described in its repository | Accepted |

## The tests

| # | Risk / limit | What is done about it | Status |
|---|--------------|-----------------------|--------|
| 11 | **The tests of the running boards need both boards and the PC** — they cannot run on a hosted CI machine | They are the last steps of `scripts/build.sh`, run on the bench. A runner with the boards attached is planned | Accepted |
| 12 | **The check that restarts the bridge can fail on a reading sent twice** — when the restart falls between a reading being published and its answer (see 4) | It needs unlucky timing. Run the check again | Accepted |
| 13 | **Nothing tests a reply going ahead of waiting readings** — it needs a backlog of readings on the board at the moment a command arrives | The order is read from the code. A check is to be worked out | Open |

## See also

[firmware.md](../system/firmware.md) · [protocol.md](../system/protocol.md) · [decision_logs.md](decision_logs.md) · the bridge's own risks, in its repository: [risks.md](https://github.com/JehadAbdelBaqi/uart-mqtt-bridge/blob/v0.2.1/docs/project-design/risks.md)
