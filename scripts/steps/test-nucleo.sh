#!/usr/bin/env bash
set -euo pipefail

# Tests the running Nucleo through the bridge and the broker: readings arrive, and commands are
# answered. Needs both boards running this project's firmware and wired to each other, and the
# broker reachable (the bridge's set-port-rule.sh).

source helpers/bridge-helpers.sh
source helpers/nucleo-test.sh

# The bridge's repository sits beside this one
BRIDGE_REPO="../../uart-mqtt-bridge"

if [ ! -f "$BRIDGE_REPO/scripts/helpers/bridge-test.sh" ]; then
    echo "The bridge's test helpers were not found in $BRIDGE_REPO/scripts." >&2
    echo "The uart-mqtt-bridge repository has to sit in the same folder as this one." >&2
    exit 1
fi

# Full paths, because the bridge's scripts are loaded from their own folder
SCRIPT_CONFIG="$(full_path "bridge-config.sh")"
TOPICS_HEADER="$(full_path "../firmware/include/config/topic_routes.h")"
CONFIG_HEADER="$(full_path "../firmware/include/config/config.h")"
PROTOCOL_HEADER="$(full_path "../firmware/include/config/protocol.h")"
export SCRIPT_CONFIG

# 1. The bridge's settings and test helpers, with this project's config
cd "$BRIDGE_REPO/scripts"
# shellcheck source=/dev/null
source helpers/settings.sh
# shellcheck source=/dev/null
source helpers/network-helpers.sh
# shellcheck source=/dev/null
source helpers/bridge-test.sh
# shellcheck source=/dev/null
source helpers/report.sh

# 2. Where the broker is, the topics, and the values the firmware is built with
find_lan_address
READINGS_TOPIC="$(topic_from_header "$TOPICS_HEADER" TOPIC_READINGS)"
REPLIES_TOPIC="$(topic_from_header "$TOPICS_HEADER" TOPIC_REPLIES)"
COMMANDS_TOPIC="$(topic_from_header "$TOPICS_HEADER" TOPIC_COMMANDS)"
READ_INTERVAL_MS="$(number_from_header "$CONFIG_HEADER" SENSOR_READ_INTERVAL_MS)"
RATE_MAX_S="$(number_from_header "$CONFIG_HEADER" SENSOR_READ_INTERVAL_MAX_S)"
COMMAND_HELP="$(number_from_header "$PROTOCOL_HEADER" PROTOCOL_COMMAND_HELP)"
COMMAND_READ="$(number_from_header "$PROTOCOL_HEADER" PROTOCOL_COMMAND_READ)"
COMMAND_RATE="$(number_from_header "$PROTOCOL_HEADER" PROTOCOL_COMMAND_RATE)"

# The interval the rate check tries, in seconds: shorter than the one the firmware is built with
SHORTER_RATE_S=2

# 3. Readings: the sensor, the handshake and the way up
# shellcheck disable=SC2153  # PROJECT_NAME and BROKER_PORT come from the bridge's settings
report_run "test: readings arrive" check_reading_arrives "$PROJECT_NAME" "$LAN_ADDRESS" "$BROKER_PORT" "$READINGS_TOPIC" 60

# 4. Commands and their replies: the way down and back up. A command is sent as its number
report_run "test: commands answered" check_commands_answered "$PROJECT_NAME" "$LAN_ADDRESS" "$BROKER_PORT" "$COMMANDS_TOPIC" "$REPLIES_TOPIC" "$RATE_MAX_S" "$COMMAND_HELP" "$COMMAND_READ" "$COMMAND_RATE"

# 5. The rate command changes how often readings arrive: a shorter interval, then the built-in one
report_run "test: rate changes the timing" check_rate_changes_timing "$PROJECT_NAME" "$LAN_ADDRESS" "$BROKER_PORT" "$COMMANDS_TOPIC" "$REPLIES_TOPIC" "$READINGS_TOPIC" "$READ_INTERVAL_MS" "$COMMAND_RATE" "$SHORTER_RATE_S"

# 6. Readings arrive at the interval the firmware is built with, each sent once. Coming after
# check 5, it also shows the rate was put back
report_run "test: reading interval and order" check_reading_interval "$PROJECT_NAME" "$LAN_ADDRESS" "$BROKER_PORT" "$READINGS_TOPIC" "$READ_INTERVAL_MS"

# 7. Readings are kept while the broker is stopped, and arrive in order once it is back.
# This stops the broker in the VM for a while, and starts it again
# shellcheck disable=SC2153  # VM_NAME comes from the bridge's settings
report_run "test: kept while broker stopped" check_readings_kept_while_broker_stopped "$PROJECT_NAME" "$LAN_ADDRESS" "$BROKER_PORT" "$READINGS_TOPIC" "$READ_INTERVAL_MS" "$VM_NAME"

echo "Nucleo test passed."
