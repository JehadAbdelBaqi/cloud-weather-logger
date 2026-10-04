#!/usr/bin/env bash
set -euo pipefail

# Builds this project's firmware and uploads it to the Nucleo. The Nucleo must be plugged in.

source helpers/firmware-helpers.sh

PLATFORMIO="$HOME/.platformio/penv/bin/platformio"

build_and_upload_firmware "$PLATFORMIO" "../firmware"
