# The checks of the running Nucleo, made through the bridge and the broker.
# They use check_line_arrives and check_exchange from the bridge's helpers/bridge-test.sh.

# Prints the topic a name stands for in the firmware's topic_routes.h, so the topics are written
# in one place.
# Arguments: <topic_routes.h> <name, e.g. TOPIC_READINGS>
topic_from_header() {
    local header="$1"
    local name="$2"
    local topic

    topic=$(grep "#define $name " "$header" | cut -d '"' -f 2)

    if [ -z "$topic" ]; then
        echo "$name was not found in $header." >&2
        exit 1
    fi

    echo "$topic"
}

# Prints the number a name is defined as in the firmware's config.h, so a value the firmware is
# built with is not written a second time in the tests.
# Arguments: <config.h> <name, e.g. SENSOR_READ_INTERVAL_MS>
number_from_header() {
    local header="$1"
    local name="$2"
    local number

    number=$(grep -E "^#define ${name}[[:space:]]" "$header" | awk '{ print $3 }')

    if [ -z "$number" ]; then
        echo "$name was not found in $header." >&2
        exit 1
    fi

    echo "$number"
}

# Stops the script unless a reading arrives from the Nucleo in time. A reading is only sent once
# the handshake with the bridge is made, so this proves the sensor, the handshake and the whole
# way up to the broker.
# Arguments: <project name> <address> <port> <readings topic> <seconds to wait>
check_reading_arrives() {
    if ! check_line_arrives "$1" "$2" "$3" "$4" "$5"; then
        echo "Check that both boards are running, wired to each other, and that the bridge's LED is solid green." >&2
        exit 1
    fi

    echo "Readings arrive."
}

# Stops the script unless a command sent to the Nucleo gets the reply expected.
# The command is given an ID of its own, which the reply has to carry.
# Arguments: <project name> <address> <port> <commands topic> <replies topic> <command> <expected result>
check_command() {
    local commands_topic="$4"
    local replies_topic="$5"
    local command="$6"
    local expected="$7"
    local id="t$RANDOM"

    if ! check_exchange "$1" "$2" "$3" "$commands_topic" "C,$id,$command" "$replies_topic" "R,$id,$expected" 5; then
        echo "The command '$command' was not answered with '$expected'." >&2
        exit 1
    fi

    echo "Command '$command' works."
}

# Stops the script unless two readings in a row arrive at least a given time apart.
# Arguments: <project name> <address> <port> <readings topic> <fewest seconds apart> <seconds to wait>
check_reading_gap() {
    local project_name="$1"
    local address="$2"
    local port="$3"
    local topic="$4"
    local fewest="$5"
    local timeout="$6"
    local cert_dir="$HOME/certs/$project_name"
    local connection=(-h "$address" -p "$port" --cafile "$cert_dir/ca.crt" --cert "$cert_dir/client.crt" --key "$cert_dir/client.key")
    local arrival_times
    local first_at
    local second_at
    local gap

    echo "Timing the gap between two readings on $topic..."

    # The pipe hands over each reading as it arrives, so the clock is read at that moment.
    # mosquitto_sub ends with an error code when its time is up; the check below deals with that
    arrival_times=$(mosquitto_sub "${connection[@]}" -t "$topic" -C 2 -W "$timeout" 2> /dev/null | while read -r _; do date +%s; done | tr '\n' ' ') || true
    read -r first_at second_at <<< "$arrival_times"

    if [ -z "${second_at:-}" ]; then
        echo "Two readings did not arrive within $timeout s." >&2
        exit 1
    fi

    gap=$((second_at - first_at))
    if [ "$gap" -lt "$fewest" ]; then
        echo "The readings were $gap s apart; expected at least $fewest s." >&2
        exit 1
    fi

    echo "Readings are $gap s apart."
}

