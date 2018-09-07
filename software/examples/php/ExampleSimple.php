<?php

require_once('Tinkerforge/IPConnection.php');
require_once('Tinkerforge/BrickletTemperatureV2.php');

use Tinkerforge\IPConnection;
use Tinkerforge\BrickletTemperatureV2;

const HOST = 'localhost';
const PORT = 4223;
const UID = 'XYZ'; // Change XYZ to the UID of your Temperature Bricklet 2.0

$ipcon = new IPConnection(); // Create IP connection
$t = new BrickletTemperatureV2(UID, $ipcon); // Create device object

$ipcon->connect(HOST, PORT); // Connect to brickd
// Don't use device before ipcon is connected

// Get current temperature
$temperature = $t->getTemperature();
echo "Temperature: " . $temperature/100.0 . " °C\n";

echo "Press key to exit\n";
fgetc(fopen('php://stdin', 'r'));
$ipcon->disconnect();

?>
