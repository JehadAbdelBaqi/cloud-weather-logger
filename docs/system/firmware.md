# Firmware

The Nucleo-F411RE firmware: written on ST's STM32Cube HAL, built with
PlatformIO (`framework = stm32cube`).

[← README](../../README.md)

> **Built in `v0.1.0`:** everything on this page, apart from the real time on a
> reading and the test in CI, which are marked as planned.

## Peripherals

| Peripheral | Device | Use |
|------------|--------|-----|
| USART1 (PA9 TX / PA10 RX) | Wi-Fi bridge | Protocol lines |
| USART2 (via ST-LINK USB) | PC serial monitor | Debug output only |
| I2C | BME280 | Temperature / pressure / humidity |
| RTC on the LSE crystal | — | Timestamps. *Planned, not built* |

## Code layout

```
firmware/
  src/main.c     set-up, then the main loop
  src/app/       the application: what this program does with the libraries
  src/app/messaging/   this project's messages
  src/app/operations/  what is sent and when, and what is done with what arrives
  src/app/stores/      what is held in memory until it is sent and acknowledged
  include/config/      the project's values: no code behind them
  include/app/         the headers of the files in src/app/
  lib/           the libraries, one folder per module
```

- **A library knows nothing about this program.** Nothing under `lib/` includes
  a file from `src/` or `include/`, and each library is given its pins when it
  is set up.
- **The application uses the libraries.** It decides when a reading is taken,
  what is sent and what the LED shows.

### Config

`include/config/` holds the project's values. Each file is `#define`s only and
includes no code, so the bridge's build can read them as well as the Nucleo's.

| File | Holds |
|------|-------|
| `board_config.h` | How the board is wired: every pin assignment, and the baud rate and line limit of the link to the bridge |
| `config.h` | Tunable values: the link's quiet limit, the margin the bridge answers within and the margin after which it counts the Nucleo as gone, the LED's blink times, how often the sensor is read, the limits a `rate` command may set and how many readings and replies the stores hold. These are choices, changed here at any time; the docs name them and do not repeat their values |
| `protocol.h` | The letter of each line type, the field separator, the number of each command and the fixed words |
| `topic_routes.h` | The MQTT topic for each line type that goes through the broker |
| `bridge_config.h` | What is handed to the bridge's build |

- **A value is defined once.** A file that needs one defined elsewhere takes it
  from there.
- **`bridge_config.h` defines almost nothing itself.** It gives the bridge the
  names the bridge looks for, each filled from the project's own definition:

| The bridge's setting | Comes from |
|----------------------|------------|
| `UART_BAUD`, `LINE_MAX_LEN` | `LINK_BAUD` and `LINK_LINE_LIMIT` in `board_config.h` |
| `UPLINK_ROUTES` | The letters in `protocol.h`, paired with the topics in `topic_routes.h` |
| `DOWNLINK_TOPICS` | The topics in `topic_routes.h` |
| The line ending, the answer's form and the text of each status | Set in `bridge_config.h`, from the separator and the words in `protocol.h` |
| How long the bridge waits for the broker, and how long before it counts the Nucleo as quiet | Worked out in `bridge_config.h` from the timings in `config.h` |
| `LOG_LINES` | Set in `bridge_config.h`; it is the bridge's own |

