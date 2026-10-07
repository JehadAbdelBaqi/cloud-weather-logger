#!/usr/bin/env bash
set -euo pipefail

# Builds and uploads the Wi-Fi bridge with this project's config, by running the bridge's own
# scripts. The Genesis Mini must be plugged in, with no serial monitor holding its port.

source helpers/bridge-helpers.sh

# The bridge's repository sits beside this one
BRIDGE_REPO="../../uart-mqtt-bridge"

build_and_upload_bridge "$BRIDGE_REPO" "bridge-config.sh" "../firmware/include/config/bridge_config.h" "build-config.sh"
