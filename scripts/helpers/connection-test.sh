# The checks of the connection between the Nucleo and the bridge, made from the PC with no wire
# touched: by silencing the Nucleo through its debugger, and by restarting the bridge over USB.
# They use check_line_arrives from the bridge's helpers/bridge-test.sh, and collect_readings and
# seqs_go_up_by_one from helpers/nucleo-test.sh.

# Prints the serial port of a board, from the names under /dev/serial/by-id, which stay the same
# whichever order the boards were plugged in.
# Arguments: <a word in the port's name, e.g. Espressif>
serial_port() {
    local word="$1"
    local port

    port=$(find /dev/serial/by-id -name "*$word*" 2> /dev/null | head -n 1)

    if [ -z "$port" ]; then
        echo "No serial port with '$word' in its name was found: is the board plugged in?" >&2
        exit 1
    fi

    echo "$port"
}

# Halts the Nucleo for a given time, then restarts it, through its ST-LINK. While it is halted it
# sends nothing, as if it had lost power; the restart is what power coming back looks like.
# The debugger has to stay connected for the Nucleo to stay halted, so this returns only once the
# Nucleo has been restarted.
# Arguments: <the openocd folder> <how long to halt it for, in milliseconds>
silence_nucleo() {
    local openocd_folder="$1"
    local halted_ms="$2"

    "$openocd_folder/bin/openocd" -s "$openocd_folder/openocd/scripts" -f board/st_nucleo_f4.cfg \
        -c "init; halt; sleep $halted_ms; reset run; shutdown" > /dev/null 2>&1
}

# Stops the script unless the bridge counts the Nucleo as gone when it falls silent, and takes it
# back when it returns: the Nucleo is halted for longer than the bridge's limit and restarted,
# and the bridge's log has to say the connection was lost and then made, with a reading arriving
# on the broker afterwards.
# Arguments: <project name> <address> <port> <readings topic> <the openocd folder>
#            <the bridge's serial port>
#            <how long the bridge hears nothing before the Nucleo counts as gone, in milliseconds>
check_bridge_loses_and_regains_nucleo() {
    local readings_topic="$4"
    local openocd_folder="$5"
    local bridge_port="$6"
    local gone_limit_ms="$7"
    local halted_ms=$((gone_limit_ms + 3000))
    local log
    local reader
    local lost_line
    local made_line
    log=$(mktemp)

    # 1. Read the bridge's log into a file for as long as the check runs
    stty -F "$bridge_port" 115200 raw -echo -hupcl
    cat "$bridge_port" > "$log" &
    reader=$!

    # 2. Halt the Nucleo for longer than the bridge's limit, then restart it
    echo "Halting the Nucleo for $((halted_ms / 1000)) s, then restarting it..."
    if ! silence_nucleo "$openocd_folder" "$halted_ms"; then
        kill "$reader"
        rm -f "$log"
        echo "The Nucleo could not be halted through its debugger." >&2
        exit 1
    fi

    # 3. A reading arriving shows the Nucleo is back and has shaken hands with the bridge
    if ! check_line_arrives "$1" "$2" "$3" "$readings_topic" 30; then
        kill "$reader"
        rm -f "$log"
        echo "No reading arrived after the Nucleo was restarted." >&2
        exit 1
    fi
    kill "$reader"

    # 4. The bridge said the connection was lost, and after that said it was made
    lost_line=$(grep -n "connection to the MCU lost" "$log" | head -n 1 | cut -d: -f1)
    made_line=$(grep -n "connection to the MCU made" "$log" | tail -n 1 | cut -d: -f1)

    if [ -z "$lost_line" ]; then
        echo "The bridge did not count the Nucleo as gone. Its log:" >&2
        cat "$log" >&2
        rm -f "$log"
        exit 1
    fi
    if [ -z "$made_line" ] || [ "$made_line" -lt "$lost_line" ]; then
        echo "The bridge did not take the Nucleo back. Its log:" >&2
        cat "$log" >&2
        rm -f "$log"
        exit 1
    fi

    grep "connection to the MCU" "$log"
    rm -f "$log"
    echo "The bridge counts a silent Nucleo as gone, and takes it back when it returns."
}

# Restarts the bridge over its USB port, with the tool that uploads to it. Nothing is uploaded:
# the tool is asked only to identify the chip, and restarts it when it is done.
# Arguments: <python> <esptool.py> <the bridge's serial port>
restart_bridge() {
    local python="$1"
    local esptool="$2"
    local bridge_port="$3"

    "$python" "$esptool" --chip esp32s3 --port "$bridge_port" --before default_reset --after hard_reset chip_id > /dev/null 2>&1
}

# Stops the script unless the Nucleo keeps its readings while the bridge is gone, and sends them
# when it is back: readings are collected from before the bridge is restarted until after it has
# rejoined Wi-Fi and the broker, and their sequence numbers have to go up by one, with none
# missing and none twice. While the bridge restarts nothing answers the Nucleo at all, which is
# the one case where its wait for an answer runs out.
# Arguments: <project name> <address> <port> <readings topic> <python> <esptool.py>
#            <the bridge's serial port> <interval the firmware is built with, in milliseconds>
check_nucleo_keeps_readings_while_bridge_restarts() {
    local python="$5"
    local esptool="$6"
    local bridge_port="$7"
    local interval_s=$(($8 / 1000))
    local count=4
    local timeout=$((interval_s * count + 45))
    local readings
    readings=$(mktemp)

    # 1. Listen from before the restart, so no reading on either side of it is missed
    echo "Collecting $count readings in a row on $4, across a restart of the bridge..."
    collect_readings "$1" "$2" "$3" "$4" "$count" "$timeout" > "$readings" &
    sleep "$interval_s"

    # 2. Restart the bridge
    echo "Restarting the bridge..."
    if ! restart_bridge "$python" "$esptool" "$bridge_port"; then
        wait || true
        rm -f "$readings"
        echo "The bridge could not be restarted over USB." >&2
        exit 1
    fi

    # 3. Wait for the readings: the ones taken while the bridge was away arrive once it is back
    if ! wait; then
        echo "$count readings did not arrive within $timeout s. What did arrive:" >&2
        cat "$readings" >&2
        rm -f "$readings"
        exit 1
    fi
    cat "$readings"

    # 4. They carry on from one to the next, with none twice and none missing
    if ! seqs_go_up_by_one "any" "$(cat "$readings")"; then
        rm -f "$readings"
        echo "A reading taken while the bridge restarted was lost or sent twice." >&2
        exit 1
    fi

    rm -f "$readings"
    echo "Readings are kept while the bridge restarts, and arrive in order afterwards."
}
