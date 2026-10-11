#!/usr/bin/env bash
set -euo pipefail

# Tests the connection between the two boards: what the bridge does when the Nucleo falls silent
# and when it comes back. Needs both boards running this project's firmware, wired to each other
# and plugged into this PC, and the broker reachable (the bridge's set-port-rule.sh).

source helpers/bridge-helpers.sh
source helpers/nucleo-test.sh
source helpers/connection-test.sh

# The bridge's repository sits beside this one
BRIDGE_REPO="../../uart-mqtt-bridge"

if [ ! -f "$BRIDGE_REPO/scripts/helpers/bridge-test.sh" ]; then
    echo "The bridge's test helpers were not found in $BRIDGE_REPO/scripts." >&2
    echo "The uart-mqtt-bridge repository has to sit in the same folder as this one." >&2
    exit 1
fi

# The debugger PlatformIO installed for uploading to the Nucleo
OPENOCD_FOLDER="$HOME/.platformio/packages/tool-openocd"

# A word in the name of the bridge's serial port under /dev/serial/by-id
BRIDGE_PORT_WORD="Espressif"

# Full paths, because the bridge's scripts are loaded from their own folder
SCRIPT_CONFIG="$(full_path "bridge-config.sh")"
TOPICS_HEADER="$(full_path "../firmware/include/config/topic_routes.h")"
CONFIG_HEADER="$(full_path "../firmware/include/config/config.h")"
export SCRIPT_CONFIG

# 1. The bridge's settings and test helpers, with this project's config
cd "$BRIDGE_REPO/scripts"
# shellcheck source=/dev/null
source helpers/settings.sh
# shellcheck source=/dev/null
source "$NETWORK_HELPERS"
# shellcheck source=/dev/null
source helpers/bridge-test.sh
# shellcheck source=/dev/null
source helpers/report.sh

# 2. Where the broker and the bridge are, the topic, and the values the firmware is built with
find_lan_address "$LAN_ADAPTER"
READINGS_TOPIC="$(topic_from_header "$TOPICS_HEADER" TOPIC_READINGS)"
BRIDGE_PORT="$(serial_port "$BRIDGE_PORT_WORD")"
ACK_QUIET_LIMIT_MS="$(number_from_header "$CONFIG_HEADER" ACK_QUIET_LIMIT_MS)"
MCU_QUIET_BUFFER_MS="$(number_from_header "$CONFIG_HEADER" MCU_QUIET_BUFFER_MS)"

# How long the bridge hears nothing before the Nucleo counts as gone: worked out as
# bridge_config.h works it out
GONE_LIMIT_MS=$((ACK_QUIET_LIMIT_MS + MCU_QUIET_BUFFER_MS))

# 3. The bridge counts a silent Nucleo as gone, and takes it back when it returns
# shellcheck disable=SC2153  # PROJECT_NAME and BROKER_PORT come from the bridge's settings
report_run "test: bridge loses and regains the Nucleo" check_bridge_loses_and_regains_nucleo "$PROJECT_NAME" "$LAN_ADDRESS" "$BROKER_PORT" "$READINGS_TOPIC" "$OPENOCD_FOLDER" "$BRIDGE_PORT" "$GONE_LIMIT_MS"

echo "Connection test passed."
