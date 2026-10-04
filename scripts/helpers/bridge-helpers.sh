# Prints the full path of a file, given a path relative to the current folder.
# Arguments: <file>
full_path() {
    echo "$(cd "$(dirname "$1")" && pwd)/$(basename "$1")"
}

# Sets the bridge up for this project and builds and uploads its firmware, by running the
# bridge's own e2e.sh with this project's config files.
# Arguments: <bridge repository folder> <script config> <firmware config> <build config>
build_and_upload_bridge() {
    local bridge_repo="$1"
    local script_config
    local firmware_config
    local build_config

    if [ ! -f "$bridge_repo/scripts/e2e.sh" ]; then
        echo "The bridge's scripts were not found in $bridge_repo/scripts." >&2
        echo "The uart-mqtt-bridge repository has to sit in the same folder as this one." >&2
        exit 1
    fi

    # The bridge's scripts run from their own folder, so they need full paths
    script_config="$(full_path "$2")"
    firmware_config="$(full_path "$3")"
    build_config="$(full_path "$4")"

    echo "Building the bridge in $bridge_repo with this project's config..."
    (cd "$bridge_repo/scripts" && SCRIPT_CONFIG="$script_config" FIRMWARE_CONFIG="$firmware_config" BUILD_CONFIG="$build_config" bash e2e.sh)
    echo "Bridge built and uploaded."
}
