# Builds a PlatformIO project's firmware and uploads it to its board. Stops if either fails.
# Arguments: <platformio command> <project folder>
build_and_upload_firmware() {
    local platformio="$1"
    local project_dir="$2"

    echo "Building the firmware in $project_dir and uploading it to the board..."
    "$platformio" run --project-dir "$project_dir" --target upload
    echo "Firmware uploaded."
}