# Stops the script unless each kind of command gets the reply expected: one that works, one that
# is not known, and two with a value that is refused.
# Arguments: <project name> <address> <port> <commands topic> <replies topic>
#            <longest interval the rate command allows, in seconds>
check_commands_answered() {
    local too_long=$(($6 + 1))

    check_command "$1" "$2" "$3" "$4" "$5" "help" "help read rate <s>"
    check_command "$1" "$2" "$3" "$4" "$5" "read" "ok"
    check_command "$1" "$2" "$3" "$4" "$5" "reboot" "err unknown command"
    check_command "$1" "$2" "$3" "$4" "$5" "rate" "err rate needs a number of seconds"
    check_command "$1" "$2" "$3" "$4" "$5" "rate $too_long" "err rate out of range"
}

# Stops the script unless the rate command changes how often readings arrive: the interval is
# doubled, two readings are timed, and the interval the firmware is built with is put back.
# Arguments: <project name> <address> <port> <commands topic> <replies topic> <readings topic>
#            <interval the firmware is built with, in milliseconds>
check_rate_changes_timing() {
    local built_in_s=$(($7 / 1000))
    local doubled_s=$((built_in_s * 2))

    check_command "$1" "$2" "$3" "$4" "$5" "rate $doubled_s" "ok"
    check_reading_gap "$1" "$2" "$3" "$6" $((doubled_s - 1)) $((doubled_s * 3 + 10))
    check_command "$1" "$2" "$3" "$4" "$5" "rate $built_in_s" "ok"
}

# Prints the sequence number of a reading: the field after the letter in W,<seq>,<time>,...
# Arguments: <reading>
reading_seq() {
    echo "$1" | cut -d, -f2
}

# Collects readings from the readings topic and prints them, one per line.
# Arguments: <project name> <address> <port> <readings topic> <how many> <seconds to wait>
# Returns: 1 if fewer than that arrived in time
collect_readings() {
    local project_name="$1"
    local address="$2"
    local port="$3"
    local topic="$4"
    local count="$5"
    local timeout="$6"
    local cert_dir="$HOME/certs/$project_name"
    local connection=(-h "$address" -p "$port" --cafile "$cert_dir/ca.crt" --cert "$cert_dir/client.crt" --key "$cert_dir/client.key")

    # mosquitto_sub ends with an error code when its time is up before it has them all
    mosquitto_sub "${connection[@]}" -t "$topic" -C "$count" -W "$timeout" 2> /dev/null
}

# Checks that the sequence numbers of readings go up by one from each reading to the next.
# Arguments: <sequence number the first reading must have, or "any"> <the readings, one per line>
# Returns: 1 at the first reading that is out of step, after saying which
seqs_go_up_by_one() {
    local expected="$1"
    local readings="$2"
    local reading
    local seq

    while read -r reading; do
        seq=$(reading_seq "$reading")
        if [ "$expected" != "any" ] && [ "$seq" != "$expected" ]; then
            echo "Reading $seq arrived where reading $expected was expected." >&2
            return 1
        fi
        expected=$((seq + 1))
    done <<< "$readings"
}

# Reads readings as they arrive and prints the first gap between two in a row that is within a
# range, in seconds. Gaps outside the range are passed over: readings that were held back arrive
# in a burst before the usual timing is back.
# Arguments: <fewest seconds> <most seconds>
# Returns: 1 if the readings ended with no gap in the range
first_gap_in_range() {
    local fewest="$1"
    local most="$2"
    local last_at=0
    local now
    local gap

    while read -r _; do
        now=$(date +%s)
        gap=$((now - last_at))
        last_at="$now"
        if [ "$gap" -ge "$fewest" ] && [ "$gap" -le "$most" ]; then
            echo "$gap"
            return 0
        fi
    done

    return 1
}

