# Protocol

The text lines exchanged over UART between the Nucleo and the Wi-Fi bridge.
The bridge routes each line by its first letter.

[← README](../../README.md)

## Line types

| Line | Direction | Meaning |
|------|-----------|---------|
| `W,<seq>,<epoch>,<t_centi_C>,<p_Pa>,<rh_centi_pct>` | up | A reading |
| `R,<id>,<result>` | up | Reply to a command |
| `A,<seq>` | down | Acknowledgement from the server: the reading is stored |
| `L,<letter>,<first field>,<status>` | down | A response to a line the Nucleo sent up |
| `T,<epoch>` | down | Set the time |
| `C,<id>,<command>` | down | A command |
| `H,request` | up | Handshake: "are you there?" |

Up = Nucleo → server. Down = server → Nucleo. The `H` line goes no further than
the bridge: it is answered by the bridge itself. The bridge's responses are
written by the bridge too, so neither has a topic.

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

## `L` — response

The bridge answers every line the Nucleo sends up, with one line type:

```
L,<letter>,<first field>,<status>
```

`<letter>` and `<first field>` are those of the line being answered, so the
Nucleo knows exactly which line a response is for.

```
Nucleo ── W,42,... ──► bridge ── publish ──► broker
                       bridge ◄── confirmed ── broker
Nucleo ◄── L,W,42,ok ─ bridge
```

| Line sent up | Its response |
|--------------|--------------|
| `H,request` | `L,H,request,<status>` |
| `W,42,...` | `L,W,42,<status>` |
| `R,7,ok` | `L,R,7,<status>` |

**The status** says what became of the line:

| Status | Means |
|--------|-------|
| `ok` | For a reading or a reply: the broker has the line. For the handshake: the bridge has Wi-Fi and the broker |
| `con_err_w` | The bridge has no Wi-Fi. The line was not published |
| `con_err_br` | The bridge has Wi-Fi, but no connection to the broker. The line was not published |
| `err_br` | The line was published, but the broker did not confirm it in time |
| `ans_err_long` | The line was too long for the bridge to answer in full: the first field is left empty |

- **One response for one line.** The Nucleo sends its next line of that kind
  only once it has the response to the one before.
- **`ok` on a reading removes it from the Nucleo's store.** With any other
  status the reading stays there, and the Nucleo sends the handshake until the
  bridge answers `ok`; the reading then goes again. So a reading can reach the
  broker twice, with the same `seq` and the same time.
- **The bridge always answers, one way or the other**, within a time shorter
  than the Nucleo waits (see [Timings](#timings)). No response at all means the
  bridge, or the wires to it, are gone.
- **The bridge does not read the line.** The broker confirms a message by a
  number the bridge gave it, so the bridge remembers the letter and first
  field of each line it publishes and hands those back.
- **The first field of every line sent up is what names it**: a reading's
  sequence number, and for a reply the ID of the command it answers.

## `A` — acknowledgement

Comes with the server stack. Until then the bridge's response, above, is the
one the Nucleo acts on.


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
| `rate <s>` | Sets how often readings are taken, in whole seconds, within the limits set in the config | `R,<id>,ok` |

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
| `R,<id>,err rate out of range` | `rate` below `SENSOR_READ_INTERVAL_MIN_S` or above `SENSOR_READ_INTERVAL_MAX_S` |
| `R,<id>,err no reading` | `read`, but the sensor could not be read or the reading could not be sent |
| No reply | The line has no ID, or one longer than 16 characters: there is nothing to answer with |

- **The interval set by `rate` lasts until the Nucleo restarts**, when it goes
  back to the interval it is built with, `SENSOR_READ_INTERVAL_MS`.
- **Every time here is a value in `firmware/include/config/config.h`**, not
  part of the protocol: the interval the firmware is built with and the two
  limits of `rate`. They are changed there.
- **A reply is only sent while the connection to the bridge is made**, like a
  reading.

## `H` — handshake

The Nucleo and the bridge are two boards joined by wires. The handshake
establishes that the bridge hears the Nucleo and can answer it, and says how
far up the bridge can reach.

```
H,request                  the Nucleo: "are you there?"
L,H,request,<status>       the bridge: "I am here", and whether it has Wi-Fi and the broker
```

- **Only the Nucleo asks.** The bridge answers and never asks. To the bridge,
  any line from the Nucleo shows that the Nucleo is there.
- **The answer is a response like any other** (see [`L`](#l--response)), and
  its status is `ok`, `con_err_w` or `con_err_br`.
- **`ok` puts the Nucleo in its ok state**, in which it sends its readings.
  With any other status, or with no answer, it stays in its no-response state
  and sends nothing but the handshake.
- **When the Nucleo asks:** when it starts, and whenever the quiet limit has
  passed since it last sent a line. That covers a line that got no answer and
  a link that has gone quiet.
- **Not answered:** the handshake is sent again once every quiet limit, for as
  long as it takes. The Nucleo carries on taking readings and holding them.
- **Never published.** The bridge handles the `H` line itself; the letter `H`
  is reserved and has no topic.

### Timings

The times are config, not part of the design: they are set once, in
`firmware/include/config/config.h`, and are changed there. **Both boards are
built from them**, the bridge's being worked out in `bridge_config.h`.
Changing one takes a rebuild and upload of both boards. This page does not
give their values, so that it cannot disagree with the file.

| Setting | What it does | Raising it | Lowering it |
|---------|--------------|------------|-------------|
| `HANDSHAKE_QUIET_LIMIT_MS` | On the Nucleo: how long since the last line it sent before it gives up on any answer and sends the handshake | A lost bridge is noticed later; fewer handshakes on a quiet link | A lost bridge is noticed sooner; set at or below the reading interval, a handshake goes out between readings on a working link |
| `BRIDGE_ANSWER_BUFFER_MS` | How much sooner than the quiet limit the bridge answers, whatever the broker has done | The bridge gives the broker less time to confirm a line | The bridge's answer arrives closer to the moment the Nucleo gives up |

- **The bridge waits for the broker for the quiet limit less the buffer**, then
  answers `err_br`. Its answer therefore always reaches the Nucleo before the
  Nucleo's own wait runs out. The build stops if the buffer is under a second,
  or not less than the quiet limit.
- **The quiet limit is longer than the reading interval**
  (`SENSOR_READ_INTERVAL_MS`), so on a working link a reading always goes out
  before the limit runs out, and the handshake is sent at start-up and after a
  missed answer only.
- **The bridge counts the Nucleo as quiet** when it has heard nothing from it
  for a time worked out from the same settings, and as gone after as long
  again.

| | No response | Ok |
|---|---|---|
| Nucleo's user LED | Fast blink (`LED_CONNECTING_BLINK_MS`) | Slow blink (`LED_CONNECTED_BLINK_MS`) |

| | Handshake not yet answered | Nucleo heard from | Nucleo quiet | Nucleo gone |
|---|---|---|---|---|
| Bridge's LED | Blinks red | Shows Wi-Fi and broker state | Flashes green, fast | Blinks red |

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
| `H` | up | none — answered by the bridge itself |
| `L` | down | none — written by the bridge itself |

- Topic shape: `weather/<device>/<type>`; the device ID is `nucleo-01`.
- QoS 1 on all four topics.

## See also

[architecture.md](architecture.md) · [firmware.md](firmware.md) · [server.md](server.md)
