# Builds a PlatformIO project's firmware and uploads it to its board. Stops if either fails.
# Arguments: <platformio command> <project folder>
build_and_upload_firmware() {
    local platformio="$1"
    local project_dir="$2"

    echo "Building the firmware in $project_dir and uploading it to the board..."
    "$platformio" run --project-dir "$project_dir" --target upload
    echo "Firmware uploaded."
}

# Runs a PlatformIO project's unit tests on the PC. Stops if any of them fails.
# Arguments: <platformio command> <project folder>
run_unit_tests() {
    local platformio="$1"
    local project_dir="$2"

    echo "Running the unit tests in $project_dir on the PC..."
    "$platformio" test --project-dir "$project_dir" --environment native
    echo "Unit tests passed."
}
