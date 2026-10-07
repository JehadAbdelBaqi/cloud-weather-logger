# Protocol

The text lines exchanged over UART between the Nucleo and the Wi-Fi bridge.
The bridge routes each line by its first letter.

[← README](../../README.md)

## Line types

| Line | Direction | Meaning |
|------|-----------|---------|
| `W,<seq>,<epoch>,<t_centi_C>,<p_Pa>,<rh_centi_pct>` | up | A reading |
| `R,<id>,<result>` | up | Reply to a command |
| `A,<seq>` | down | Acknowledgement |
| `T,<epoch>` | down | Set the time |
| `C,<id>,<command>` | down | A command |
| `H,request` | both | Handshake: "are you there?" |
| `H,ack` | both | Handshake: "I am here" |

Up = Nucleo → server. Down = server → Nucleo. The `H` lines go no further than
the bridge: they pass between the Nucleo and the bridge only.

## `W` — reading

| Field | Type | Unit / scale |
|-------|------|--------------|
| `seq` | unsigned integer | Sequence number; starts again when the Nucleo is switched off and on |
| `epoch` | unsigned integer | Sample time, Unix time in seconds. A stand-in until the RTC is set up: a fixed starting time plus the seconds since the Nucleo was switched on |
| `t_centi_C` | signed integer | Temperature in hundredths of °C |
| `p_Pa` | unsigned integer | Pressure in pascals |
| `rh_centi_pct` | unsigned integer | Relative humidity in hundredths of % |

```
W,1042,1790812800,2140,101325,4550
        → seq 1042, 21.40 °C, 101325 Pa, 45.50 %
```

All fields are integers. Ingest converts them to decimals before writing to
InfluxDB.

## `A` — acknowledgement

`A,<seq>` means every reading **up to and including** `seq` is stored in
InfluxDB.

- **Cumulative.** One line acknowledges many readings.
- **Gap-free.** Ingest acks only up to the last `seq` with no gap before it; a
  missing reading is resent.
- Sent only after the database write succeeds.

## `T` — time

`T,<epoch>` sets the Nucleo's RTC (Unix time in seconds).

- Sent on every (re)connect, then hourly.
- After a power-up the Nucleo takes **no readings until the first `T`** has set
  its clock.
- Source: the bridge, which gets the time from NTP.

## `C` / `R` — commands and replies

```
C,<id>,<command>      down
R,<id>,ok             up
R,<id>,err <reason>   up
R,<id>,<text>         up
```

The `id` is echoed in the reply, matching it to its command. It is chosen by
whoever sends the command: any text of 1 to 16 characters with no comma in it.

| Command | Does | Reply |
|---------|------|-------|
| `help` | Lists the commands | `R,<id>,help read rate <s>` |
| `read` | Takes a reading now | A normal `W` line, then `R,<id>,ok` |
| `rate <s>` | Sets how often readings are taken, in whole seconds from 1 to 3600 | `R,<id>,ok` |

A reading taken by `read` is stored, acked and graphed like any other. The
timed readings carry on at their own times.

```
C,7,rate 10      →   R,7,ok          a reading every 10 s from now on
C,8,read         →   W,...  then  R,8,ok
C,9,reboot       →   R,9,err unknown command
```

When a command cannot be carried out:

| Reply | When |
|-------|------|
| `R,<id>,err unknown command` | The command is not one of the three. They are written in small letters |
| `R,<id>,err no command` | The line has an ID and nothing after it |
| `R,<id>,err rate needs a number of seconds` | `rate` with no value, or one that is not a whole number |
| `R,<id>,err rate out of range` | `rate` below 1 or above 3600 |
| `R,<id>,err no reading` | `read`, but the sensor could not be read or the reading could not be sent |
| No reply | The line has no ID, or one longer than 16 characters: there is nothing to answer with |

- **The interval set by `rate` lasts until the Nucleo restarts**, when it goes
  back to the 2 s it is built with.
- **A reply is only sent while the connection to the bridge is made**, like a
  reading.

## `H` — handshake

The Nucleo and the bridge are two boards joined by wires. The handshake
establishes that the two hear each other, and keeps checking. It covers the
UART connection only, not Wi-Fi or the broker.

```
H,request      either side: "are you there?"
H,ack          the side that received H,request: "I am here"
```