# Stops the script unless each reading is sent once: over several readings in a row, the sequence
# numbers go up by one, with none twice and none missing.
# Arguments: <project name> <address> <port> <readings topic>
#            <interval the firmware is built with, in milliseconds>
check_readings_sent_once() {
    local interval_s=$(($5 / 1000))
    local count=4
    local timeout=$((interval_s * count + 15))
    local readings

    echo "Collecting $count readings in a row on $4..."
    if ! readings=$(collect_readings "$1" "$2" "$3" "$4" "$count" "$timeout"); then
        echo "$count readings did not arrive within $timeout s." >&2
        exit 1
    fi
    echo "$readings"

    if ! seqs_go_up_by_one "any" "$readings"; then
        echo "A reading was sent twice or left out." >&2
        exit 1
    fi

    echo "Each reading is sent once."
}

# Stops the script unless readings arrive at the interval the firmware is built with: two in a
# row that are that long apart, give or take a second.
# Arguments: <project name> <address> <port> <readings topic>
#            <interval the firmware is built with, in milliseconds>
check_reading_interval() {
    local project_name="$1"
    local address="$2"
    local port="$3"
    local topic="$4"
    local interval_s=$(($5 / 1000))
    local timeout=$((interval_s * 4 + 30))
    local cert_dir="$HOME/certs/$project_name"
    local connection=(-h "$address" -p "$port" --cafile "$cert_dir/ca.crt" --cert "$cert_dir/client.crt" --key "$cert_dir/client.key")
    local gap

    echo "Timing readings on $topic until two in a row are $interval_s s apart..."

    # The pipe hands over each reading as it arrives, so the clock is read at that moment.
    # mosquitto_sub ends with an error code when the pipe closes or its time is up
    gap=$(mosquitto_sub "${connection[@]}" -t "$topic" -W "$timeout" 2> /dev/null | first_gap_in_range $((interval_s - 1)) $((interval_s + 1))) || true

    if [ -z "$gap" ]; then
        echo "No two readings in a row were $interval_s s apart within $timeout s." >&2
        exit 1
    fi

    echo "Readings are $gap s apart."
}

# Stops the script unless readings are kept while they cannot be delivered, and sent afterwards:
# the broker is stopped for a few reading intervals and started again, and the readings taken
# meanwhile then all arrive, oldest first, carrying on from the last one delivered before the stop.
# The broker is started again before anything is checked.
# Arguments: <project name> <address> <port> <readings topic>
#            <interval the firmware is built with, in milliseconds> <multipass command> <vm name>
check_readings_kept_while_broker_stopped() {
    local interval_s=$(($5 / 1000))
    local multipass="$6"
    local vm_name="$7"
    local missed=4
    local stopped_s=$((interval_s * missed))
    local count=$((missed + 1))
    local before
    local after
    local last_seq

    # 1. The last reading delivered before the stop
    if ! before=$(collect_readings "$1" "$2" "$3" "$4" 1 $((interval_s * 2 + 10))); then
        echo "No reading arrived before the broker was stopped." >&2
        exit 1
    fi
    last_seq=$(reading_seq "$before")
    echo "Last reading before the stop: $last_seq"

    # 2. Stop the broker for long enough that several readings are taken, then start it again
    echo "Stopping the broker for $stopped_s s..."
    "$multipass" exec "$vm_name" -- sudo systemctl stop mosquitto
    sleep "$stopped_s"
    "$multipass" exec "$vm_name" -- sudo systemctl start mosquitto
    echo "Broker started again."

    # 3. Listen as soon as the broker is up, before the bridge has connected to it again
    sleep 2
    echo "Collecting the $count readings that follow reading $last_seq..."
    if ! after=$(collect_readings "$1" "$2" "$3" "$4" "$count" 90); then
        echo "The readings taken while the broker was stopped did not arrive within 90 s." >&2
        exit 1
    fi
    echo "$after"

    # 4. They carry on from the last one delivered, with none twice and none missing
    if ! seqs_go_up_by_one $((last_seq + 1)) "$after"; then
        echo "A reading taken while the broker was stopped was lost or sent twice." >&2
        exit 1
    fi

    echo "Readings are kept while the broker is stopped, and arrive in order afterwards."
}
