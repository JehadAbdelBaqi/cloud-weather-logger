#!/usr/bin/env bash
set -euo pipefail

# Runs the firmware's unit tests on the PC: the code with no hardware in it. No board is needed.

source helpers/firmware-helpers.sh

PLATFORMIO="$HOME/.platformio/penv/bin/platformio"

run_unit_tests "$PLATFORMIO" "../firmware"