- **Either side starts.** Each board sends `H,request` when it starts, so a
  restart of either one is covered.
- **Made, on the Nucleo, only by an `H,ack`** from the bridge: the answer to the
  Nucleo's own request, which shows that lines get through in both directions.
- **A request from the bridge** is answered with `H,ack`, and the Nucleo sends
  its own `H,request` straight after it, without waiting for its timer, so the
  connection is made again at once when the bridge restarts.
- **Made, on the bridge,** once it has received `H,ack`, or has received
  `H,request` and answered it.
- **Not connected:** `H,request` is repeated every 2 s, for as long as it takes.
- **Connected:** nothing is sent while lines keep arriving from the other side;
  any line shows it is still there. After 3 s with none, `H,request` is sent
  every 2 s. After 5 in a row go unanswered, the connection counts as lost.
  A lost connection is noticed about 13 s after the last line.
- **Never published.** The bridge handles `H` lines itself; the letter `H` is
  reserved and has no topic.

### Timings

Three values time the handshake. They are set once, in
`firmware/include/config/config.h`, and **both boards are built with them**:
the bridge is handed the same file. Changing one changes it on the Nucleo and
on the bridge, and takes a rebuild and upload of both.

| Setting | Value | What it does | Raising it | Lowering it |
|---------|-------|--------------|------------|-------------|
| `HANDSHAKE_REQUEST_INTERVAL_MS` | 2000 | How often `H,request` is repeated while it goes unanswered | A board that has just started, or come back, takes longer to be found | Found sooner; more lines on the UART while the other board is away |
| `HANDSHAKE_QUIET_LIMIT_MS` | 3000 | Once connected: how long with no line from the other board before it is asked | Fewer requests on a quiet link; a lost board is noticed later | A lost board is noticed sooner; a board with nothing to send is asked more often |
| `HANDSHAKE_MISSED_LIMIT` | 5 | Unanswered requests in a row before the connection counts as lost | Rides out a longer gap without calling it lost; a real loss is noticed later | A loss is noticed sooner; a short gap, such as a board being restarted, may count as a loss |

- **Time to notice a lost board**, counted from its last line: the quiet limit,
  plus the missed limit times the request interval. With the values above that
  is 3 s + 5 × 2 s = 13 s.
- **The Nucleo asks about every 3 s while all is well.** The bridge sends
  nothing down unless it has a line to pass on, so the Nucleo goes without
  hearing from it for the quiet limit and asks. The bridge rarely asks, because
  a reading reaches it every 2 s.
- **Keep the quiet limit above the reading interval** (`SENSOR_READ_INTERVAL_MS`,
  2000). Below it, the bridge would ask between every two readings.

| | Not connected | Connected | Connected, but quiet and being asked |
|---|---|---|---|
| Nucleo's user LED | Toggles every 100 ms | Toggles every 500 ms | Toggles every 500 ms |
| Bridge's LED | Blinks red | Shows Wi-Fi and broker state | Flashes green, 100 ms on / off |

The Nucleo sends nothing but `H,request` until the connection is made.

## UART settings

| Setting | Value |
|---------|-------|
| Baud rate | 115200 (`LINK_BAUD` in `firmware/include/config/board_config.h`) |
| Longest line | 127 characters, not counting the terminator (`LINK_LINE_LIMIT`, same file) |
| Frame | 8 data bits, no parity, 1 stop bit (8N1) |
| Line terminator | `\n` sent; `\n` or `\r\n` accepted |

## MQTT

The bridge routes each line by its first letter. The MQTT payload is the protocol
line as-is, without the terminator. The topics are defined in
`firmware/include/config/topic_routes.h`.

| Line | Direction | Topic |
|------|-----------|-------|
| `W` | up | `weather/nucleo-01/readings` |
| `R` | up | `weather/nucleo-01/replies` |
| `A` | down | `weather/nucleo-01/acks` |
| `C` | down | `weather/nucleo-01/commands` |
| `T` | down | none — generated by the bridge itself |
| `H` | both | none — between the Nucleo and the bridge only |

- Topic shape: `weather/<device>/<type>`; the device ID is `nucleo-01`.
- QoS 1 on all four topics.

## See also

[architecture.md](architecture.md) · [firmware.md](firmware.md) · [server.md](server.md)
