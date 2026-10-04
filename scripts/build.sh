#!/usr/bin/env bash
set -euo pipefail

# Builds and uploads the Wi-Fi bridge with this project's config.
# Run from scripts/. The steps to run are set in build-config.sh.

source helpers/bridge-helpers.sh

# The bridge's repository sits beside this one
BRIDGE_REPO="../../uart-mqtt-bridge"

build_and_upload_bridge "$BRIDGE_REPO" "bridge-config.sh" "../firmware/bridge/bridge-config.h" "build-config.sh"
