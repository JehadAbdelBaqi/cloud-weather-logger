# The checks of the running Nucleo, made through the bridge and the broker.
# They use check_line_arrives from the bridge's helpers/bridge-test.sh.

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

# Prints the number a name is defined as in one of the firmware's config headers, so a value the
# firmware is built with is not written a second time in the tests.
# Arguments: <the header, e.g. config.h> <name, e.g. SENSOR_READ_INTERVAL_MS>
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

# Publishes a command and prints the reply: the first line that then arrives on the replies topic.
# It stops listening as soon as that line arrives. Prints nothing if none arrives in time.
# Arguments: <project name> <address> <port> <commands topic> <replies topic> <the command's line>
command_reply() {
    local project_name="$1"
    local address="$2"
    local port="$3"
    local commands_topic="$4"
    local replies_topic="$5"
    local message="$6"
    local cert_dir="$HOME/certs/$project_name"
    local connection=(-h "$address" -p "$port" --cafile "$cert_dir/ca.crt" --cert "$cert_dir/client.crt" --key "$cert_dir/client.key")
    local received
    received=$(mktemp)

    # Listen first, in the background, so the reply can't arrive before anything is listening
    mosquitto_sub "${connection[@]}" -t "$replies_topic" -C 1 -W 5 > "$received" 2> /dev/null &
    sleep 1
    mosquitto_pub "${connection[@]}" -t "$commands_topic" -m "$message" -q 1
    wait || true  # mosquitto_sub ends with an error code when its time is up

    cat "$received"
    rm -f "$received"
}

# Stops the script unless a command sent to the Nucleo gets the reply expected.
# The command is given an ID of its own, which the reply has to carry.
# Arguments: <project name> <address> <port> <commands topic> <replies topic>
#            <the command's number, or nothing for a command with only an ID>
#            <the command's value, or nothing if it has none> <expected result>
check_command() {
    local command="$6"
    local value="$7"
    local expected="$8"
    local id="t$RANDOM"
    local message="C,$id"
    local reply

    if [ -n "$command" ]; then
        message="$message,$command"
    fi
    if [ -n "$value" ]; then
        message="$message,$value"
    fi

    echo "Sending '$message'..."
    reply=$(command_reply "$1" "$2" "$3" "$4" "$5" "$message")

    if [ "$reply" != "R,$id,$expected" ]; then
        echo "The command '$message' was not answered with '$expected'. What arrived: '$reply'" >&2
        exit 1
    fi

    echo "Arrived: $reply"
}

# Stops the script unless the read command is answered with a reading: a reply that carries the
# command's ID and then the reading's four values, R,<id>,<time>,<temperature>,<pressure>,<humidity>.
# The values change from one reading to the next, so the reply is checked for its form.
# Arguments: <project name> <address> <port> <commands topic> <replies topic>
#            <the read command's number>
check_read_command() {
    local id="t$RANDOM"
    local message="C,$id,$6"
    local reply

    echo "Sending '$message'..."
    reply=$(command_reply "$1" "$2" "$3" "$4" "$5" "$message")

    if ! echo "$reply" | grep -Eq "^R,$id,[0-9]+,-?[0-9]+,[0-9]+,[0-9]+$"; then
        echo "The command '$message' was not answered with a reading. What arrived: '$reply'" >&2
        exit 1
    fi

    echo "Arrived: $reply"
}

# Stops the script unless each kind of command gets the reply expected: the ones that work, one
# with no command, one that is not known, and two with a value that is refused.
# Arguments: <project name> <address> <port> <commands topic> <replies topic>
#            <longest interval the rate command allows, in seconds>
#            <the help command's number> <the read command's number> <the rate command's number>
check_commands_answered() {
    local too_long=$(($6 + 1))
    local help="$7"
    local read="$8"
    local rate="$9"
    local unknown=$((rate + 50))

    check_command "$1" "$2" "$3" "$4" "$5" "$help" "" "$help=help $read=read $rate=rate"
    check_read_command "$1" "$2" "$3" "$4" "$5" "$read"
    check_command "$1" "$2" "$3" "$4" "$5" "" "" "err no command"
    check_command "$1" "$2" "$3" "$4" "$5" "$unknown" "" "err unknown command"
    check_command "$1" "$2" "$3" "$4" "$5" "$rate" "" "err rate needs a number of seconds"
    check_command "$1" "$2" "$3" "$4" "$5" "$rate" "$too_long" "err rate out of range"
}

