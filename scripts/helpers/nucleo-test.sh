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
