#!/usr/bin/env bash
set -euo pipefail

# Runs the steps switched on in build-config.sh, in order, and ends with a short report of them.
# Run from scripts/.

source build-config.sh

# The bridge's repository sits beside this one. The report is its helper, so that the bridge's
# own steps and this project's go into one report.
BRIDGE_REPO="../../uart-mqtt-bridge"

if [ ! -f "$BRIDGE_REPO/scripts/helpers/report.sh" ]; then
    echo "The bridge's scripts were not found in $BRIDGE_REPO/scripts." >&2
    echo "The uart-mqtt-bridge repository has to sit in the same folder as this one." >&2
    exit 1
fi
# shellcheck source=/dev/null
source "$BRIDGE_REPO/scripts/helpers/report.sh"

if [ "$#" -gt 0 ]; then
    echo "build.sh takes no options: set the steps to run in build-config.sh." >&2
    exit 1
fi

# The report is the last thing printed, whether the run finished or failed
report_start
trap report_print EXIT

# Each entry is a switch, then the step it runs. After a step fails, the rest are not run.
FAILED=0
for step in "${PROJECT_STEPS[@]}"; do
    run="${step%% *}"
    script="${step#* }"
    name="$(report_step_name "$script")"

    if [ "$run" != 1 ]; then
        continue
    fi
    if [ "$FAILED" = 1 ]; then
        report_add "$name" "not run"
        continue
    fi

    # shellcheck disable=SC2086  # a step's options are separate words
    report_run_step "$name" bash $script || FAILED=1
done

if [ "$FAILED" = 1 ]; then
    exit 1
fi

echo "Done."
