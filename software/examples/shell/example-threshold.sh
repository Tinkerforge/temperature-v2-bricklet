#!/bin/sh
# Connects to localhost:4223 by default, use --host and --port to change this

uid=XYZ # Change XYZ to the UID of your Temperature Bricklet 2.0

# Handle incoming temperature callbacks
tinkerforge dispatch temperature-v2-bricklet $uid temperature\
 --execute "echo Temperature: {temperature}/100 °C. It is too hot, we need air conditioning!" &

# Configure threshold for temperature "greater than 30 °C"
# with a debounce period of 1s (1000ms)
tinkerforge call temperature-v2-bricklet $uid set-temperature-callback-configuration 1000 false threshold-option-greater 3000 0

echo "Press key to exit"; read dummy

kill -- -$$ # Stop callback dispatch in background
