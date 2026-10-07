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

# 2. Where the broker is, and the topics
find_lan_address "$LAN_ADAPTER"
READINGS_TOPIC="$(topic_from_header "$TOPICS_HEADER" TOPIC_READINGS)"
REPLIES_TOPIC="$(topic_from_header "$TOPICS_HEADER" TOPIC_REPLIES)"
COMMANDS_TOPIC="$(topic_from_header "$TOPICS_HEADER" TOPIC_COMMANDS)"

# 3. Readings: the sensor, the handshake and the way up
# shellcheck disable=SC2153  # PROJECT_NAME and BROKER_PORT come from the bridge's settings
report_run "test: readings arrive" check_reading_arrives "$PROJECT_NAME" "$LAN_ADDRESS" "$BROKER_PORT" "$READINGS_TOPIC" 60

# 4. Commands and their replies: the way down and back up
report_run "test: commands answered" check_commands_answered "$PROJECT_NAME" "$LAN_ADDRESS" "$BROKER_PORT" "$COMMANDS_TOPIC" "$REPLIES_TOPIC"

# 5. The rate command changes how often readings arrive
report_run "test: rate changes the timing" check_rate_changes_timing "$PROJECT_NAME" "$LAN_ADDRESS" "$BROKER_PORT" "$COMMANDS_TOPIC" "$REPLIES_TOPIC" "$READINGS_TOPIC"

echo "Nucleo test passed."
