var Tinkerforge = require('tinkerforge');

var HOST = 'localhost';
var PORT = 4223;
var UID = 'XYZ'; // Change XYZ to the UID of your Temperature Bricklet 2.0

var ipcon = new Tinkerforge.IPConnection(); // Create IP connection
var t = new Tinkerforge.BrickletTemperatureV2(UID, ipcon); // Create device object

ipcon.connect(HOST, PORT,
    function (error) {
        console.log('Error: ' + error);
    }
); // Connect to brickd
// Don't use device before ipcon is connected

ipcon.on(Tinkerforge.IPConnection.CALLBACK_CONNECTED,
    function (connectReason) {
        // Configure threshold for temperature "greater than 3000 °C"
        // with a debounce period of 1s (1000ms)
        t.setTemperatureCallbackConfiguration(1000, false, '>', 3000*100, 0);
    }
);

// Register temperature callback
t.on(Tinkerforge.BrickletTemperatureV2.CALLBACK_TEMPERATURE,
    // Callback function for temperature callback
    function (temperature) {
        console.log('Temperature: ' + temperature/100.0 + ' °C');
        console.log('It is too hot, we need air conditioning!');
    }
);

console.log('Press key to exit');
process.stdin.on('data',
    function (data) {
        ipcon.disconnect();
        process.exit(0);
    }
);
