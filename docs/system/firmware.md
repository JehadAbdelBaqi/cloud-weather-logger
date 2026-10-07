# Firmware

The Nucleo-F411RE firmware: written on ST's STM32Cube HAL, built with
PlatformIO (`framework = stm32cube`).

[← README](../../README.md)

## Peripherals

| Peripheral | Device | Use |
|------------|--------|-----|
| USART1 (PA9 TX / PA10 RX) | Wi-Fi bridge | Protocol lines |
| USART2 (via ST-LINK USB) | PC serial monitor | Debug output only |
| I2C | BME280 | Temperature / pressure / humidity |
| SPI | 3.5" ILI9488 screen | On-device display |
| RTC on the LSE crystal | — | Timestamps |

## Code layout

```
firmware/
  src/main.c     set-up, then the main loop
  src/app/       the application: what this program does with the libraries
  src/app/messaging/   the lines to and from the bridge
  include/config/      the project's values: no code behind them
  include/app/         the headers of the files in src/app/, folder for folder
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
| `config.h` | Tunable timings: the handshake's three values, the LED's blink times, how often the sensor is read and the limits a `rate` command may set |
| `protocol.h` | The letter of each line type, the field separator, the command words and the fixed words |
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
| The three handshake timings | `config.h`, under the same names the bridge reads |
| `LOG_LINES` | Set in `bridge_config.h`; it is the bridge's own |

- **Both boards are built with the same link settings and handshake timings**,
  so the two ends cannot drift apart. What each handshake timing does, and
  what changing it means, is in [protocol.md](protocol.md#h--handshake).
- **The `link` library keeps a line limit of its own**, because a library does
  not read the project's files. The build stops if it differs from
  `LINK_LINE_LIMIT`.
- **`board.h` is not one of these.** It declares the two set-up functions in
  `board.c`, and is in `include/app/` with the other headers.

### Messaging

Everything about the lines exchanged with the bridge is in `src/app/messaging/`.
Each line type has one handler, and the handlers are built from small shared
functions.

| File | Job | Needs the board |
|------|-----|-----------------|
| `router.c` | Takes each line that arrives and hands it to the handler for its first letter | Yes |
| `message_handlers.c` | One complete handler for each line type: the handshake, a command, a reading | Yes |
| `commands.c` | The table of commands, and what each one does and replies | Yes |
| `message_helpers.c` | What the handlers share that touches the hardware: sending a line of one word, sending a reply, the time for a reading | Yes |
| `line.c` | Text work on a line: start it, add a field, find or copy a field, check its word, read a number | No, it builds on a PC as it stands |
| `polling.c` | Asks the bridge whether it is there, on a timer, and notices when it stops answering | Yes |
| `bridge_connection.c` | Whether the connection to the bridge is made | Yes |

The letters of the line types and the fixed words are defined once, in
`include/config/protocol.h`, and the bridge's routing table is built from the
same letters (see [Config](#config)).

### Libraries

| Folder | Job |
|--------|-----|
| `gpio/` | Turns on a port's clock, sets up pins |
| `uart/` | Sets up a UART, sends and receives bytes |
| `i2c/` | Sets up an I2C bus, reads and writes a device's registers |
| `led/` | Sets up and toggles an LED |
| `clock/` | Sets the system clock |
| `bme280/` | The sensor: measurements, calibration values, raw values to real units |
| `link/` | Text lines to and from the bridge, over a UART |
| `debug/` | Text to the PC's serial monitor, over a UART |
| `format/` | Numbers to text |
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
            link_framing.c          collects received bytes into a line
  debug/    debug.c                 writes text and numbers to the debug UART
  format/   format_number.c         a number to decimal or hexadecimal text
  time/     time_interval.c         whether an interval has passed since an earlier moment
```

## Drivers

- Peripherals are driven through the STM32Cube HAL.
- Clock, pin and peripheral setup is written by hand with HAL calls. No code is
  generated by STM32CubeMX.
- A small job may write a register directly, through the CMSIS device header
  that ships with the HAL.

## UART

Polled: readings go out on a timer, and a polled line parser handles incoming
lines (commands, acks, time). No receive interrupt or ring buffer.

Protocol lines always run on USART1, to the bridge. USART2 carries debug output
only, so the Nucleo can be watched in a serial monitor while the link runs.

## Store-and-forward

```
 sample ─► keep in memory ─► send W line ─► ... ─► A,<seq> arrives
                                                     │
                       readings up to seq are acked ◄┘
                       their memory is freed
                       anything unacked is resent
```

- Every reading is kept in the Nucleo's memory until its `seq` is acknowledged.
- The ack is sent only after the reading is written to InfluxDB.
- Unacked readings are resent.
- When the memory is full, the Nucleo stops taking readings until there is
  room again.
- Readings held in memory are lost if the Nucleo restarts or loses power.

## Sequence numbers

`seq` counts up from the start each time the Nucleo is switched on. It is not
kept across a restart.

## Time

- Readings are timestamped on the Nucleo by its RTC, driven by the on-board
  32.768 kHz LSE crystal (X2 on the MB1136 C-04 board).
- The bridge sends the real time down the UART (`T` line) on connect and hourly.
- No readings are taken after a power-up until the first `T` has set the clock.
- Readings resent from memory carry their original sample time.

## Screen

A 3.5" ILI9488 on the SPI bus, with its own chip select, DC and reset lines.
Touch is not used.

| Shows |
|-------|
| Latest temperature / pressure / humidity |
| RTC time |
| Link status |
| Backlog (readings held, not yet acked) |

## Commands

`help`, `read`, `rate <s>` — arriving as `C` lines from the server, through the
broker and the bridge, and each answered with an `R` line. Formats and replies
in [protocol.md](protocol.md#c--r--commands-and-replies).

```
C line arrives
   │
router            finds the handler for the letter C
   │
handle_command    takes out the ID and the command; a line with no ID is ignored
   │
commands_run      finds the command's word in a table and runs its function
   │
send_reply        R,<id>,<result>
```

- **The commands are a table** of a word and the function that carries it out,
  in `commands.c`. A new command is one function and one row.
- **`read` uses the same code as a timed reading**, so what it sends is a
  normal `W` line with the next sequence number.
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
  | A reading arrives on the readings topic within 60 s | The sensor is read, the handshake with the bridge is made, and a line gets all the way up |
  | `help`, `read` and an unknown command each get the reply expected | A command gets down to the Nucleo and its reply gets back up, with the same ID |
  | `rate` with no value, and `rate 0`, are refused with their own replies | The checks on a command's value |
  | After `rate 5`, two readings in a row are at least 4 s apart | The command really changes how often readings are taken |

  It ends by setting the rate back to 2 s. Each of the three groups of checks
  is a line in the report that `build.sh` prints when it ends: one line for
  each step that ran, on both boards, with `success`, `FAILED` or `not run`. The topics are read from
  `firmware/include/config/topic_routes.h`, and the checks themselves are the
  bridge's own test helpers, used with this project's topics.
- **Hardware-in-the-loop test in CI** — see [deployment.md](deployment.md).

## See also

[protocol.md](protocol.md) · [architecture.md](architecture.md) · [deployment.md](deployment.md)
