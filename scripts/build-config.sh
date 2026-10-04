# The steps build.sh runs, in order. 1 runs the step, 0 leaves it out.
# shellcheck disable=SC2034  # the values are used by the bridge's e2e.sh

# The bridge's steps, run by the bridge's own e2e.sh
STEPS=(
    "0 steps/nuke.sh"
    "0 steps/clean-build.sh"
    "1 steps/create-vm.sh"
    "1 steps/setup-certs.sh --keep-alive"
    "1 steps/build-and-upload.sh"
)

# 1 leaves the port rule in place when the run ends; the bridge needs it to reach the broker
KEEP_PORT_RULE=1
