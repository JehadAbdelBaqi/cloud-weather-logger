#!/usr/bin/env bash
set -euo pipefail

# Runs the steps switched on in build-config.sh, in order. Run from scripts/.

source build-config.sh

if [ "$#" -gt 0 ]; then
    echo "build.sh takes no options: set the steps to run in build-config.sh." >&2
    exit 1
fi

# Each entry is a switch, then the step it runs
for step in "${PROJECT_STEPS[@]}"; do
    run="${step%% *}"
    script="${step#* }"

    if [ "$run" = 1 ]; then
        # shellcheck disable=SC2086  # a step's options are separate words
        bash $script
    fi
done

echo "Done."
