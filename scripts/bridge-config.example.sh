# The weather logger's values for the Wi-Fi bridge's scripts. Copy it to bridge-config.sh and fill in your own.
# shellcheck disable=SC2034  # the values are used by the bridge's scripts

# Names the folders the SSH key and certificates are kept in
PROJECT_NAME="cloud-weather-logger"

# Name of the Multipass VM that runs the test broker
VM_NAME="broker"

# Port the broker listens on
BROKER_PORT=8883

# Wi-Fi network the bridge joins
WIFI_SSID="<your_wifi_network_name>"
WIFI_PASSWORD="<your_wifi_password>"
