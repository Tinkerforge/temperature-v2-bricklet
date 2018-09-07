#!/usr/bin/env ruby
# -*- ruby encoding: utf-8 -*-

require 'tinkerforge/ip_connection'
require 'tinkerforge/bricklet_temperature_v2'

include Tinkerforge

HOST = 'localhost'
PORT = 4223
UID = 'XYZ' # Change XYZ to the UID of your Temperature Bricklet 2.0

ipcon = IPConnection.new # Create IP connection
t = BrickletTemperatureV2.new UID, ipcon # Create device object

ipcon.connect HOST, PORT # Connect to brickd
# Don't use device before ipcon is connected

# Register temperature callback
t.register_callback(BrickletTemperatureV2::CALLBACK_TEMPERATURE) do |temperature|
  puts "Temperature: #{temperature/100.0} °C"
  puts 'It is too hot, we need air conditioning!'
end

# Configure threshold for temperature "greater than 30 °C"
# with a debounce period of 1s (1000ms)
t.set_temperature_callback_configuration 1000, false, '>', 30*100, 0

puts 'Press key to exit'
$stdin.gets
ipcon.disconnect