- **Both boards are built from the same link settings and timings**, so the
  two ends cannot drift apart. What each timing does, and what changing it
  means, is in [protocol.md](protocol.md#timings).
- **The `link` library keeps a line limit of its own**, because a library does
  not read the project's files. The build stops if it differs from
  `LINK_LINE_LIMIT`.
- **`board.h` is not one of these.** It declares the two set-up functions in
  `board.c`, and is in `include/app/` with the other headers.

### Messaging

`src/app/messaging/` holds this project's messages. Building a message, and
getting a field out of one, is the `message` library's job
(see [Libraries](#libraries)), which is handed the field separator. Neither has
hardware or timing in it, so all of it builds and is tested on the PC.

| File | Job |
|------|-----|
| `messages.c` | The description of each type of message. The handshake's field is written in; a reading's fields are filled in each time one is sent |
| `reading_to_message.c` | A stored reading as the reading message: the one place that knows both. Also the one place a reading's values are listed, as text, for a reading message and for the reply to a `read` command |

Every field is text by the time a message is built. A number is turned into
text first, as a signed 32-bit value, so the time field cannot go past
19 January 2038.

### Operations

`src/app/operations/` decides what is sent and when, and handles what arrives.

| File | Job | Needs the board |
|------|-----|-----------------|
| `run_pass.c` | The function that is one pass of the main loop | Yes |
| `message_handlers.c` | The handler for each kind of message, going out and coming in | Yes |
| `outgoing.c` | Sends a message: builds it, sends it, and marks it as waiting for its answer | Yes |
| `incoming.c` | The steps for a message that has arrived: read a response or a command into its parts, set the state from a status, remove an answered reading or reply | No, it builds on a PC as it stands |
| `routing.c` | The tables: which function a letter, a state or a command's number leads to | Yes |
| `commands.c` | The function for each command, and the one that runs a command from its number | Yes |
| `poll_uart.c` | Polls the UART: routes a message that has arrived, or gives up on a wait | Yes |
| `states.c` | One function for each state, and the function that runs the current one | Yes |
| `status_led.c` | Blinks the LED to show the state | Yes |

The headers of these two folders are in `include/app/messaging/` and
`include/app/operations/`.

**What lasts from pass to pass** is one struct, `app_state_t` in
`include/app/operations/app_state.h`: the state, the awaiting flag, two
times, and the sensor's own two (when it was last read and how often it is
read, a struct of their own from `sensor_timer.h`). It is declared once, in
`main.c`, and handed to every function that needs it, through the route
tables too. No file in `operations/` keeps any of
it for itself.

**One pass of the main loop** is one function, `run_pass()`, which `main`
calls for ever. It is handed the struct and the time, and reads no clock:

```
run_pass(state, time)
   │
   ├─ take a reading if one is due, and put it in the store      every state
   ├─ poll the UART: route a message, or give up on a wait       every state
   ├─ run the function of the state the Nucleo is in
   └─ blink the LED: slowly in the ok state, quickly otherwise
```

**Two states**, each a function in `states.c`:

| State | What its function does | Entered when |
|-------|------------------------|--------------|
| No response | Sends the handshake | At start-up; when the quiet limit passes; when a response has a status other than `ok` |
| Ok | Sends the oldest reply in its store, if there is one; otherwise the oldest reading | A response has the status `ok` |

**One message is out at a time, whatever its kind.** There is one awaiting
flag, set when a message goes out and cleared when its response arrives. While
it is set nothing is sent, a handshake, a reading or a reply, and no work is
done to prepare one. The bridge holds one message while it waits for the
broker, so a second one sent in that time would cost the first its answer.

**A message that arrives is routed through tables**, each indexed by the
letter that picks the route:

```
message arrives
   │
first letter      L  ──► a response        C  ──► a command
   │
second letter     H  ──► the handshake's response
                  W  ──► a reading's response
                  R  ──► a reply's response
```

A letter with no route is ignored. Each response handler reads the response
into its three parts, which clears the awaiting flag, and sets the state from
the status. The reading's handler then
removes that reading from the store if the state is ok.

**The quiet limit** is checked where the UART is polled. When it has passed
since the last message was sent, the Nucleo goes to the no-response state and
the awaiting flag is cleared, so the handshake goes out, and after it the
reading that was held. This covers both a message that got no answer and a
link that has simply gone quiet.

The letters of the message types and the fixed words are defined once, in
`include/config/protocol.h`, and the bridge's routing table is built from the
same letters (see [Config](#config)).

### Libraries

| Folder | Job |
|--------|-----|
| `gpio/` | Turns on a port's clock, sets up pins |
| `uart/` | Sets up a UART, sends bytes, and takes each received byte in the UART's interrupt |
| `i2c/` | Sets up an I2C bus, reads and writes a device's registers |
| `led/` | Sets up and toggles an LED |
| `clock/` | Sets the system clock |
| `bme280/` | The sensor: measurements, calibration values, raw values to real units |
| `link/` | Text lines to and from the bridge, over a UART |
| `debug/` | Text to the PC's serial monitor, over a UART |
| `format/` | Numbers to text, and text to a number |
| `message/` | A message as text: building one from its letter and fields, and getting a field out of one |
| `ring/` | The places of a ring: which place of an array holds the oldest item and which is free |
| `time/` | Whether an interval has passed |

### File names in `lib/`

A file holds one kind of code, and its name says which. A header has the name
of its `.c`.

| Name | Kind | Needs the board |
|------|------|-----------------|
| `<module>_driver.c` | **Driver.** Operates a device: a peripheral of the microcontroller through the HAL, or a part wired to it through another driver | Yes |
| `<module>_<job>.c` | **One job, no hardware.** Maths, joining bytes into values, finding the end of a line, numbers to text, comparing times. Named after the job | No, it builds on a PC as it stands |
| `<module>.c` | **Sits on a driver.** Uses a driver to offer something that is not a device: lines over a UART, text on the debug output | Yes |

```
lib/
  gpio/     gpio_driver.c           port clocks and pin set-up, used by the drivers below
  uart/     uart_driver.c           the UART peripheral
  i2c/      i2c_driver.c            the I2C peripheral
  led/      led_driver.c            the LED's pin
  clock/    clock_driver.c          the system clock
  bme280/   bme280_driver.c         talks to the sensor over I2C
            bme280_raw.c            a measurement's bytes to raw values
            bme280_calibration.c    the calibration bytes to calibration values
            bme280_compensation.c   raw values to real units
  link/     link.c                  sends and receives lines over the UART
            link_buffer.c           holds received bytes until they are read
            link_framing.c          collects received bytes into a line
  debug/    debug.c                 writes text and numbers to the debug UART
  format/   format_number.c         a number to decimal or hexadecimal text, and decimal text to a number
  message/  message_build.c         a message as text, from its letter and fields
            message_fields.c        filling a message's fields in, and getting a field out of a message
  ring/     ring_places.c           which place of an array holds the oldest item, and which is free
  time/     time_interval.c         whether an interval has passed since an earlier moment
```

## Drivers

- Peripherals are driven through the STM32Cube HAL.
- Clock, pin and peripheral setup is written by hand with HAL calls. No code is
  generated by STM32CubeMX.
- A small job may write a register directly, through the CMSIS device header
  that ships with the HAL.

## UART

Received by interrupt. Each byte from the bridge is taken the moment it
arrives and put in a buffer in memory; the main loop takes whole lines out of
the buffer when it gets there.

```
bridge ──► USART1 ──► interrupt ──► buffer ──► main loop ──► a line
           holds one byte            512 places
```

- **The UART holds one received byte.** Without the buffer, a byte arriving
  while the main loop was busy, reading the sensor for instance, would be lost.
- **The buffer has room for more than arrives during the longest pass** of the
  main loop.
- **The interrupt only puts bytes in and the main loop only takes them out**,
  so neither has to stop the other.

Protocol lines always run on USART1, to the bridge. USART2 carries debug output
only, so the Nucleo can be watched in a serial monitor while the link runs.

## Readings held and sent again

A reading is never sent straight from the sensor. It goes into a store in the
Nucleo's memory, and is sent from there. It stays in the store until it is
acknowledged.

```
 reader (on a timer)                 sender (driven by the connection)
 sensor ──► store in memory ──► oldest reading ──► bridge ──► broker
                  ▲                                              │
                  └────── acknowledged: removed from the store ◄─┘
```

Two parts, which do not depend on each other:

| Part | Runs | Does |
|------|------|------|
| **Reader** | On a timer, whatever the state of the connection | Takes a reading, gives it its sequence number and its time, puts it in the store |
| **Sender** | In the ok state; it has no timer of its own | Takes the oldest reading in the store, sends it, waits for its acknowledgement, then moves to the next |

**The sender, step by step**

1. Send the oldest reading in the store.
2. Wait for its acknowledgement. The next reading is not sent until it comes.
3. Acknowledged: the reading is removed from the store. Back to step 1.

One reading at a time, in the order they were taken. A reading sent again
keeps the sequence number and the time it was given when it was taken.

**What stops the sender, and what starts it again**

| Situation | How the Nucleo knows | Sender |
|-----------|----------------------|--------|
| The bridge does not answer | The quiet limit passes with no response | Stops; the Nucleo sends the handshake |
| The bridge cannot pass the reading on (no Wi-Fi, no broker, or the broker did not confirm it) | The response has a status other than `ok` | Stops; the Nucleo sends the handshake |
| The path is back | The handshake's response has the status `ok` | Carries on, from the reading it was on |

A reading is never sent again on a timer. When its answer does not come, or
comes back as anything but `ok`, the reading stays in the store and the Nucleo
asks the bridge whether it is there; the reading goes again once the answer to
that is `ok`.

Meanwhile the reader carries on: readings taken during an outage queue up in
the store and go out, oldest first, once the sender starts again.

**The store**

| | |
|---|---|
| Where | The Nucleo's memory (RAM), about 50 KB set aside |
| One reading | 20 bytes: sequence number, time, temperature, pressure, humidity |
| Holds | About 2,500 readings. How long that covers is 2,500 times the reading interval |
| When full | No readings are taken until there is room again. Nothing already held is thrown away |
| On a restart or a power cut | Everything in it is lost |

Replies to commands have a store of their own, kept the same way: oldest out
first, each staying until the bridge answers `ok` for it. It is small, a
handful of replies, each the ID of its command and the result as text; how
many it holds is set in `config.h`. Both stores are in `src/app/stores/`, and
both keep their places with the `ring` library.

**Who acknowledges**

| Acknowledgement | From | Means |
|-----------------|------|-------|
| `L,W,<seq>,ok` | The bridge, once the broker confirms the reading | The broker has the reading. This is what removes it from the store |
| The server's response | The server, once the reading is written to the database | The reading is stored. Comes with the server stack |

**The same reading can arrive twice.** If a reading got through and its
acknowledgement was lost, it is sent again. Whatever takes the readings in has
to treat one it already has as a repeat, not as a new reading.

**Still to settle**

| Open point | Note |
|------------|------|
| Telling a repeat from a new reading after a restart | `seq` starts again at every power-on, so the same number can be a different reading. The time on the reading tells them apart, once it is the real time |

## Sequence numbers

`seq` counts up from the start each time the Nucleo is switched on. It is not
kept across a restart. While the connection is down it carries on counting:
readings are held in the store with the numbers they were given, and nothing
is renumbered when they are sent later.

## Time

**In `v0.1.0` the time on a reading is a stand-in:** a fixed starting time,
`READING_TIME_START` in `config.h`, plus the seconds since the Nucleo was
switched on. It counts up one second per second and starts again at every
power-on. A reading is given its time when it is taken, so readings sent late
or sent again keep their order and their spacing.

*Planned, not built:*

- Readings are timestamped on the Nucleo by its RTC, driven by the on-board
  32.768 kHz LSE crystal (X2 on the MB1136 C-04 board).
- The real time is sent down to the Nucleo to set that clock.
- No readings are taken after a power-up until the clock has been set.

## Commands

`help`, `read` and `rate` — arriving as `C` lines through the broker and the
bridge, and each answered with an `R` line. In `v0.1.0` they are published to
the broker by hand or by the tests; the server that will send them is planned. Formats and replies
in [protocol.md](protocol.md#c--r--commands-and-replies).

```
C,<id>,<number>,<value> arrives
   │
   ├─ read it into its ID, its number and its value
   ├─ run it: the number is the position in the table of commands, and the
   │  function there carries the command out and writes the result
   └─ put the ID and the result in the reply store
```

- **A command is a number**, and the commands are a table in `routing.c` with
  the function for each at that number's position. A new command is one
  function, one row and one number in `protocol.h`.
- **A reply waits in a store of its own** until it can be sent, because one
  message is out at a time. In the ok state a waiting reply goes ahead of the
  readings that are waiting: someone is waiting for it. It stays in the store
  until the bridge answers `ok` for it.
- **The reply is the answer itself.** There is no separate line to say a
  command arrived. `help` replies with the list, `rate` with `ok`, and `read`
  with the reading.
- **`read` takes a fresh reading and sends it as the reply**, with the
  command's ID in place of a sequence number. It is not put in the reading
  store.
- **`rate` changes the interval in memory**, within the limits in `config.h`.
  It is not kept across a restart.

## Testing

- **Unit tests on the PC** — for the code with no hardware in it, in the
  libraries and in the application, run with `pio test -e native` from
  `firmware/`. They are also the first step of `scripts/build.sh`
  (`steps/unit-tests.sh`), so a failing test stops a build before anything is
  uploaded. See
  [firmware/test/README.md](../../firmware/test/README.md).
- **A test of the running boards, through the broker** — `scripts/steps/test-nucleo.sh`,
  the last step of `scripts/build.sh`. With both boards running and the broker
  reachable it checks, in order:

  | Check | Proves |
  |-------|--------|
  | A reading arrives on the readings topic within the time the test allows | The sensor is read, the handshake with the bridge is made, and a line gets all the way up |
  | `help` gets the list of commands, `read` gets a reading, and a command with nothing after its ID or with an unknown number gets its own reply | A command gets down to the Nucleo and its reply gets back up, with the same ID |
  | `rate` with no value, and with one above the limit, are refused with their own replies | The checks on a command's value |
  | After `rate` is given a shorter interval, two readings in a row arrive that far apart; the built-in interval is then put back | The command really changes how often readings are taken |
  | Two readings in a row arrive one reading interval apart, with sequence numbers that go up by one | Readings are taken and sent at the interval the firmware is built with, each one once; and the interval was put back |
  | The broker is stopped for two reading intervals and started again; the readings taken meanwhile then arrive in order | Readings are held while they cannot be delivered, and none is lost or sent twice |

  Each command stops listening as soon as its reply arrives. The test reads the interval,
  and the limit of `rate`, from `config.h`, so it follows whatever is set
  there, and the number of each command from `protocol.h`. Each group of checks
  is a line in the report that `build.sh` prints when it ends: one line for
  each step that ran, on both boards, with `success`, `FAILED` or `not run`. The topics are read from
  `firmware/include/config/topic_routes.h`, and the checks themselves are the
  bridge's own test helpers, used with this project's topics.
- **A test of the connection between the two boards** — `scripts/steps/test-connections.sh`,
  a step of `scripts/build.sh` after the one above. No wire is touched:

  | Check | Proves |
  |-------|--------|
  | The Nucleo is halted through its debugger for longer than the bridge's limit, then restarted. The bridge's log has to say the connection was lost and then made, and a reading has to arrive afterwards | The bridge counts a silent Nucleo as gone, and takes it back when its handshake is answered |
  | The bridge is restarted over USB while readings are collected from before the restart until after it; their sequence numbers have to go up by one | With nothing answering it at all, the Nucleo holds its readings and sends them in order once the bridge is back, none lost and none twice |

  The bridge's limit is worked out from `config.h`, as `bridge_config.h` works
  it out. The bridge's log is read from its USB serial port, found by its name
  under `/dev/serial/by-id`, so both boards have to be plugged into the PC.
- **Hardware-in-the-loop test in CI** — *planned, not built*; see [deployment.md](deployment.md).

## See also

[protocol.md](protocol.md) · [architecture.md](architecture.md) · [deployment.md](deployment.md)
