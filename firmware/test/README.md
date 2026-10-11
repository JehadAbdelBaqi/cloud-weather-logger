# Unit tests

Tests for the firmware code that has no hardware in it. They are built for the
PC and run on it, with no board plugged in.

[← README](../../README.md)

## Running them

From `firmware/`:

```
pio test -e native
```

They also run as the first step of `scripts/build.sh`, before either board is
built.

One test folder alone:

```
pio test -e native -f link/test_link_framing
```

## What is tested

The libraries' files of the kind `<module>_<job>.c`: one job, no hardware
(see [firmware.md](../../docs/system/firmware.md)), and the application's
files with no hardware in them. There is one test folder for each, under a
folder named after its library, or `app/` for the application.

```
test/
  app/      test_message/              the handshake message, a stored reading as the reading message
            test_incoming/             reading a response, setting the state from a status, removing
                                       an answered reading
            test_reading_store/        holding readings until they are removed: oldest first,
                                       sequence numbers, a full store
  bme280/   test_bme280_raw/           measurement bytes to raw values
            test_bme280_calibration/   calibration bytes to calibration values
            test_bme280_compensation/  raw values to real units
  format/   test_format_number/        a number to decimal or hexadecimal text, and decimal text to
                                       a number
  link/     test_link_framing/         received bytes to a line
            test_link_buffer/          holding received bytes until they are read
  message/  test_message_build/        constructing a message from its letter and fields
            test_message_fields/       filling a message's fields in, getting a field out of a message
  time/     test_time_interval/        whether an interval has passed
```

## What is not tested here

| Code | Why |
|------|-----|
| Drivers (`<module>_driver.c`) | They operate a device, so they only run on the board |
| `link.c`, `debug.c` | They sit on a driver |
| The rest of the application (`src/`) | It calls the HAL, the link, the sensor and the LED directly |

## How it works

- **Unity** is the test framework. PlatformIO fetches it and runs the tests.
- **Each test folder is one small program.** PlatformIO builds it with the
  PC's compiler, runs it and reads the result of each test from its output.
- **The `native` environment in `platformio.ini` lists the files to compile.**
  Only those files are built for the PC. Nothing else under `lib/` or `src/`
  is, because a driver includes the HAL, which only builds for the
  microcontroller.
- **A test file has the same shape every time:** `setUp()` runs before each
  test, one function per case, and `main()` lists the cases to run.

## Adding a test

1. The code to test goes in a file of its own with no hardware in it:
   `lib/<module>/<module>_<job>.c` for a library, or a file under `src/app/`
   for the application.
2. In `platformio.ini`, under `[env:native]`: add the file to
   `build_src_filter`, and the folder its header is in to `build_flags` if it
   is not there yet.
3. Add `test/<module>/test_<module>_<job>/test_<module>_<job>.c`, with `app`
   as the module for an application file.
4. Run `pio test -e native`.