# Stops the script unless the rate command changes how often readings arrive: a shorter interval
# is set, two readings in a row have to arrive that far apart, and the interval the firmware is
# built with is put back. The check of the reading interval that follows shows it is back.
# Arguments: <project name> <address> <port> <commands topic> <replies topic> <readings topic>
#            <interval the firmware is built with, in milliseconds> <the rate command's number>
#            <the shorter interval to try, in seconds>
check_rate_changes_timing() {
    local built_in_s=$(($7 / 1000))
    local rate="$8"
    local shorter_s="$9"

    check_command "$1" "$2" "$3" "$4" "$5" "$rate" "$shorter_s" "ok"
    check_reading_interval "$1" "$2" "$3" "$6" $((shorter_s * 1000))
    check_command "$1" "$2" "$3" "$4" "$5" "$rate" "$built_in_s" "ok"
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

# Reads readings as they arrive and prints the first two in a row whose gap is within a range:
# the gap in seconds, then the sequence numbers of the two. Gaps outside the range are passed
# over: readings that were held back arrive in a burst before the usual timing is back.
# Arguments: <fewest seconds> <most seconds>
# Returns: 1 if the readings ended with no gap in the range
first_pair_in_range() {
    local fewest="$1"
    local most="$2"
    local last_at=0
    local last_seq=""
    local reading
    local now
    local gap
    local seq

    while read -r reading; do
        now=$(date +%s)
        gap=$((now - last_at))
        seq=$(reading_seq "$reading")
        if [ "$gap" -ge "$fewest" ] && [ "$gap" -le "$most" ]; then
            echo "$gap $last_seq $seq"
            return 0
        fi
        last_at="$now"
        last_seq="$seq"
    done

    return 1
}

# Stops the script unless readings arrive at a given interval and each is sent once: two in a row
# that are that long apart, give or take a second, with sequence numbers that go up by one.
# Arguments: <project name> <address> <port> <readings topic> <the interval, in milliseconds>
check_reading_interval() {
    local project_name="$1"
    local address="$2"
    local port="$3"
    local topic="$4"
    local interval_s=$(($5 / 1000))
    local timeout=$((interval_s * 4 + 30))
    local cert_dir="$HOME/certs/$project_name"
    local connection=(-h "$address" -p "$port" --cafile "$cert_dir/ca.crt" --cert "$cert_dir/client.crt" --key "$cert_dir/client.key")
    local pair
    local gap
    local first_seq
    local second_seq

    echo "Timing readings on $topic until two in a row are $interval_s s apart..."

    # The pipe hands over each reading as it arrives, so the clock is read at that moment.
    # mosquitto_sub ends with an error code when the pipe closes or its time is up
    pair=$(mosquitto_sub "${connection[@]}" -t "$topic" -W "$timeout" 2> /dev/null | first_pair_in_range $((interval_s - 1)) $((interval_s + 1))) || true
    read -r gap first_seq second_seq <<< "$pair"

    if [ -z "${second_seq:-}" ]; then
        echo "No two readings in a row were $interval_s s apart within $timeout s." >&2
        exit 1
    fi
    if [ "$second_seq" != $((first_seq + 1)) ]; then
        echo "Reading $second_seq arrived after reading $first_seq: one was sent twice or left out." >&2
        exit 1
    fi

    echo "Readings $first_seq and $second_seq are $gap s apart."
}

# Stops the script unless readings are kept while they cannot be delivered, and sent afterwards:
# the broker is stopped for two reading intervals and started again, and the readings taken
# meanwhile then all arrive, oldest first, carrying on from the last one delivered before the stop.
# The broker is started again before anything is checked.
# Arguments: <project name> <address> <port> <readings topic>
#            <interval the firmware is built with, in milliseconds> <multipass command> <vm name>
check_readings_kept_while_broker_stopped() {
    local interval_s=$(($5 / 1000))
    local multipass="$6"
    local vm_name="$7"
    local missed=2
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
